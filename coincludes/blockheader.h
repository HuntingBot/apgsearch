#pragma once

#include <string>
#include <sstream>
#include "cryptography.h"

// We impose the minimum = initial difficulty to be 7 CPU-hours per block.
// This is the equilibrium difficulty if there are 42 cores mining (as we
// target a block rate of one block per 10 minutes).
#define MIN_LOG_DIFFICULTY 0x407a400000000000ull

namespace cgold {

    struct Blockheader {

        // The first 80 bytes are provided by the miner of the prevblock:
        uint8_t prevblock_hashes[64]; // SHA-256 concat Keccak-256 of last BH
        uint64_t prevblock_nonce;
        uint64_t prevblock_time; // in nanoseconds since the Unix epoch

        // The next 16 bytes are calculable from previous blocks:
        uint64_t blocknumber;
        uint64_t log_difficulty;

        // The next 96 bytes are provided by the fullnode:
        uint8_t address_fullnode[32]; // receives transaction fees
        uint8_t tx_merkle_hashes[64]; // SHA-256 concat Keccak-256 of MT root

        // The final 64 bytes are provided by the miner of the block:
        uint8_t extranonce[32];
        uint8_t address_miner[32]; // receives block reward


        // Returns the 48-character seedroot of the block:
        std::string seedroot() {
            uint8_t thisblock[256];
            std::memcpy(thisblock, this, 256);
            return sha288encode(thisblock, 256);
        }

        // Returns the (49-68)-character seed of the prevblock:
        std::string prevblock_seed() {
            std::stringstream ss;
            ss << human_readable(prevblock_hashes + 32);
            ss << prevblock_nonce;
            return ss.str();
        }

        Blockheader() = default;

        Blockheader(uint64_t genesis_nonce, uint64_t genesis_time) {

            std::memset(this, 0, 256);

            uint8_t genesis_info[64] = {84, 104, 101, 32, 82, 101, 103, 105,
            115, 116, 101, 114, 32, 48, 54, 47, 77, 97, 114, 47, 50, 48,
            49, 56, 32, 71, 111, 111, 103, 108, 101, 32, 114, 101, 118, 101,
            97, 108, 115, 32, 55, 50, 45, 113, 117, 98, 105, 116, 32, 113,
            117, 97, 110, 116, 117, 109, 32, 99, 104, 105, 112, 13, 10, 0};
            std::memcpy(prevblock_hashes, genesis_info, 64);

            prevblock_nonce = genesis_nonce;
            prevblock_time = genesis_time;
            blocknumber = 0;
            log_difficulty = MIN_LOG_DIFFICULTY;

        }

        Blockheader(uint64_t genesis_time) : Blockheader(442350544, genesis_time) {}

        double get_difficulty() {

            double x;
            std::memcpy(&x, &log_difficulty, 8);
            return x;

        }

        Blockheader(const Blockheader &prevblock, uint64_t n, uint64_t t) {

            std::memset(this, 0, 256);
            uint8_t prevblock_contents[256];
            std::memcpy(prevblock_contents, &prevblock, 256);
            hashpair(prevblock_contents, 256, prevblock_hashes);

            prevblock_nonce = n;
            prevblock_time = t;
            blocknumber = prevblock.blocknumber + 1;

            // We update the difficulty to target a 10-minute block time:
            int64_t actual_time = ((int64_t) (t - prevblock.prevblock_time));
            int64_t desired_time = 600000000000ll;
            int64_t max_change  =  200000000000ll;
            int64_t min_change  = -200000000000ll;
            int64_t earlyness = desired_time - actual_time;
            earlyness = (earlyness < min_change) ? min_change : earlyness;
            earlyness = (earlyness > max_change) ? max_change : earlyness;
            log_difficulty = prevblock.log_difficulty + earlyness * 100;
            if (log_difficulty < MIN_LOG_DIFFICULTY) {
                log_difficulty = MIN_LOG_DIFFICULTY;
            }
        }

    };

    static_assert(sizeof(Blockheader) == 256,
        "Sanity check failed: Blockheader must be exactly 256 bytes");

}
