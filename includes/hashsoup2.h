#pragma once

#include "../lifelib/soup/hashsoup.h"

/*
 * Produce a new seed based on the original seed, current time and PID:
 */
std::string reseed(std::string seed) {

    std::ostringstream ss;
    ss << seed << " " << clock() << " " << time(NULL) << " " << getpid();

    std::string prehash = ss.str();

    unsigned char digest[SHA256::DIGEST_SIZE];
    memset(digest,0,SHA256::DIGEST_SIZE);

    SHA256 ctx = SHA256();
    ctx.init();
    ctx.update( (unsigned char*)prehash.c_str(), prehash.length());
    ctx.final(digest);

    const char alphabet[] = "abcdefghijkmnpqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789";

    std::ostringstream newseed;
    newseed << "l_";
    for (int i = 0; i < 12; i++) {
        newseed << alphabet[digest[i] % 56];
    }

    return newseed.str();

}

