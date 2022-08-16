import base64

buf = b"\xfc\x48\..."

# b64shellcode = base64.b64encode(buf1)                   # b'xxxx'
b64shellcode = base64.b64encode(buf).decode('ascii')    #获取纯字符串

print(b64shellcode)