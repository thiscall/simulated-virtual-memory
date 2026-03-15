#include "framework.h"

#pragma optimize("", off)
void* APIENTRY mymemset(void* base, int ch, size_t n) {
	for (UCHAR* p = (UCHAR*)base; n--; p++) *p = (UCHAR)ch;
	return base;
}
#pragma optimize("", on)
PVOID APIENTRY _internal_alloc(size_t size) {
	PVOID res = HeapAlloc(GetProcessHeap(), 0, size);
	if (res) mymemset(res, 0, size);
	return res;
}
void APIENTRY _internal_free(PVOID ptr) {
	HeapFree(GetProcessHeap(), 0, ptr);
}