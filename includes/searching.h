#pragma once

#include <iomanip>
#include <stdio.h>

#include <atomic>
#include <thread>
#include <chrono>

#ifdef USE_OPEN_MP
#include <omp.h>
#endif

#ifndef _WIN32
#include <sys/select.h>
#include <termios.h>
// determine whether there's a keystroke waiting
int keyWaiting() {
    struct timeval tv;
    fd_set fds;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); // STDIN_FILENO is 0

    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);

    return FD_ISSET(STDIN_FILENO, &fds);
}
#endif

void populateLuts() {

    apg::best_instruction_set();
    std::vector<apg::bitworld> vbw = apg::hashsoup("", SYMMETRY);

    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::pattern pat(&lt, lt.fromplanes(vbw), RULESTRING);

    pat.advance(0, 0, 8);

}

void partialGPUSearch(std::vector<uint64_t> *vec, std::string seed, SoupSearcher *localSoup) {

    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);

    for (auto it = vec->begin(); it != vec->end(); ++it) {
        std::ostringstream ss;
        ss << (*it);
        localSoup->censusSoup(seed, ss.str(), cfier);
    }

} 

void partialSearch(uint64_t n, int m, int threadNumber, std::string seedroot,
    SoupSearcher *localSoup, uint64_t *j, std::atomic<bool> *running,
    difficul_t difficulty, std::atomic<uint64_t> *bestSoup) {

    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);

    while (*running) {

        uint64_t i = threadNumber + m * ((*j));

        if ((n > 0) && (i >= n)) { break; }

        std::ostringstream ss;
        ss << i;
        dsentry dp = localSoup->censusSoup(seedroot, ss.str(), cfier);

        if (i % 100000 == 0)
        {
            std::cout << i << " soups processed..." << std::endl;
        }

        (*j)++;

        if (bestSoup != 0) {
            if (dp.first >= difficulty) {
                (*bestSoup) = i; // this transcends the difficulty target
                (*running) = false; // we can now exit
                std::cerr << "Block won: " << seedroot << i << " contains a " << dp.second << std::endl;
                std::cerr << "Target: " << difficulty << "; attained: " << dp.first << std::endl;
            }
        }

    }

}

void threadSearch(uint64_t n, int m, std::string payoshaKey, std::string seed,
                    int local_log, std::atomic<bool> &running, bool testing,
                    difficul_t difficulty, std::atomic<uint64_t> *bestSoup, apg::DifficultyHolder *dtab) {

    SoupSearcher globalSoup;

    if (dtab != 0) {
        globalSoup.difficulties = *dtab;
    }

    populateLuts();

    std::vector<uint64_t> completed(m, 0);

    uint64_t maxcount = 0;

    while (running) {

        maxcount += n;

        #ifdef USE_OPEN_MP
        #pragma omp parallel num_threads(m)
        {
            int i = omp_get_thread_num();
            SoupSearcher localSoup;
            partialSearch(maxcount, m, i, seed, &localSoup, &(completed[i]), &running);
            #pragma omp critical
            {
                globalSoup.aggregate(&(localSoup.census), &(localSoup.alloccur));
            }
        }

        #else

        std::vector<SoupSearcher> localSoups(m);
        std::vector<std::thread> lsthreads(m);
        for (int i = 0; i < m; i++) {
            localSoups[i].difficulties = globalSoup.difficulties;
            lsthreads[i] = std::thread(partialSearch, maxcount, m, i, seed, &(localSoups[i]),
                                        &(completed[i]), &running, difficulty, bestSoup);
        }

        for (int i = 0; i < m; i++) {
            lsthreads[i].join();
            globalSoup.aggregate(&(localSoups[i].census), &(localSoups[i].alloccur));
        }

        #endif

        uint64_t totalSoups = 0;

        for (int i = 0; i < m; i++) { totalSoups += completed[i]; }

        std::cout << "----------------------------------------------------------------------" << std::endl;
        std::cout << totalSoups << " soups completed." << std::endl;
        std::cout << "Attempting to contact payosha256." << std::endl;
        std::string payoshaResponse = globalSoup.submitResults(payoshaKey, seed, totalSoups, local_log, testing);

        if (payoshaResponse.length() == 0) {
            std::cout << "Connection was unsuccessful." << std::endl;
            std::cout << "Continuing search..." << std::endl;
        } else {
            std::cout << "Connection was successful." << std::endl;
            break;
        }
    }
}

void parallelSearch(uint64_t n, int m, std::string payoshaKey, std::string seed, int local_log, std::atomic<bool> &running, bool testing) {

    threadSearch(n, m, payoshaKey, seed, local_log, running, testing, 0, 0, 0);

}



bool runSearch(int64_t n, int desired_m, std::string payoshaKey, std::string seed, int local_log, int unicount, bool testing) {

    #ifndef _WIN32
    #ifndef STDIN_SYM
    struct termios ttystate;

    if (desired_m == 0) {
        // turn on non-blocking reads
        tcgetattr(STDIN_FILENO, &ttystate);
        ttystate.c_lflag &= ~ICANON;
        ttystate.c_cc[VMIN] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
    }
    #endif
    #endif

    SoupSearcher soup;
    soup.tilesProcessed = 0;
    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);

    auto start = std::chrono::system_clock::now();
    auto overall_start = start;
    auto current = start;
    auto last_current = start;

    std::cout << "Running " << n << " soups per haul:" << std::endl;

    int64_t i = 0;
    int64_t lasti = 0;

    #ifdef USING_GPU
    apg::GpuSearcher gs(0, unicount);
    std::vector<uint64_t> vec = gs.pump(seed, 0);
    #endif

    bool finishedSearch = false;
    bool quitByUser = false;

    while ((finishedSearch == false) && (quitByUser == false)) {

        #ifndef USING_GPU
        std::ostringstream ss;

        #ifndef STDIN_SYM
        ss << i;
        #else
        bool readingrle = false;
        std::string stdin_line;
        while (std::getline(std::cin, stdin_line)) {

            if (stdin_line.length() == 0) { continue; }

            if (readingrle) { ss << "-" << stdin_line; }

            if ((stdin_line[0] == 'x') && (readingrle == false)) {
                readingrle = true;
            }

            if (stdin_line.find('!') != std::string::npos) { break; }
        }
        if (readingrle == false) { quitByUser = true; }
        #endif

        #endif

        if (quitByUser == false) {

            #ifdef USING_GPU

            int m = (desired_m == 0) ? 4 : desired_m;

            std::vector<std::vector<uint64_t> > subvecs(m);

            for (unsigned int j = 0; j < vec.size(); j++) {
                subvecs[j % m].push_back(vec[j]);
            }

            std::vector<SoupSearcher> localSoups(m);
            std::vector<std::thread> lsthreads(m);
            for (int j = 0; j < m; j++) {
                lsthreads[j] = std::thread(partialGPUSearch, &(subvecs[j]), seed, &(localSoups[j]));
            }

            i += 1000000;
            vec = gs.pump(seed, i / 1000000);

            for (int j = 0; j < m; j++) {
                lsthreads[j].join();
                soup.aggregate(&(localSoups[j].census), &(localSoups[j].alloccur));
            }

            #else
            soup.censusSoup(seed, ss.str(), cfier);
            i += 1;
            #endif

            last_current = current;
            current = std::chrono::system_clock::now();
            double elapsed =         0.001 * std::chrono::duration_cast<std::chrono::milliseconds>(current - start).count();
            double current_elapsed = 0.001 * std::chrono::duration_cast<std::chrono::milliseconds>(current - last_current).count();
            double overall_elapsed = 0.001 * std::chrono::duration_cast<std::chrono::milliseconds>(current - overall_start).count();

            if ((elapsed >= 10.0) || ((current_elapsed >= 1.0) && (i == (lasti + 1))) || (i - lasti >= 1000000)) {
                std::cout << RULESTRING << "/" << SYMMETRY << ": " << i << " soups completed (" << std::fixed << std::setprecision(3) << ((i - lasti) / elapsed) << " soups/second current, " << (i / overall_elapsed) << " overall)." << std::endl;
                lasti = i;

                start = std::chrono::system_clock::now();

                #ifndef _WIN32
                #ifndef STDIN_SYM
                if(keyWaiting()) {
                    char c = fgetc(stdin);
                    if ((c == 'q') || (c == 'Q'))
                        quitByUser = true;
                }
                #endif
                #endif
                
            }

        }

        if ((i % n == 0) || quitByUser) {

            std::cout << "----------------------------------------------------------------------" << std::endl;
            std::cout << i << " soups completed." << std::endl;
            std::cout << "Attempting to contact payosha256." << std::endl;
            std::string payoshaResponse = soup.submitResults(payoshaKey, seed, i, local_log, testing);
            if (payoshaResponse.length() == 0) {
                std::cout << "Connection was unsuccessful; continuing search..." << std::endl;
            } else {
                if (payoshaResponse == "testing") { std::cout << "testing mode" << std::endl; }
                std::cout << "\033[31;1m" << payoshaResponse << "\033[0m" << std::endl;
                std::cout << "Connection was successful; starting new search..." << std::endl;
                finishedSearch = true;
            }
            std::cout << "----------------------------------------------------------------------" << std::endl;
        }

    }

    std::cerr << "Tiles processed: " << soup.tilesProcessed << std::endl;
    
    #ifndef _WIN32
    #ifndef STDIN_SYM
    // turn on blocking reads
    if (desired_m == 0) {
        tcgetattr(STDIN_FILENO, &ttystate);
        ttystate.c_lflag |= ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
    }
    #endif
    #endif

    return quitByUser;

}


