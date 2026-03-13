#include "framework.h"

PVOID APIENTRY _internal_alloc(size_t size) {
	PVOID res = HeapAlloc(GetProcessHeap(), 0, size);
	if (res) memset(res, 0, size);
	return res;
}
void APIENTRY _internal_free(PVOID ptr) {
	HeapFree(GetProcessHeap(), 0, ptr);

}
