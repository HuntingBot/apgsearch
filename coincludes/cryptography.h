#pragma once

#include <stdint.h>
#include <string>
#include <iostream>
#include <utility>

#include "../lifelib/soup/sha256.h"

extern "C"
{
#include "sha3/sha3.h"
#include "../dilithium/api.h"
}

namespace cgold {

    typedef std::vector<uint8_t> bytevec;

    bytevec sign_message(const bytevec &message, const bytevec &password) {

        unsigned long long smlen;

        bytevec tx(CRYPTO_PUBLICKEYBYTES + message.size() + CRYPTO_BYTES);
        unsigned char sk[CRYPTO_SECRETKEYBYTES];

        uint8_t digest[32];
        memset(digest, 0, 32);

        SHA256 ctx = SHA256();
        ctx.init();
        ctx.update(password.data(), password.size());
        ctx.final(digest);

        crypto_sign_keypair(digest, &tx[0], sk);
        crypto_sign(&tx[CRYPTO_PUBLICKEYBYTES], &smlen, message.data(), message.size(), sk);
        tx.resize(CRYPTO_PUBLICKEYBYTES + smlen);

        return tx;

    }

    // CRC-32 implementation from http://home.thep.lu.se/~bjorn/crc/crc32_simple.c

    uint32_t crc32_for_byte(uint32_t r) {
      for(int j = 0; j < 8; ++j)
        r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
      return r ^ (uint32_t)0xFF000000L;
    }

    void crc32(const uint8_t *data, size_t n_bytes, uint32_t* crc) {
      static uint32_t table[0x100];
      if(!*table)
        for(size_t i = 0; i < 0x100; ++i)
          table[i] = crc32_for_byte(i);
      for(size_t i = 0; i < n_bytes; ++i)
        *crc = table[(uint8_t)* crc ^ data[i]] ^ *crc >> 8;
    }

    uint32_t human_unreadable(std::string address, uint8_t *binaddr) {

        if (address.length() != 48) { return -1; }
        uint8_t digest[36];

        for (int i = 0; i < 12; i++) {
            uint32_t a = 0;
            for (int j = 4*i+3; j >= 4*i; j--) {
                char c = address[j];
                a <<= 6;
                if ((c >= '0') && (c <= '9')) {
                    a += ((c - '0') + 0);
                } else if ((c >= 'a') && (c <= 'z')) {
                    a += ((c - 'a') + 10);
                } else if ((c >= 'A') && (c <= 'Z')) {
                    a += ((c - 'A') + 36);
                } else if (c == '_') {
                    a += 62;
                } else if (c == '-') {
                    a += 63;
                }
            }
            digest[3*i]   = (a & 255);
            digest[3*i+1] = ((a >> 8) & 255);
            digest[3*i+2] = ((a >> 16) & 255);
        }

        uint32_t crc = 0;
        uint32_t crc2 = 0;
        std::memcpy(&crc2, digest + 32, 4);

        crc32(digest, 32, &crc);
        
        uint32_t check = (crc ^ crc2);

        if ((check == 0) && (binaddr != 0)) {
            std::memcpy(binaddr, digest, 32);
        }

        return check;

    }

    uint32_t verify_crc32(std::string addr) {
        return human_unreadable(addr, 0);
    }

    std::string human_readable(const uint8_t* data) {

        uint32_t dig32[9];
        memset(dig32, 0, 36);
        std::memcpy(dig32, data, 32);
        crc32(data, 32, dig32 + 8);

        uint8_t digest[36];
        std::memcpy(digest, dig32, 36);

        std::string x = "";

        for (int i = 0; i < 36; i += 3) {
            uint64_t a = 0;
            a += digest[i+2]; a <<= 8;
            a += digest[i+1]; a <<= 8;
            a += digest[i+0];
            for (int j = 0; j < 4; j++) {
                x += "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-"[a & 63];
                a >>= 6;
            }
        }

        return x;
    }

    void hashpair(const unsigned char* data, size_t nbytes, uint8_t* output) {
        /*
        * Produce a 64-byte hash by concatenating a SHA2-256 and a SHA3-256
        * of the input data. As Keccak and the Merkle-Damgard construction
        * are very dissimilar, it is doubly unlikely that someone could
        * perform a second-preimage attack on both hashes simultaneously.
        */

        memset(output, 0, 64);
        SHA256 ctx = SHA256();
        ctx.init();
        ctx.update(data, nbytes);
        ctx.final(output);
        sha3((void*) data, nbytes, (void*) (output + 32), 32);
    }

    std::string sha288encode(const unsigned char* data, size_t nbytes) {
        /*
        * Produces a SHA3-256 hash, appends a CRC-32 checksum, and converts
        * the resulting 36 bytes into a 48-character URL-safe representation.
        *
        * Note: this only offers the same security level as SHA3-256; the
        * extra 32 bits function simply to ensure the hash has not been
        * mistyped.
        */

        uint8_t digest[32];
        memset(digest, 0, 32);
        sha3((void*) data, nbytes, (void*) digest, 32);

        return human_readable(digest);
    }

    std::string pubkey2addr(const unsigned char* pkbytes) {

        return sha288encode(pkbytes, CRYPTO_PUBLICKEYBYTES);
    }

    std::string password2addr(const uint8_t *password_data, const size_t password_length) {

        unsigned char sk[CRYPTO_SECRETKEYBYTES];
        unsigned char pk[CRYPTO_PUBLICKEYBYTES];

        uint8_t digest[32];
        memset(digest, 0, 32);

        SHA256 ctx = SHA256();
        ctx.init();
        ctx.update(password_data, password_length);
        ctx.final(digest);

        crypto_sign_keypair(digest, pk, sk);
        std::string address = pubkey2addr(pk);

        return address;

    }

    std::string password2addr(const bytevec &password) {

        return password2addr(password.data(), password.size());

    }

    std::string password2addr(const std::string &password) {

        return password2addr((uint8_t*) password.c_str(), password.size());

    }

    bytevec unsign_message(bytevec &tx, uint8_t* output) {

        unsigned long long smlen = tx.size() - CRYPTO_PUBLICKEYBYTES;
        unsigned long long mlen = 0;
        bytevec m2(tx.size() + CRYPTO_BYTES);
        int ret = crypto_sign_open(m2.data(), &mlen, &tx[CRYPTO_PUBLICKEYBYTES], smlen, &tx[0]);

        bytevec message;

        if (ret == 0) {
            message.assign(m2.data(), m2.data() + mlen);
            sha3((void*) &tx[0], CRYPTO_PUBLICKEYBYTES, (void*) output, 32);
        }

        return message;

    }

    void addrgen() {
        std::string password;
        while (std::getline(std::cin, password)) {
            std::string address = password2addr(password);
            std::cout << address << " check=" << verify_crc32(address) << std::endl;
        }
    }

}
