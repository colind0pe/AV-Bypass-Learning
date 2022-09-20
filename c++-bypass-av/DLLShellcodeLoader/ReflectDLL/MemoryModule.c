/*
 * Memory DLL loading code
 * Version 0.0.3
 *
 * Copyright (c) 2004-2012 by Joachim Bauch / mail@joachim-bauch.de
 * http://www.joachim-bauch.de
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is MemoryModule.c
 *
 * The Initial Developer of the Original Code is Joachim Bauch.
 *
 * Portions created by Joachim Bauch are Copyright (C) 2004-2012
 * Joachim Bauch. All Rights Reserved.
 *
 */

#define _CRT_SECURE_NO_DEPRECATE 1

#define _CRT_NONSTDC_NO_DEPRECATE 1
#ifndef __GNUC__
 // disable warnings about pointer <-> DWORD conversions
#pragma warning( disable : 4311 4312 )
#endif

#ifdef _WIN64
#define POINTER_TYPE ULONGLONG
#else
#define POINTER_TYPE DWORD
#endif

#if (defined DEBUG) || (defined _DEBUG)
#define DEBUG_OUTPUT 1
#endif

#include <Windows.h>
#include <winnt.h>
#ifdef DEBUG_OUTPUT
#include <stdio.h>
#endif

#ifndef IMAGE_SIZEOF_BASE_RELOCATION
// Vista SDKs no longer define IMAGE_SIZEOF_BASE_RELOCATION!?
#define IMAGE_SIZEOF_BASE_RELOCATION (sizeof(IMAGE_BASE_RELOCATION))
#endif

#include "MemoryModule.h"

typedef struct {
	PIMAGE_NT_HEADERS headers;
	unsigned char* codeBase;
	HMODULE* modules;
	int numModules;
	int initialized;
} MEMORYMODULE, * PMEMORYMODULE;

typedef BOOL(WINAPI* DllEntryProc)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

#define GET_HEADER_DICTIONARY(module, idx)	&(module)->headers->OptionalHeader.DataDirectory[idx]

#ifdef DEBUG_OUTPUT
static void
OutputLastError(const char* msg)
{
	LPSTR tmp;
	char* tmpmsg;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPSTR)&tmp, 0, NULL);
	tmpmsg = (char*)LocalAlloc(LPTR, strlen(msg) + strlen(tmp) + 3);
	sprintf(tmpmsg, "%s: %s", msg, tmp);
	OutputDebugStringA(tmpmsg);
	LocalFree(tmpmsg);
	LocalFree(tmp);
}
#endif

static void CopySections(const unsigned char* data, PIMAGE_NT_HEADERS old_headers, PMEMORYMODULE module)
{
	int i, size;
	unsigned char* codeBase = module->codeBase;
	unsigned char* dest;
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(module->headers);
	for (i = 0; i < module->headers->FileHeader.NumberOfSections; i++, section++) {
		if (section->SizeOfRawData == 0) {
			// section doesn't contain data in the dll itself, but may define
			// uninitialized data
			size = old_headers->OptionalHeader.SectionAlignment;
			if (size > 0) {
				dest = (unsigned char*)VirtualAlloc(codeBase + section->VirtualAddress,
					size,
					MEM_COMMIT,
					PAGE_READWRITE);

				section->Misc.PhysicalAddress = (POINTER_TYPE)dest;
				memset(dest, 0, size);
			}

			// section is empty
			continue;
		}

		// commit memory block and copy data from dll
		dest = (unsigned char*)VirtualAlloc(codeBase + section->VirtualAddress,
			section->SizeOfRawData,
			MEM_COMMIT,
			PAGE_READWRITE);
		memcpy(dest, data + section->PointerToRawData, section->SizeOfRawData);
		section->Misc.PhysicalAddress = (POINTER_TYPE)dest;
	}
}

// 此处3维数组的定义比较巧妙，类似Executable, Readable, Writeable进行排列组合
// 详见FinalizeSections函数中对这个数组的使用
//
// Protection flags for memory pages (Executable, Readable, Writeable)
static int ProtectionFlags[2][2][2] = {
	{
		// not executable
		{PAGE_NOACCESS, PAGE_WRITECOPY},
		{PAGE_READONLY, PAGE_READWRITE},
	}, {
		// executable
		{PAGE_EXECUTE, PAGE_EXECUTE_WRITECOPY},
		{PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE},
	},
};

static void FinalizeSections(PMEMORYMODULE module)
{
	int i;
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(module->headers);
#ifdef _WIN64
	POINTER_TYPE imageOffset = (module->headers->OptionalHeader.ImageBase & 0xffffffff00000000);
#else
#define imageOffset 0
#endif

	// loop through all sections and change access flags
	for (i = 0; i < module->headers->FileHeader.NumberOfSections; i++, section++) {
		DWORD protect, oldProtect, size;
		int executable = (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
		int readable = (section->Characteristics & IMAGE_SCN_MEM_READ) != 0;
		int writeable = (section->Characteristics & IMAGE_SCN_MEM_WRITE) != 0;

		if (section->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) {
			// section is not needed any more and can safely be freed
			VirtualFree((LPVOID)((POINTER_TYPE)section->Misc.PhysicalAddress | imageOffset), section->SizeOfRawData, MEM_DECOMMIT);
			continue;
		}

		// determine protection flags based on characteristics
		protect = ProtectionFlags[executable][readable][writeable];
		if (section->Characteristics & IMAGE_SCN_MEM_NOT_CACHED) {
			protect |= PAGE_NOCACHE;
		}

		// determine size of region
		size = section->SizeOfRawData;
		if (size == 0) {
			if (section->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) {
				size = module->headers->OptionalHeader.SizeOfInitializedData;
			}
			else if (section->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
				size = module->headers->OptionalHeader.SizeOfUninitializedData;
			}
		}

		if (size > 0) {
			// change memory access flags
			if (VirtualProtect((LPVOID)((POINTER_TYPE)section->Misc.PhysicalAddress | imageOffset), size, protect, &oldProtect) == 0)
#ifdef DEBUG_OUTPUT
				OutputLastError("Error protecting memory page")
#endif
				;
		}
	}
#ifndef _WIN64
#undef imageOffset
#endif
}

static void PerformBaseRelocation(PMEMORYMODULE module, SIZE_T delta)
{
	DWORD i;
	unsigned char* codeBase = module->codeBase;

	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(module, IMAGE_DIRECTORY_ENTRY_BASERELOC);
	if (directory->Size > 0) {
		PIMAGE_BASE_RELOCATION relocation = (PIMAGE_BASE_RELOCATION)(codeBase + directory->VirtualAddress);
		for (; relocation->VirtualAddress > 0; ) {
			unsigned char* dest = codeBase + relocation->VirtualAddress;
			unsigned short* relInfo = (unsigned short*)((unsigned char*)relocation + IMAGE_SIZEOF_BASE_RELOCATION);
			for (i = 0; i < ((relocation->SizeOfBlock - IMAGE_SIZEOF_BASE_RELOCATION) / 2); i++, relInfo++) {
				DWORD* patchAddrHL;
#ifdef _WIN64
				ULONGLONG* patchAddr64;
#endif
				int type, offset;

				// the upper 4 bits define the type of relocation
				type = *relInfo >> 12;
				// the lower 12 bits define the offset
				offset = *relInfo & 0xfff;

				switch (type)
				{
				case IMAGE_REL_BASED_ABSOLUTE:
					// skip relocation
					break;

				case IMAGE_REL_BASED_HIGHLOW:
					// change complete 32 bit address
					patchAddrHL = (DWORD*)(dest + offset);
					*patchAddrHL += delta;
					break;

#ifdef _WIN64
				case IMAGE_REL_BASED_DIR64:
					patchAddr64 = (ULONGLONG*)(dest + offset);
					*patchAddr64 += delta;
					break;
#endif

				default:
					//printf("Unknown relocation: %d\n", type);
					break;
				}
			}

			// advance to next relocation block
			relocation = (PIMAGE_BASE_RELOCATION)(((char*)relocation) + relocation->SizeOfBlock);
		}
	}
}

static int BuildImportTable(PMEMORYMODULE module)
{
	int result = 1;
	unsigned char* codeBase = module->codeBase;

	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(module, IMAGE_DIRECTORY_ENTRY_IMPORT);
	if (directory->Size > 0) {
		PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(codeBase + directory->VirtualAddress);
		for (; !IsBadReadPtr(importDesc, sizeof(IMAGE_IMPORT_DESCRIPTOR)) && importDesc->Name; importDesc++) {
			POINTER_TYPE* thunkRef;
			FARPROC* funcRef;

			HMODULE handle = LoadLibraryA((LPCSTR)(codeBase + importDesc->Name));
			if (handle == NULL) {
#if DEBUG_OUTPUT
				OutputLastError("Can't load library");
#endif
				result = 0;
				break;
			}

			module->modules = (HMODULE*)realloc(module->modules, (module->numModules + 1) * (sizeof(HMODULE)));
			if (module->modules == NULL) {
				result = 0;
				break;
			}

			module->modules[module->numModules++] = handle;
			if (importDesc->OriginalFirstThunk) {
				thunkRef = (POINTER_TYPE*)(codeBase + importDesc->OriginalFirstThunk);
				funcRef = (FARPROC*)(codeBase + importDesc->FirstThunk);
			}
			else {
				// no hint table
				thunkRef = (POINTER_TYPE*)(codeBase + importDesc->FirstThunk);
				funcRef = (FARPROC*)(codeBase + importDesc->FirstThunk);
			}
			for (; *thunkRef; thunkRef++, funcRef++) {
				if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) {
					*funcRef = (FARPROC)GetProcAddress(handle, (LPCSTR)IMAGE_ORDINAL(*thunkRef));
				}
				else {
					PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)(codeBase + (*thunkRef));
					*funcRef = (FARPROC)GetProcAddress(handle, (LPCSTR)&thunkData->Name);
				}
				if (*funcRef == 0) {
					result = 0;
					break;
				}
			}

			if (!result) {
				break;
			}
		}
	}

	return result;
}

HMEMORYMODULE MemoryLoadLibrary(const void* data)
{
	PMEMORYMODULE result;
	PIMAGE_DOS_HEADER dos_header; // DOS头
	PIMAGE_NT_HEADERS old_header; // PE头
	unsigned char* code, * headers;
	SIZE_T locationDelta;
	DllEntryProc DllEntry;
	BOOL successfull;

	// 获取DOS头指针，并检查DOS头
	dos_header = (PIMAGE_DOS_HEADER)data;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
#if DEBUG_OUTPUT
		OutputDebugStringA("Not a valid executable file.\n");
#endif
		return NULL;
	}

	// 获取PE头指针，并检查PE头
	old_header = (PIMAGE_NT_HEADERS) & ((const unsigned char*)(data))[dos_header->e_lfanew];
	if (old_header->Signature != IMAGE_NT_SIGNATURE) {
#if DEBUG_OUTPUT
		OutputDebugStringA("No PE header found.\n");
#endif
		return NULL;
	}

	// 在"PEHeader.OptionalHeader.ImageBase"处预定"PEHeader.OptionalHeader.SizeOfImage"字节的空间
	code = (unsigned char*)VirtualAlloc((LPVOID)(old_header->OptionalHeader.ImageBase),
		old_header->OptionalHeader.SizeOfImage,
		MEM_RESERVE,
		PAGE_READWRITE);

	if (code == NULL) {
		// try to allocate memory at arbitrary position
		code = (unsigned char*)VirtualAlloc(NULL,
			old_header->OptionalHeader.SizeOfImage,
			MEM_RESERVE,
			PAGE_READWRITE);
		if (code == NULL) {
#if DEBUG_OUTPUT
			OutputLastError("Can't reserve memory");
#endif
			return NULL;
		}
	}

	// 在进程的默认堆上分配"sizeof(MEMORYMODULE)"字节的空间用于存放MEMORYMODULE结构体
	// 方便函数末尾将该结构体指针当作返回值返回
	result = (PMEMORYMODULE)HeapAlloc(GetProcessHeap(), 0, sizeof(MEMORYMODULE));
	result->codeBase = code;
	result->numModules = 0;
	result->modules = NULL;
	result->initialized = 0;


	// 一次性从code地址处将整个映像所需的内存区域都分配
	VirtualAlloc(code,
		old_header->OptionalHeader.SizeOfImage,
		MEM_COMMIT,
		PAGE_READWRITE);

	// 原作者的代码中此处会再次调用VirtualAlloc从code处分配SizeOfHeaders大小的内存，
	// 但这步操作属于多余的，因为上一步已经在code处分配了所需的整个内存区域了，
	// 所以直接将此处更改为 headers = code;
	//
	//headers = (unsigned char *)VirtualAllocEx(process, code,
	//	old_header->OptionalHeader.SizeOfHeaders,
	//	MEM_COMMIT,
	//	PAGE_READWRITE);
	headers = code;

	// 拷贝DOS头 + DOS STUB + PE头到headers地址处
	memcpy(headers, dos_header, dos_header->e_lfanew + old_header->OptionalHeader.SizeOfHeaders);
	result->headers = (PIMAGE_NT_HEADERS) & ((const unsigned char*)(headers))[dos_header->e_lfanew];

	// 更新"MEMORYMODULE.PIMAGE_NT_HEADERS"结构体中的基地址
	result->headers->OptionalHeader.ImageBase = (POINTER_TYPE)code;

	// 从dll文件内容中拷贝每个section（节）的数据到新的内存区域
	CopySections(data, old_header, result);

	// 检查加载到进程地址空间的位置和之前PE文件中指定的基地址是否一致，如果不一致，则需要重定位
	locationDelta = (SIZE_T)(code - old_header->OptionalHeader.ImageBase);
	if (locationDelta != 0) {
		PerformBaseRelocation(result, locationDelta);
	}

	// 加载依赖dll，并构建"PEHeader.OptionalHeader.DataDirectory.Image_directory_entry_import"导入表
	if (!BuildImportTable(result)) {
		goto error;
	}

	// 根据每个Section的"PEHeader.Image_Section_Table.Characteristics"属性来设置内存页的访问属性；
	// 如果被设置为"discardable"属性，则释放该内存页
	FinalizeSections(result);

	// 获取DLL的入口函数指针，并调用
	if (result->headers->OptionalHeader.AddressOfEntryPoint != 0) {
		DllEntry = (DllEntryProc)(code + result->headers->OptionalHeader.AddressOfEntryPoint);
		if (DllEntry == 0) {
#if DEBUG_OUTPUT
			OutputDebugStringA("Library has no entry point.\n");
#endif
			goto error;
		}

		// notify library about attaching to process
		successfull = (*DllEntry)((HINSTANCE)code, DLL_PROCESS_ATTACH, 0);
		if (!successfull) {
#if DEBUG_OUTPUT
			OutputDebugStringA("Can't attach library.\n");
#endif
			goto error;
		}
		result->initialized = 1;
	}

	return (HMEMORYMODULE)result;

error:
	// cleanup
	MemoryFreeLibrary(result);
	return NULL;
}

FARPROC MemoryGetProcAddress(HMEMORYMODULE module, const char* name)
{
	unsigned char* codeBase = ((PMEMORYMODULE)module)->codeBase;
	int idx = -1;
	DWORD i, * nameRef;
	WORD* ordinal;
	PIMAGE_EXPORT_DIRECTORY exports;
	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY((PMEMORYMODULE)module, IMAGE_DIRECTORY_ENTRY_EXPORT);
	if (directory->Size == 0) {
		// no export table found
		return NULL;
	}

	exports = (PIMAGE_EXPORT_DIRECTORY)(codeBase + directory->VirtualAddress);
	if (exports->NumberOfNames == 0 || exports->NumberOfFunctions == 0) {
		// DLL doesn't export anything
		return NULL;
	}

	// search function name in list of exported names
	nameRef = (DWORD*)(codeBase + exports->AddressOfNames);
	ordinal = (WORD*)(codeBase + exports->AddressOfNameOrdinals);
	for (i = 0; i < exports->NumberOfNames; i++, nameRef++, ordinal++) {
		if (stricmp(name, (const char*)(codeBase + (*nameRef))) == 0) {
			idx = *ordinal;
			break;
		}
	}

	if (idx == -1) {
		// exported symbol not found
		return NULL;
	}

	if ((DWORD)idx > exports->NumberOfFunctions) {
		// name <-> ordinal number don't match
		return NULL;
	}

	// AddressOfFunctions contains the RVAs to the "real" functions
	return (FARPROC)(codeBase + (*(DWORD*)(codeBase + exports->AddressOfFunctions + (idx * 4))));
}

void MemoryFreeLibrary(HMEMORYMODULE mod)
{
	int i;
	PMEMORYMODULE module = (PMEMORYMODULE)mod;

	if (module != NULL) {
		if (module->initialized != 0) {
			// notify library about detaching from process
			DllEntryProc DllEntry = (DllEntryProc)(module->codeBase + module->headers->OptionalHeader.AddressOfEntryPoint);
			(*DllEntry)((HINSTANCE)module->codeBase, DLL_PROCESS_DETACH, 0);
			module->initialized = 0;
		}

		if (module->modules != NULL) {
			// free previously opened libraries
			for (i = 0; i < module->numModules; i++) {
				if (module->modules[i] != INVALID_HANDLE_VALUE) {
					FreeLibrary(module->modules[i]);
				}
			}

			free(module->modules);
		}

		if (module->codeBase != NULL) {
			// release memory of library
			VirtualFree(module->codeBase, 0, MEM_RELEASE);
		}

		HeapFree(GetProcessHeap(), 0, module);
	}

}

