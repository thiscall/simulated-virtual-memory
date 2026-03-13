#include "framework.h"
PVMINFO APIENTRY _vm_locate_info(size_t addr) {
	for (PVMLIST x = _vm_reg; x; x = x->next) if (x->pVM && addr >= (size_t)x->pVM->pMem && addr < (size_t(x->pVM->pMem) + (x->pVM->sizeMB << 20))) return x->pVM;
	return NULL;
}
LONG APIENTRY _vm_exception_handler(PEXCEPTION_POINTERS excpt) {
	PVMINFO pVM;
	size_t addr = excpt->ExceptionRecord->ExceptionInformation[1];
	if (excpt->ExceptionRecord->ExceptionRecord ||
		excpt->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION ||
		!(pVM = _vm_locate_info(addr))) return EXCEPTION_CONTINUE_SEARCH;
	EnterCriticalSection(&_vmlock);
	DWORD res = _vm_unchecked_commit(pVM, (PVOID)addr);
	LeaveCriticalSection(&_vmlock);
	if (res) return EXCEPTION_CONTINUE_EXECUTION;
	else return EXCEPTION_CONTINUE_SEARCH;
}
DWORD APIENTRY _vm_direct_commit(PVMINFO pVM, size_t page_offset) {
	const size_t pos = size_t(pVM->pMem) + (page_offset << 20);
	LARGE_INTEGER dist = { 0 };
	dist.QuadPart = page_offset << 20;
	void* pt = (void*)pos;
	DWORD suc = 0;
	if (!VirtualAlloc(pt, 1Ui64 << 20, MEM_COMMIT, PAGE_READWRITE)) return 0;
	SetFilePointerEx(pVM->hFile, dist, NULL, FILE_BEGIN);
	ReadFile(pVM->hFile, pt, 1U << 20, &suc, NULL);
	if (suc != (1Ui32 << 20)) return 0;
	pVM->pLoadTime[page_offset] = ++(pVM->loadCount);
	if (pVM->loadCount >= 0xEE000000U) {
		pVM->loadCount -= 0xE0000000U;
		for (DWORD i = 0; i < pVM->sizeMB; i++)
			if (pVM->pLoadTime[i] > 0xE0000000U) pVM->pLoadTime[i] -= 0xE0000000U;
	}
	pVM->curCommit++;
	return 1;
}
DWORD APIENTRY _vm_unchecked_commit(PVMINFO pVM, PVOID ptr) {
	size_t offset = ((size_t)ptr - (size_t)pVM->pMem) >> 20;
	if (offset > pVM->sizeMB || pVM->pLoadTime[offset] != ~0U) return 0;
	if (pVM->curCommit == pVM->maxCommit && !_vm_autotrim(pVM, offset)) return 0;
	return _vm_direct_commit(pVM, offset);
}
DWORD APIENTRY _vm_direct_decommit(PVMINFO pVM, size_t page_offset) {
	const size_t pos = size_t(pVM->pMem) + (page_offset << 20);
	void* pt = (void*)pos;
	DWORD suc = 0;
	LARGE_INTEGER dist = { 0 }; dist.QuadPart = page_offset << 20;
	SetFilePointerEx(pVM->hFile, dist, NULL, FILE_BEGIN);
	WriteFile(pVM->hFile, pt, 1U << 20, &suc, NULL);
	if (suc != (1U << 20)) return 0;
	pVM->pLoadTime[page_offset] = ~0U, pVM->curCommit--;
	return VirtualFree(pt, 1Ui64 << 20, MEM_DECOMMIT);
}
DWORD APIENTRY _vm_unchecked_decommit(PVMINFO pVM, PVOID ptr) {
	size_t offset = ((size_t)ptr - (size_t)pVM->pMem) >> 20;
	if (offset > pVM->sizeMB || pVM->pLoadTime[offset] == ~0U) return 0;
	return _vm_direct_decommit(pVM, offset);
}
DWORD APIENTRY _vm_autotrim(PVMINFO pVM, size_t pageBypass) {
	DWORD found = ~0U;
	for (DWORD i = 0; i < pVM->sizeMB; i++) {
		if (i == pageBypass) continue;
		if (found == ~0U) {
			if (pVM->pLoadTime[i] != ~0U) found = i;
			continue;
		} else if (pVM->pLoadTime[i] < pVM->pLoadTime[found]) found = i;
	}
	if (found == ~0U) return 0;
	else return _vm_direct_decommit(pVM, found);
}
