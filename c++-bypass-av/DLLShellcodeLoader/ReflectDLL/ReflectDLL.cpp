#include <Windows.h>;
#include <stdio.h>
#include "MemoryModule.h"

typedef VOID(*msg)(VOID);

// 打开文件并获取大小
DWORD OpenBadCodeDLL(HANDLE& hBadCodeDll, LPCWSTR lpwszBadCodeFileName) {
	DWORD dwHighFileSize = 0;
	DWORD dwLowFileSize = 0;
	// 打开文件
	hBadCodeDll = CreateFile(lpwszBadCodeFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hBadCodeDll == INVALID_HANDLE_VALUE) {
		return GetLastError();
	}
	dwLowFileSize = GetFileSize(hBadCodeDll, &dwHighFileSize);
	return dwLowFileSize;
}


int main()
{
	msg RunMsg;  // msg函数的函数指针
	HMEMORYMODULE hModule; // MemoryModule句柄，应该可以这么理解,,
	HANDLE hBadCodeDll = INVALID_HANDLE_VALUE; // 打开PE文件的句柄
	WCHAR szBadCodeFile[] = TEXT("shellcode.dll"); // PE文件的物理路径
	DWORD dwFileSize = 0; // PE文件大小
	DWORD dwReadOfFileSize = 0; // 已读取的PE文件大小
	PBYTE bFileBuffer = NULL; // PE文件的内存地址

	// 打开文件
	dwFileSize = OpenBadCodeDLL(hBadCodeDll, szBadCodeFile);
	// 如果打开失败直接退出
	if (hBadCodeDll == INVALID_HANDLE_VALUE) {
		return GetLastError();
	}
	// 申请放置PE文件的内存空间
	bFileBuffer = new BYTE[dwFileSize];
	// 读取文件
	ReadFile(hBadCodeDll, bFileBuffer, dwFileSize, &dwReadOfFileSize, NULL);
	// 如果读取错误直接退出
	if (dwReadOfFileSize != dwFileSize) {
		return GetLastError();
	}
	// 关闭打开PE文件的句柄
	CloseHandle(hBadCodeDll);
	// 导入PE文件
	hModule = MemoryLoadLibrary(bFileBuffer);
	// 如果加载失败，就退出
	if (hModule == NULL) {
		delete[] bFileBuffer;
		return -1;
	}
	// 获取msg导出函数地址
	RunMsg = (msg)MemoryGetProcAddress(hModule, "msg");
	// 运行msg函数
	RunMsg();
	// 释放资源
	MemoryFreeLibrary(hModule);
	// 释放PE内存
	delete[] bFileBuffer;

	return GetLastError();
}