# __*__coding:utf-8 __*__
from optparse import OptionParser
import sys

def xorEncode(file,key,output):
    shellcode = ""
    shellcode_size = 0
    while True:
        code = file.read(1)
        if not code :
            break
        code = ord(code) ^ key
        code_hex = hex(code)
        code_hex = code_hex.replace("0x",'')
        if len(code_hex) == 1:
            code_hex = '0'+code_hex
        shellcode += '\\x' + code_hex
        shellcode_size += 1
    file.close()
    output.write(shellcode)
    output.close()
    print(f"shellcodeSize:{shellcode_size}")

if __name__== "__main__":
    usage = "usage: %prog [-f] input_filename [-k] key [-o] output_filename"
    parser = OptionParser(usage=usage)
    parser.add_option("-f","--file",help="选取raw格式的shellcode",type="string",dest="file")
    parser.add_option("-k","--key",help="输入加密Key",type="int",dest="key",default=10)
    parser.add_option("-o","--output",help="输出文件名",type="string",dest="output")

    if len(sys.argv) < 4:
        parser.print_help()
        exit()

    (options, params) = parser.parse_args()
    with open(options.file,'rb') as file:
        with open(options.output,'w+') as output:
            xorEncode(file,options.key,output)
