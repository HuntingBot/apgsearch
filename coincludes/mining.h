#pragma once

#include <atomic>
#include <fstream>

#include "blockheader.h"
#include "nanotime.h"

int nprocessors_onln() {

    std::ifstream instream("/proc/cpuinfo");
    std::string line;

    int nproc = 0;

    while (std::getline(instream, line)) {
        if (line.empty()) { continue; }
        if ((line.length() > 8) && (line.substr(0, 9) == "processor")) { nproc += 1; }
    }

    return nproc;

}

cgold::Blockheader parallelMine(int m, std::string payoshaKey,
                                cgold::Blockheader &bh,
                                std::atomic<bool> &running) {

    difficul_t difficulty = bh.get_difficulty();
    std::string seedroot = bh.seedroot();

    const uint64_t invalid_soup = -1;

    std::atomic<uint64_t> bestSoup(invalid_soup);

    threadSearch(0, m, payoshaKey, seedroot, 0, running, 0, difficulty, &bestSoup, 0);

    cgold::Blockheader new_bh;
    new_bh.clear();

    if (bestSoup != invalid_soup) {

        new_bh = cgold::Blockheader(bh, bestSoup, cgold::nanotime());

    }
 
    return new_bh;

}

int greedy_mine(int argc, char *argv[]) {

    if (apg::rule2int(RULESTRING) != 0) {
        std::cerr << "Abort: apgsearch rule does not match lifelib rule" << std::endl;
        return 1;
    }

    // Default values:
    std::string payoshaKey = "#anon";
    int parallelisation = 0;
    std::string addr = "";
    std::string filename = "";

    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-k") == 0) {
            payoshaKey = argv[i+1];
        } else if (strcmp(argv[i], "-p") == 0) {
            parallelisation = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-a") == 0) {
            addr = argv[i+1];
        } else if (strcmp(argv[i], "-f") == 0) {
            filename = argv[i+1];
        }
    }

    if (addr.length() != 48) {
        std::cerr << "Abort: target address must have 48 characters." << std::endl;
        return 1;
    }

    if (cgold::human_unreadable(addr, 0) != 0) {
        std::cerr << "Abort: target address fails cyclic redundancy check." << std::endl;
        return 1;
    }

    if (parallelisation == 0) {
        parallelisation = nprocessors_onln();
        std::cerr << "Warning: using all " << parallelisation << " cores" << std::endl;
    }

    cgold::Blockheader currentBlock(cgold::nanotime());

    if (filename == "") {
        std::cerr << "Warning: beginning from genesis block" << std::endl;
    } else {
        currentBlock.load_block(filename);
    }

    while (true) {

        uint32_t check1 = currentBlock.include_transactions(cgold::bytevec(), addr);
        uint32_t check2 = currentBlock.include_tail(addr);

        std::cerr << "check1: " << check1 << ", check2: " << check2 << std::endl;

        std::cerr << "Current difficulty: " << currentBlock.get_difficulty() << std::endl;

        currentBlock.save_block();

        std::atomic<bool> running(true);

        currentBlock = parallelMine(parallelisation, payoshaKey, currentBlock, running);
    }
}

dsentry get_block_difficulty(cgold::Blockheader &block, SoupSearcher &ss, apg::classifier &cfier) {

    std::string seed = block.prevblock_seed();
    dsentry p = ss.censusSoup(seed, "", cfier);
    return p;

}

bool verify_is_successor(cgold::Blockheader &prevblock, cgold::Blockheader &currblock, SoupSearcher &ss, apg::classifier &cfier) {

    // Verify cryptography:
    cgold::Blockheader alt_currblock(prevblock, currblock.prevblock_nonce, currblock.prevblock_time);
    auto comparator = memcmp((void*) &currblock, (void*) &alt_currblock, 96);
    if (comparator != 0) {
        std::cerr << "Warning: blocks do not chain" << std::endl;
        return false;
    }

    // Verify proof-of-work:
    dsentry p = get_block_difficulty(currblock, ss, cfier);
    difficul_t target_difficulty = prevblock.get_difficulty();
    if (p.first < target_difficulty) {
        std::cerr << "Warning: Object " << p.second << " has a difficulty of " << p.first;
        std::cerr << " which falls short of " << target_difficulty << std::endl;
        return false;
    }

    return true;

}

int verify_blocks(int argc, char* argv[]) {

    SoupSearcher ss;
    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);
    cgold::Blockheader last_block;

    int errors = 0;

    for (int i = 1; i < argc; i++) {
        std::string filename = argv[i];
        cgold::Blockheader this_block;
        this_block.load_block(filename);
        if (i > 1) {
            bool good = verify_is_successor(last_block, this_block, ss, cfier);
            if (!good) {
                errors += 1;
                std::cerr << "Block " << filename << " is a non-sequitur." << std::endl;
            }
        }
        last_block = this_block;
    }

    return errors;
}
