#pragma once

#include <stdint.h>
#include <string>
#include <iostream>
#include <utility>

#include "../includes/sha256.h"
extern "C"
{
#include "../dilithium/api.h"
}

namespace apg {

    std::vector<unsigned char> sign_message(const std::string &message, const std::string &password) {

        unsigned long long smlen;

        std::vector<unsigned char> tx(CRYPTO_PUBLICKEYBYTES + message.length() + CRYPTO_BYTES);
        unsigned char sk[CRYPTO_SECRETKEYBYTES];

        uint8_t digest[32];
        memset(digest, 0, 32);

        SHA256 ctx = SHA256();
        ctx.init();
        ctx.update( (unsigned char*) password.c_str(), password.length());
        ctx.final(digest);

        crypto_sign_keypair(digest, &tx[0], sk);
        crypto_sign(&tx[CRYPTO_PUBLICKEYBYTES], &smlen, (unsigned char*) message.c_str(), message.length(), sk);
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

        uint8_t digest[36];
        crc32(digest, 32, dig32 + 8);
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

    std::string sha288encode(const unsigned char* data, size_t nbytes) {
        /*
        * Produces a SHA-256 hash, appends a CRC-32 checksum, and converts
        * the resulting 36 bytes into a 48-character URL-safe representation.
        *
        * Note: this only offers the same security level as SHA-256; the
        * extra 32 bits function simply to ensure the hash has not been
        * mistyped.
        */

        uint8_t digest[36];
        memset(digest, 0, 32);

        SHA256 ctx = SHA256();
        ctx.init();
        ctx.update(data, nbytes);
        ctx.final(digest);

        return human_readable(digest);
    }

    std::string pubkey2addr(const unsigned char* pkbytes) {

        return sha288encode(pkbytes, CRYPTO_PUBLICKEYBYTES);
    }

    std::string password2addr(const std::string &password) {

        unsigned char sk[CRYPTO_SECRETKEYBYTES];
        unsigned char pk[CRYPTO_PUBLICKEYBYTES];

        uint8_t digest[32];
        memset(digest, 0, 32);

        SHA256 ctx = SHA256();
        ctx.init();
        ctx.update( (unsigned char*) password.c_str(), password.length());
        ctx.final(digest);

        crypto_sign_keypair(digest, pk, sk);
        std::string address = pubkey2addr(pk);

        return address;

    }

    std::pair<std::string, std::string> unsign_message(std::vector<unsigned char> &tx) {

        unsigned long long smlen = tx.size() - CRYPTO_PUBLICKEYBYTES;
        unsigned long long mlen = 0;
        unsigned char m2[tx.size() + CRYPTO_BYTES];
        int ret = crypto_sign_open(m2, &mlen, &tx[CRYPTO_PUBLICKEYBYTES], smlen, &tx[0]);

        std::string message;
        std::string address;

        if (ret == 0) {
            message.assign((char*) m2, mlen);
            address = pubkey2addr((unsigned char*) &tx[0]);
        }

        return std::pair<std::string, std::string>(message, address);

    }

    void addrgen() {
        std::string password;
        while (std::getline(std::cin, password)) {
            std::string address = password2addr(password);
            std::cout << address << " check=" << verify_crc32(address) << std::endl;
        }
    }

} // namespace apg
