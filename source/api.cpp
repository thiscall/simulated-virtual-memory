#include "framework.h"

DLLEXPORT PVOID APIENTRY vmCreate(LPCSTR file, DWORD sizeMB, DWORD bufMB) {
	if (sizeMB < 8U || sizeMB > (1U << 16)) return NULL;
	if (bufMB < 4U) bufMB = 4U;
	if (bufMB >= sizeMB) return NULL;
	PVMINFO pVM = (PVMINFO)_internal_alloc(sizeof(VMINFO));
	if (!pVM) return NULL;
	pVM->sizeMB = sizeMB, pVM->maxCommit = bufMB;
	pVM->hFile = CreateFileA(file, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE | FILE_FLAG_WRITE_THROUGH, NULL);
	LARGE_INTEGER x = { 0 }; x.QuadPart = sizeMB << 20;
	SetFilePointerEx(pVM->hFile, x, NULL, FILE_BEGIN);
	int r1 = SetEndOfFile(pVM->hFile);
	pVM->pMem = VirtualAlloc(NULL, sizeMB << 20, MEM_RESERVE, PAGE_READWRITE);
	pVM->pLoadTime = (PDWORD)_internal_alloc(sizeMB << 2);
	if (!pVM->hFile || !pVM->pMem || !r1 || !pVM->pLoadTime) {
		CloseHandle(pVM->hFile);
		VirtualFree(pVM->pMem, 0, MEM_RELEASE);
		_internal_free(pVM->pLoadTime);
		_internal_free(pVM);
		return NULL;
	}
	memset(pVM->pLoadTime, -1, sizeMB << 2);
	PVMLIST _head = (PVMLIST)_internal_alloc(sizeof(VMLIST));
	_head->pVM = pVM, _head->next = _vm_reg;
	_vm_reg = _head;
	return pVM;
}
DLLEXPORT void APIENTRY vmDestroy(PVOID pVM) {
	if (!pVM) return;
	PVMINFO vm = (PVMINFO)pVM;
	CloseHandle(vm->hFile);
	VirtualFree(vm->pMem, 0, MEM_DECOMMIT | MEM_RELEASE);
	_internal_free(pVM);
	for (PVMLIST x = _vm_reg; x; x = x->next) if (x->pVM == pVM) x->pVM = NULL;
}
DLLEXPORT PVOID APIENTRY vmBase(PVOID pVM) {
	return pVM ? ((PVMINFO)pVM)->pMem : NULL;
}
DLLEXPORT void APIENTRY vmTrim(PVOID pVM) {
	if (!pVM) return;
	PVMINFO vm = (PVMINFO)pVM;
	for (DWORD i = 0; i < vm->sizeMB; i++)
		if (vm->pLoadTime[i] != ~0U) _vm_direct_decommit(vm, i);
}
