#pragma once

#include <stdint.h>
#include <string>
#include <iostream>
#include <utility>

#include "sha256.h"
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

std::string pubkey2addr(const unsigned char* pkbytes) {

    uint32_t dig32[9];
    uint8_t digest[32];
    memset(digest, 0, 32);

    SHA256 ctx = SHA256();
    ctx.init();
    ctx.update(pkbytes, CRYPTO_PUBLICKEYBYTES);
    ctx.final(digest);

    memset(dig32, 0, 36);

    std::memcpy(dig32, digest, 32);
    crc32(digest, 32, dig32 + 8);

    std::string address = "'";
    for (int i = 0; i < 9; i++) {
        address += base85encode(dig32[i]);
    }
    address += "'";

    return address;
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

void addrgen() {
    std::string password;
    while (std::getline(std::cin, password)) {
        std::cout << password2addr(password) << std::endl;
    }
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

uint32_t verify_crc32(std::string addr) {

    std::string address = addr;

    if ((address.length()) && (address[0] == 0x27)) {
        address = address.substr(1);
    }

    uint32_t dig32[9];
    memset(dig32, 0, 36);
    if (base85decode(dig32, address, 9) != 9) { return -1; }
    uint8_t digest[32];
    std::memcpy(digest, dig32, 32);
    uint32_t crc = 0;
    crc32(digest, 32, &crc);
    
    return (crc ^ dig32[8]);

}

} // namespace apg
