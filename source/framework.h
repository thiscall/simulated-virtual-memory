#pragma once
#include <windows.h>

#define DLLEXPORT extern "C" __declspec(dllexport)

typedef struct _VMINFO {
	HANDLE hFile;
	DWORD sizeMB; //reserved area
	PVOID pMem; //reserved address
	PDWORD pLoadTime;
	DWORD loadCount;
	DWORD curCommit; //commit count
	DWORD maxCommit;  //max commit pages
}VMINFO, *PVMINFO;
typedef struct _VMLIST {
	PVMINFO pVM;
	struct _VMLIST* next;
}VMLIST, *PVMLIST;
extern PVOID hVEH;
extern PVMLIST _vm_reg;
extern CRITICAL_SECTION _vmlock;
//API
DLLEXPORT PVOID APIENTRY vmCreate(LPCSTR file, DWORD sizeMB, DWORD bufMB);
DLLEXPORT void APIENTRY vmDestroy(PVOID pVM);
DLLEXPORT PVOID APIENTRY vmBase(PVOID pVM);
DLLEXPORT void APIENTRY vmTrim(PVOID pVM);
//Internal Functions
PVOID APIENTRY _internal_alloc(size_t size);
void APIENTRY _internal_free(PVOID ptr);
//Page Faults
PVMINFO APIENTRY _vm_locate_info(size_t addr);
LONG APIENTRY _vm_exception_handler(PEXCEPTION_POINTERS excpt);
DWORD APIENTRY _vm_direct_commit(PVMINFO pVM, size_t page_offset);
DWORD APIENTRY _vm_unchecked_commit(PVMINFO pVM, PVOID ptr);
DWORD APIENTRY _vm_direct_decommit(PVMINFO pVM, size_t page_offset);
DWORD APIENTRY _vm_unchecked_decommit(PVMINFO pVM, PVOID ptr);
DWORD APIENTRY _vm_autotrim(PVMINFO pVM, size_t pageBypass);
