#pragma once

#include <string>
#include "cryptography.h"

struct Blockheader {

    uint8_t prevblock_hashes[64];

    uint64_t prevblock_nonce;
    uint64_t timestamp;
    uint64_t blocknumber;
    uint64_t difficulty;

    uint8_t tx_merkle_hashes[64];
    uint8_t sg_merkle_hashes[64];

    uint8_t winner_address[32];


    std::string seedroot() {
        uint8_t thisblock[256];
        std::memcpy(thisblock, this, 256);
        return "l_" + apg::sha288encode(thisblock, 256);
    }

};

static_assert(sizeof(Blockheader) == 256,
    "Sanity check failed: Blockheader must be exactly 256 bytes");
