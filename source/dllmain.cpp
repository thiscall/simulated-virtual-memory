#include "framework.h"

PVOID hVEH = NULL;
PVMLIST _vm_reg = NULL;
CRITICAL_SECTION _vmlock;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        hVEH = AddVectoredExceptionHandler(1, _vm_exception_handler);
        _vm_reg = NULL;
        InitializeCriticalSection(&_vmlock);
        break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    case DLL_PROCESS_DETACH:
        RemoveVectoredExceptionHandler(hVEH);
        for (PVMLIST x = _vm_reg, tmp = NULL; x; x = tmp) {
            tmp = x->next;
            if (x->pVM) vmDestroy(x->pVM);
            _internal_free(x);
        }
        DeleteCriticalSection(&_vmlock);
        break;
    }
    return TRUE;
}