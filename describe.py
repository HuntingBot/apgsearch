#!/usr/bin/python3

from sys import argv, stderr
import hashlib
import struct
from datetime import datetime as dt

'''
We initially attempt to import sha3. On Python 2, this monkey-patches
the hashlib library to contain an implementation of sha3_256 (Keccak),
one of the cryptographic hashes used in the blockchain.
'''

try:
    import sha3
except ImportError as e:
    pass

sha2 = hashlib.sha256    # Merkle-Damgard
try:
    sha3 = hashlib.sha3_256  # Keccak
except AttributeError as e:
    stderr.write('\033[31;1mWarning:\033[0m SHA3 algorithm not present; ' +
    'please upgrade to Python 3.6 or pip install pysha3\n')
    sha3 = None

'''
We represent 256-bit binary blobs by padding them with a 32-bit CRC
checksum (to increase it to 288 bits) and represent them by 48 characters
of URL-safe base64.
'''

def crc32_for_byte(r):
    for j in range(8):
        r = (0 if (r & 1) else 0xEDB88320) ^ r >> 1;
    return r ^ 0xFF000000;

table = [crc32_for_byte(x) for x in range(256)]

def crc32(data):
    crc = 0
    for d in data:
        crc = table[(crc & 255) ^ d] ^ crc >> 8;
    return crc

def human_readable(data):
    '''
    This is a direct translation of the C++ code in coincludes/cryptography.h
    '''

    dig32 = list(struct.unpack('<IIIIIIII', data))
    dig32.append(crc32(data))
    digest = struct.pack('<IIIIIIIII', *dig32)
    digest = [b for b in digest]

    x = "";

    for i in range(0, 36, 3):
        a = 0;
        a += digest[i+2]; a <<= 8;
        a += digest[i+1]; a <<= 8;
        a += digest[i+0];
        for j in range(4):
            x += "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-"[a & 63];
            a >>= 6;

    return x;


def dispblock(x):

    bi = struct.unpack('<Q', x[80:88])[0]
    
    s = 'Block %d\n' % bi
    s += ('=' * 64 + '\n\n')

    s += 'Hexadecimal representation:\n'
    s += '---------------------------\n'

    l = ['prev_sha2', 'prev_sha3', 'misc_info', 'addr_full',
         'txmr_sha2', 'txmr_sha3', 'xtra_nonc', 'addr_mine']

    for (i, r) in enumerate(l):

        h = x[32*i:32*(i+1)].hex()
        s += '%s|%s\n' % (h, r)

    s += '\nMiscellaneous information:\n'
    s +=    '-------------------------\n'

    pn = struct.unpack('<Q', x[64:72])[0]
    ts = struct.unpack('<Q', x[72:80])[0]
    ds = dt.utcfromtimestamp(ts//(10**9)).strftime('%Y-%m-%d %H:%M:%S')
    ds += ('.' + str((10**9) + (ts % (10**9)))[1:])

    s += 'prev_nonce: %d\n'      % pn
    s += 'time_stamp: %d = %s\n' % (ts, ds)
    s += 'blockindex: %d\n'      % bi
    s += 'difficulty: %.4f\n'    % struct.unpack('<d', x[88:96])[0]

    s += '\nPrevious soup:\n'
    s +=    '-------------\n'

    seedroot = human_readable(x[32:64])
    prevseed = seedroot + str(pn)

    s += '#C seed: %s\n' % prevseed
    s += '#C sha2: %s\n' % sha2(prevseed.encode('utf8')).hexdigest()

    digest = sha2(prevseed.encode('utf8')).digest()
    bits = [((b >> (7-i)) & 1) for b in digest for i in range(8)]
    rle = ''.join(['bo'[b] for b in bits])
    rle = tuple([rle[i:16+i] for i in range(0, 256, 16)])
    rle = '%s$%s$%s$%s$\n%s$%s$%s$%s$\n%s$%s$%s$%s$\n%s$%s$%s$%s!\n' % rle

    s += 'x = 16, y = 16, rule = B3/S23\n'
    s += rle

    s += '\nWinner addresses:\n'
    s +=    '----------------\n'
    s += 'addr_full: %s\n' % human_readable(x[96:128])
    s += 'addr_mine: %s\n' % human_readable(x[224:256])

    s += '\nBlock hashes:\n'
    s +=    '------------\n'
    s += '%s|%s\n' % (sha2(x).hexdigest(), 'this_sha2')

    if sha3 is not None:
        s += '%s|%s\n' % (sha3(x).hexdigest(), 'this_sha3')

    s += ('\n' + ('=' * 64) + '\n')
    return s

def main():

    for filename in argv[1:]:

        with open(filename, 'rb') as f:
            x = f.read()

        lr = (len(x) & 255)
        if lr:
            x += bytes([0] * (256 - lr))

        for i in range(0, len(x), 256):
            print(dispblock(x[i:i+256]))

if __name__ == '__main__':
    main()
