from Crypto.IO import PEM

buf = b"\xfc\x48\x83\..."
# 加密
# passphrase：指定密钥
# marker：指定名称
buf = PEM.encode(buf, marker="shellcode", passphrase=b'12345', randfunc=None)
print(buf)