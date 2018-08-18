#pragma once

#include <atomic>

#include "blockheader.h"
#include "nanotime.h"

cgold::Blockheader parallelMine(int m, std::string payoshaKey,
                                cgold::Blockheader &bh,
                                std::atomic<bool> &running) {

    difficul_t difficulty = bh.get_difficulty();
    std::string seedroot = bh.seedroot();

    const uint64_t invalid_soup = -1;

    std::atomic<uint64_t> bestSoup(invalid_soup);

    threadSearch(0, m, payoshaKey, seedroot, 0, running, difficulty, &bestSoup, 0);

    cgold::Blockheader new_bh;
    new_bh.clear();

    if (bestSoup != invalid_soup) {

        new_bh = cgold::Blockheader(bh, bestSoup, cgold::nanotime());

    }
 
    return new_bh;

}

void greedy_mine(int m, std::string payoshaKey, std::string addr) {

    cgold::Blockheader currentBlock(cgold::nanotime());

    while (true) {

        uint32_t check1 = currentBlock.include_transactions(cgold::bytevec(), addr);
        uint32_t check2 = currentBlock.include_tail(addr);

        std::cerr << "check1: " << check1 << ", check2: " << check2 << std::endl;

        currentBlock.save_block();

        std::atomic<bool> running(true);

        currentBlock = parallelMine(m, payoshaKey, currentBlock, running);
    }

}

