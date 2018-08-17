#pragma once

#include <atomic>

#include "blockheader.h"
#include "nanotime.h"

cgold::Blockheader parallelMine(int m, std::string payoshaKey,
                                cgold::Blockheader &bh,
                                std::atomic<bool> &running) {

    difficul_t difficulty = bh.get_difficulty();
    std::string seedroot = bh.seedroot();

    SoupSearcher globalSoup;

    // Ensure the lookup tables are populated by the main thread:
    populateLuts();

    int64_t total_soups = 0;

    const uint64_t invalid_soup = -1;

    std::atomic<uint64_t> bestSoup(invalid_soup);

    #pragma omp parallel num_threads(m)
    {
        int threadNumber = omp_get_thread_num();

        SoupSearcher localSoup;
        apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
        apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);

        int64_t j = 0;

        while (running) {

            uint64_t i = threadNumber + m * (j++);

            std::ostringstream ss;
            ss << i;
            dsentry dp = localSoup.censusSoup(seedroot, ss.str(), cfier);

            if (dp.first >= difficulty) {
                bestSoup = i; // this transcends the difficulty target
                running = false; // we can now exit
            }

        }

        #pragma omp critical
        {
            total_soups += j;
            globalSoup.aggregate(&(localSoup.census), &(localSoup.alloccur));
        }
    }

    cgold::Blockheader new_bh;
    new_bh.clear();

    if (bestSoup != invalid_soup) {

        new_bh = cgold::Blockheader(bh, bestSoup, cgold::nanotime());

    }
 
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << total_soups << " soups completed." << std::endl;

    if (payoshaKey != "") {
        std::cout << "Attempting to contact payosha256." << std::endl;
        std::string payoshaResponse = globalSoup.submitResults(payoshaKey, seedroot, total_soups, false, 0);
    }
    
    return new_bh;

}

