#!/usr/bin/python3

from sys import argv, stderr
import hashlib
import struct
from datetime import datetime as dt

from collections import OrderedDict

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
        if not isinstance(d, int):
            d = ord(d)
        crc = table[(crc & 255) ^ d] ^ crc >> 8;
    return crc

def human_readable(data):
    '''
    This is a direct translation of the C++ code in coincludes/cryptography.h
    '''

    dig32 = list(struct.unpack('<IIIIIIII', data))
    dig32.append(crc32(data))
    digest = struct.pack('<IIIIIIIII', *dig32)
    digest = [(b if isinstance(b, int) else ord(b)) for b in digest]

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

def block_rle(x):

    pn = struct.unpack('<Q', x[64:72])[0]
    seedroot = human_readable(x[32:64])
    prevseed = seedroot + str(pn)

    s  = '#C seed: %s\n' % prevseed
    s += '#C sha2: %s\n' % sha2(prevseed.encode('utf8')).hexdigest()

    digest = [(b if isinstance(b, int) else ord(b)) for b in sha2(prevseed.encode('utf8')).digest()]
    bits = [((b >> (7-i)) & 1) for b in digest for i in range(8)]
    rle = ''.join(['bo'[b] for b in bits])
    rle = tuple([rle[i:16+i] for i in range(0, 256, 16)])
    rle = '%s$%s$%s$%s$\n%s$%s$%s$%s$\n%s$%s$%s$%s$\n%s$%s$%s$%s!\n' % rle

    s += 'x = 16, y = 16, rule = B3/S23\n'
    s += rle

    return s

def block_info(x):

    bi = struct.unpack('<Q', x[80:88])[0]
    od = OrderedDict()
    od['idx'] = bi

    od['hex'] = OrderedDict()
    l = ['prev_sha2', 'prev_sha3', 'misc_info', 'addr_full',
         'txmr_sha2', 'txmr_sha3', 'xtra_nonc', 'addr_mine']

    for (i, r) in enumerate(l):

        try:
            # Python 3:
            h = x[32*i:32*(i+1)].hex()
        except AttributeError:
            # Python 2:
            h = ''.join("{:02x}".format(ord(c)) for c in x[32*i:32*(i+1)])

        od['hex'][r] = h

    pn = struct.unpack('<Q', x[64:72])[0]
    ts = struct.unpack('<Q', x[72:80])[0]
    ds = dt.utcfromtimestamp(ts//(10**9)).strftime('%Y-%m-%dT%H:%M:%S')
    ds += ('.' + str((10**9) + (ts % (10**9)))[1:])

    od['misc'] = OrderedDict()
    od['misc']['prev_nonce'] = pn
    od['misc']['time_stamp'] = ts
    od['misc']['block_time'] = ds
    od['misc']['blockindex'] = bi
    od['misc']['difficulty'] = struct.unpack('<d', x[88:96])[0]

    od['addresses'] = OrderedDict()
    od['addresses']['addr_full'] = human_readable(x[96:128])
    od['addresses']['addr_mine'] = human_readable(x[224:256])

    od['hashes'] = OrderedDict()
    od['hashes']['this_sha2'] = sha2(x).hexdigest()
    if sha3 is not None:
        od['hashes']['this_sha3'] = sha3(x).hexdigest()

    return od

def display_block(x):

    info = block_info(x)

    s = 'Block %d\n' % info['idx']
    s += ('=' * 64 + '\n\n')

    for (t, k) in [('Hexadecimal representation', 'hex'),
                   ('Miscellaneous information', 'misc'),
                   ('Winner addresses', 'addresses'),
                   ('Block hashes', 'hashes'),
                   ('Previous soup', 'nonkey')]:

        s += ('\n%s:\n' % t)
        s += (('-' * len(t)) + '-\n')
        if k in info:
            for item in info[k].items():
                s += '%s|%s\n' % item

    s += block_rle(x)

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
            print(display_block(x[i:i+256]))

if __name__ == '__main__':
    main()
