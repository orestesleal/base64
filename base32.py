#!/usr/bin/python
import sys,base64
x = base64.b32encode(sys.argv[1])
print(x)
print(base64.b32decode(x))

