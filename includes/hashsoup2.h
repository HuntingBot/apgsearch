#pragma once
#include "../lifelib/soup/hashsoup.h"

#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <unistd.h>

std::string reseed(const std::string& seed) {

    std::ostringstream ss;
    ss << seed;

    // Stack pointer:
    uint64_t sp; asm( "mov %%rsp, %0" : "=rm" ( sp )); ss << " " << sp;

    // Clock:
    ss << " " << clock();

    // Current time:
    ss << " " << time(NULL);

    // Process ID:
    ss << " " << getpid();

    std::string prehash = std::move(ss).str();

    unsigned char digest[SHA256::DIGEST_SIZE];
    memset(digest,0,SHA256::DIGEST_SIZE);

    SHA256 ctx = SHA256();
    ctx.init();
    ctx.update( (unsigned char*)prehash.c_str(), prehash.length());
    ctx.final(digest);

    const char alphabet[] = "abcdefghijkmnpqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    std::string newseed = "k_123456789012";
    for (int i = 0; i < 12; i++) {
        newseed[i+2] = alphabet[digest[i] % 56];
    }
    return newseed;

}

