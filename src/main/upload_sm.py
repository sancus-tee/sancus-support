#!/usr/bin/env python

import sys

def int_to_bytes(i):
    if i > 2 ** 16:
        print 'Number too large'
        exit(1)
    return chr(i >> 8) + chr(i & 0xff)

if len(sys.argv) != 4:
    print 'Usage:', sys.argv[0], '<file> <name> <device>'
    exit(1)

elffile_name = sys.argv[1]
spm_name = sys.argv[2]
devfile_name = sys.argv[3]

try:
    with open(elffile_name, 'r') as f:
        elffile_contents = f.read()
except Exception as e:
    print 'Error reading file:', e
    exit(1)

vendor_id = int_to_bytes(0xbabe)
size = len(elffile_contents)
size_bytes = int_to_bytes(size)

try:
    with open(devfile_name, 'w') as f:
        f.write('\x01')
        f.write(spm_name + '\x00')
        f.write(vendor_id)
        f.write(size_bytes)
        f.write(elffile_contents)
except Exception as e:
    print 'Error writing to device:', e
    exit(1)
