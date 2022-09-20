def convertToMAC(shellcode):
    if len(shellcode) % 6 != 0:
        print("\n[*] length:", len(shellcode) + (6 -(len(shellcode) % 6)))
        addNullbyte = b"\x00" *(6 - (len(shellcode) % 6))
        shellcode += addNullbyte
       
    mac = []
    for i in range(0,len(shellcode), 6):
        tmp_mac = ""
        for j in shellcode[i:i + 6]:
            if len(hex(j).replace("0x", "")) == 1:
                tmp_mac = tmp_mac + "0" + hex(j).replace("0x","").upper() + "-"
            else:
                tmp_mac = tmp_mac + hex(j).replace("0x","").upper() + "-"
        mac.append(tmp_mac[:-1])
    return mac
                            
if __name__ == '__main__':
    buf = b'''\xfc\x48\...'''
    u = convertToMAC(buf)
    print(str(u).replace("'","\""))
