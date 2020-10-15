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
                            std::atomic<bool> *running, std::atomic<uint64_t> *idx, std::atomic<uint64_t> *ts) {

    uint64_t maxidx = vec->size();

    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);

    while (*running) {

        uint64_t curridx = ((*idx)++); // atomic increment
        if (curridx >= maxidx) { break; }

        uint64_t suffix = (*vec)[curridx];

        localSoup->censusSoup(seed, strConcat(suffix), cfier);
        (*ts)++;

    }
}

struct CpuSearcher {

    void pump(const std::string& seed, uint64_t j, std::vector<uint64_t> &vec) {

        (void) seed;

        for (uint64_t i = 0; i < 100000; i++) {
            vec.push_back(j * 100000 + i);
        }
    }

};

std::vector<uint64_t> narrow(const std::vector<uint64_t>& orig, uint64_t lb, uint64_t ub) {

    std::vector<uint64_t> narrowed;
    for (uint64_t x : orig) {
        if (lb <= x && x < ub) {
            narrowed.push_back(x);
        }
    }
    return narrowed;

}

#ifdef STDIN_SYM
std::string retrieveSeed(std::atomic<bool> *running) {
    std::string seed;
    bool readingrle = false;
    std::string stdin_line;
    while (std::getline(std::cin, stdin_line)) {
        if (stdin_line.empty()) {
            continue;
        }
        if (readingrle) {
            seed += '-';
            seed += stdin_line;
        }
        if ((stdin_line[0] == 'x') && !readingrle) {
            readingrle = true;
        }
        if (stdin_line.find('!') != std::string::npos) {
            break;
        }
    }
    if (!readingrle) {
        *running = false;
    }
    return seed;
}
#endif

void perpetualSearch(uint64_t soupsPerHaul, int numThreads, bool interactive, const std::string& payoshaKey, const std::string& seed,
                        int unicount, int local_log, std::atomic<bool> &running, bool testing) {
    /*
    Unifies several similar functions into one simpler one.
    */

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

    uint64_t soupsCompletedSinceStart = 0;
    uint64_t soupsCompletedBeforeLastMessage = 0;

    SoupSearcher globalSoup;
    globalSoup.tilesProcessed = 0;
    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);

    populateLuts();

    std::cout << "Running " << soupsPerHaul << " soups per haul:" << std::endl;

    #ifdef USING_GPU
    uint64_t epoch_size = 1000000;
    apg::GpuSearcher gs(0, unicount, SYMMETRY);
    #else
    uint64_t epoch_size = 100000;
    CpuSearcher gs; (void) unicount;
    #endif

    std::vector<uint64_t> vec;
    if (numThreads != 0) { gs.pump(seed, 0, vec); }

    auto timeOfStart = std::chrono::system_clock::now();
    auto timeOfLastMessage = timeOfStart;
    auto timeOfLastTimerCheck = timeOfStart;

    uint64_t maxcount = soupsPerHaul;

    uint64_t lb = 0;

    while (running) {

        if (numThreads == 0) {
#ifdef STDIN_SYM
           std::string suffix = retrieveSeed(&running);
#else
           std::string suffix = strConcat(soupsCompletedSinceStart);
#endif
            globalSoup.censusSoup(seed, suffix, cfier);
            soupsCompletedSinceStart += 1;
        } else {
            std::vector<SoupSearcher> localSoups(numThreads, SoupSearcher(&globalSoup));
            std::vector<std::thread> lsthreads(numThreads);

            std::atomic<uint64_t> idx(0);
            std::atomic<uint64_t> ts(0);

            auto nvec = narrow(vec, lb, maxcount);

            vec.clear();

            for (int j = 0; j < numThreads; j++) {
                lsthreads[j] = std::thread(partialBalancedSearch, &(nvec), seed, &(localSoups[j]), &running, &idx, &ts);
            }

            uint64_t newi = soupsCompletedSinceStart;
            lb = ((newi / epoch_size) + 1) * epoch_size;

            do {
                newi = ((newi / epoch_size) + 1) * epoch_size;
                if (newi < maxcount) {
                    gs.pump(seed, newi / epoch_size, vec);
                } else {
                    newi = maxcount;
                }
            } while ((newi < maxcount) && (vec.size() < 5000) && (running) && (ts < nvec.size()));

            for (int j = 0; j < numThreads; j++) {
                lsthreads[j].join();
                globalSoup.aggregate(&(localSoups[j].census), &(localSoups[j].alloccur));
            }

            if (running || (ts == nvec.size())) {
                soupsCompletedSinceStart = newi;
            } else {
                double diff = newi - soupsCompletedSinceStart;
                uint64_t estim = (diff * ts) / nvec.size();
                soupsCompletedSinceStart += estim;
            }
        }

        auto now = std::chrono::system_clock::now();
        double secondsSinceLastMessage = 0.001 * std::chrono::duration_cast<std::chrono::milliseconds>(now - timeOfLastMessage).count();
        double secondsSinceLastTimerCheck = 0.001 * std::chrono::duration_cast<std::chrono::milliseconds>(now - timeOfLastTimerCheck).count();
        double secondsSinceStart = 0.001 * std::chrono::duration_cast<std::chrono::milliseconds>(now - timeOfStart).count();

        timeOfLastTimerCheck = now;

        bool finished = (!running) || ((soupsCompletedSinceStart == maxcount) && vec.empty());
        uint64_t soupsCompletedSinceLastMessage = soupsCompletedSinceStart - soupsCompletedBeforeLastMessage;

        if (finished || (secondsSinceLastMessage >= 10.0) || ((secondsSinceLastTimerCheck >= 1.0) && (soupsCompletedSinceLastMessage == 1)) || (soupsCompletedSinceLastMessage >= 1000000)) {
            std::cout << RULESTRING << "/" << SYMMETRY << ": " << soupsCompletedSinceStart << " soups completed ("
                      << std::fixed << std::setprecision(3) << (soupsCompletedSinceLastMessage / secondsSinceLastMessage) << " soups/second current, "
                      << (soupsCompletedSinceStart / secondsSinceStart) << " overall)." << std::endl;

            soupsCompletedBeforeLastMessage = soupsCompletedSinceStart;
            timeOfLastMessage = now;

            #ifndef _WIN32
            if (interactive && keyWaiting()) {
                char c = fgetc(stdin);
                if ((c == 'q') || (c == 'Q')) { running = false; finished = true; }
            }
            #endif
        }

        if (finished) {
            std::cout << "----------------------------------------------------------------------" << std::endl;
            std::cout << soupsCompletedSinceStart << " soups completed." << std::endl;
            std::cout << "Attempting to contact Catagolue..." << std::endl;
            std::string payoshaResponse = globalSoup.submitResults(payoshaKey, seed, soupsCompletedSinceStart, local_log, testing);

            if (payoshaResponse.length() == 0) {
                std::cout << "Connection was unsuccessful." << std::endl;
            } else {
                std::cout << "Connection was successful." << std::endl;
                break;
            }

            if (running) {
                std::cout << "Continuing search..." << std::endl;
                vec.clear();
                if (numThreads != 0) { gs.pump(seed, soupsCompletedSinceStart / epoch_size, vec); }
                maxcount += soupsPerHaul;
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
