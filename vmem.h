#pragma once
#pragma comment(lib, "vmem.lib")
#include <windows.h>
#define DLLIMPORT extern "C" __declspec(dllimport)
DLLIMPORT PVOID APIENTRY vmCreate(LPCSTR file, DWORD sizeMB, DWORD bufMB);
/* vmCreate：创建一个虚拟内存文件
* 参数 file：临时文件名
* 参数 sizeMB：临时文件的大小（单位MB）
* 参数 bufMB：允许占用内存的大小（单位MB）
* 返回值：若创建成功，返回虚拟内存的信息；否则返回 NULL
*/
DLLIMPORT void APIENTRY vmDestroy(PVOID pVM);
/* vmDestroy：删除vmem.dll创建的虚拟内存
* 参数 pVM：虚拟内存的信息，即vmCreate的返回值
*/
DLLIMPORT PVOID APIENTRY vmBase(PVOID pVM);
/* vmBase：获取vmem.dll创建的虚拟内存基址
* 参数 pVM：虚拟内存的信息，即vmCreate的返回值
* 返回值：pVM指向的虚拟内存的基址
*/
DLLIMPORT void APIENTRY vmTrim(PVOID pVM);
/* vmDestroy：删除vmem.dll创建的虚拟内存
* 参数 pVM：虚拟内存的信息，即vmCreate的返回值
*/