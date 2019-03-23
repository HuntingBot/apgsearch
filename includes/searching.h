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

void partialBalancedSearch(std::vector<uint64_t> *vec, std::string seed, SoupSearcher *localSoup,
                            std::atomic<bool> *running, std::atomic<uint64_t> *idx, std::atomic<uint64_t> *ts,
                            difficul_t difficulty, std::atomic<uint64_t> *bestSoup) {

    uint64_t maxidx = vec->size();

    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);

    while (*running) {

        uint64_t curridx = ((*idx)++); // atomic increment
        if (curridx >= maxidx) { break; }

        uint64_t suffix = (*vec)[curridx];

        std::ostringstream ss;
        ss << suffix;
        dsentry dp = localSoup->censusSoup(seed, ss.str(), cfier);
        (*ts)++;

        if (bestSoup != 0) {
            if (dp.first >= difficulty) {
                (*bestSoup) = suffix; // this transcends the difficulty target
                (*running) = false; // we can now exit
                std::cerr << "Block won: " << seed << suffix << " contains a " << dp.second << std::endl;
                std::cerr << "Target: " << difficulty << "; attained: " << dp.first << std::endl;
            }
        }
    }
}

struct CpuSearcher {

    std::vector<uint64_t> pump(std::string seed, uint64_t j) {

        (void) seed;

        std::vector<uint64_t> vec;

        for (uint64_t i = 0; i < 100000; i++) {
            vec.push_back(j * 100000 + i);
        }

        return vec;
    }

};

std::vector<uint64_t> narrow(std::vector<uint64_t> orig, uint64_t lb, uint64_t ub) {

    std::vector<uint64_t> narrowed;

    for (auto it = orig.begin(); it != orig.end(); ++it) {

        uint64_t x = *it;

        if ((lb <= x) && (x < ub)) { narrowed.push_back(x); }

    }

    return narrowed;

}

std::string retrieveSeed(uint64_t i, std::atomic<bool> *running) {

    std::ostringstream ss;

    #ifndef STDIN_SYM
    ss << i;
    (void) running;
    #else
    (void) i;
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
    if (readingrle == false) { (*running) = false; }
    #endif

    return ss.str();

}

void perpetualSearch(uint64_t n, int m, bool interactive, std::string payoshaKey, std::string seed,
                        int unicount, int local_log, std::atomic<bool> &running, bool testing,
                        difficul_t difficulty, std::atomic<uint64_t> *bestSoup, apg::DifficultyHolder *dtab) {
    /*
    Unifies several similar functions into one simpler one.
    */

    SoupSearcher globalSoup;
    globalSoup.tilesProcessed = 0;
    if (dtab != 0) { globalSoup.difficulties = *dtab; }

    #ifndef _WIN32
    struct termios ttystate;

    if (interactive) {
        // turn on non-blocking reads
        tcgetattr(STDIN_FILENO, &ttystate);
        ttystate.c_lflag &= ~ICANON;
        ttystate.c_cc[VMIN] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
    }
    #endif

    uint64_t i = 0;
    uint64_t lasti = 0;

    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);

    populateLuts();

    std::cout << "Running " << n << " soups per haul:" << std::endl;

    #ifdef USING_GPU
    uint64_t epoch_size = 1000000;
    apg::GpuSearcher gs(0, unicount);
    #else
    uint64_t epoch_size = 100000;
    CpuSearcher gs; (void) unicount;
    #endif

    std::vector<uint64_t> vec = gs.pump(seed, 0);

    auto start = std::chrono::system_clock::now();
    auto overall_start = start;
    auto current = start;
    auto last_current = start;

    uint64_t maxcount = n;

    while (running) {

        std::vector<SoupSearcher> localSoups(m);
        std::vector<std::thread> lsthreads(m);

        if (m == 0) {
            std::string suffix = retrieveSeed(i, &running);
            globalSoup.censusSoup(seed, suffix, cfier);
            i += 1;
        } else {
            std::atomic<uint64_t> idx(0);
            std::atomic<uint64_t> ts(0);

            auto nvec = narrow(vec, i, maxcount);

            for (int j = 0; j < m; j++) {
                localSoups[j].difficulties = globalSoup.difficulties;
                lsthreads[j] = std::thread(partialBalancedSearch, &(nvec), seed, &(localSoups[j]),
                                            &running, &idx, &ts, difficulty, bestSoup);
            }

            uint64_t newi = ((i / epoch_size) + 1) * epoch_size;
            if (newi < maxcount) {
                vec = gs.pump(seed, newi / epoch_size);
            } else {
                newi = maxcount;
            }

            for (int j = 0; j < m; j++) {
                lsthreads[j].join();
                globalSoup.aggregate(&(localSoups[j].census), &(localSoups[j].alloccur));
            }

            if (running || (ts == nvec.size())) {
                i = newi;
            } else {
                double diff = newi - i;
                uint64_t estim = (diff * ts) / nvec.size();
                i += estim;
            }
        }

        last_current = current;
        current = std::chrono::system_clock::now();
        double elapsed =         0.001 * std::chrono::duration_cast<std::chrono::milliseconds>(current - start).count();
        double current_elapsed = 0.001 * std::chrono::duration_cast<std::chrono::milliseconds>(current - last_current).count();
        double overall_elapsed = 0.001 * std::chrono::duration_cast<std::chrono::milliseconds>(current - overall_start).count();

        bool finished = (!running) || (i == maxcount);

        if (finished || (elapsed >= 10.0) || ((current_elapsed >= 1.0) && (i == (lasti + 1))) || (i - lasti >= 1000000)) {
            std::cout << RULESTRING << "/" << SYMMETRY << ": " << i << " soups completed (" << std::fixed << std::setprecision(3) << ((i - lasti) / elapsed) << " soups/second current, " << (i / overall_elapsed) << " overall)." << std::endl;
            lasti = i;

            start = std::chrono::system_clock::now();

            #ifndef _WIN32
            if (interactive && keyWaiting()) {
                char c = fgetc(stdin);
                if ((c == 'q') || (c == 'Q')) { running = false; }
            }
            #endif
        }

        if (finished) {
            std::cout << "----------------------------------------------------------------------" << std::endl;
            std::cout << i << " soups completed." << std::endl;
            std::cout << "Attempting to contact payosha256." << std::endl;
            std::string payoshaResponse = globalSoup.submitResults(payoshaKey, seed, i, local_log, testing);

            if (payoshaResponse.length() == 0) {
                std::cout << "Connection was unsuccessful." << std::endl;
            } else {
                std::cout << "Connection was successful." << std::endl;
                break;
            }

            if (running) {
                std::cout << "Continuing search..." << std::endl;
                vec = gs.pump(seed, i / epoch_size);
                maxcount += n;
            }
        }
    }

    #ifndef _WIN32
    if (interactive) {
        // turn on blocking reads
        tcgetattr(STDIN_FILENO, &ttystate);
        ttystate.c_lflag |= ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
    }
    #endif

}

void perpetualSearch(uint64_t n, int m, bool interactive, std::string payoshaKey, std::string seed,
                        int unicount, int local_log, std::atomic<bool> &running, bool testing) {

    perpetualSearch(n, m, interactive, payoshaKey, seed, unicount, local_log, running, testing, 0, 0, 0);

}
