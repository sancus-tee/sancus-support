#!/usr/bin/env python

import sys

if len(sys.argv) != 3:
    print 'Usage:', sys.argv[0], '<file> <device>'
    exit(1)

elffile_name = sys.argv[1]
devfile_name = sys.argv[2]

try:
    with open(elffile_name, 'r') as f:
        elffile_contents = f.read()
except Exception as e:
    print 'Error reading file:', e
    exit(1)

size = len(elffile_contents)

if size > 2 ** 16:
    print 'File too large'
    exit(1)

size_bytes = chr(size >> 8) + chr(size & 0xff)

try:
    with open(devfile_name, 'w') as f:
        f.write('\x01')
        f.write(size_bytes)
        f.write(elffile_contents)
except Exception as e:
    print 'Error writing to device:', e
    exit(1)
