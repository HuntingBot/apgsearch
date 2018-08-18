#pragma once

#include <iomanip>
#include <stdio.h>
#include <sys/select.h>

#include <atomic>
#include <thread>

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

void populateLuts() {

        apg::bitworld bw = apg::hashsoup("", SYMMETRY);
        std::vector<apg::bitworld> vbw;
        vbw.push_back(bw);
        UPATTERN pat;
        pat.insertPattern(vbw);
        pat.advance(0, 0, 8);

}

void partialSearch(uint64_t n, int m, int threadNumber, std::string seedroot,
    SoupSearcher *localSoup, uint64_t *j, std::atomic<bool> *running) {

    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);

    while (*running) {

        uint64_t i = threadNumber + m * ((*j));

        if ((n > 0) && (i >= n)) { break; }

        std::ostringstream ss;
        ss << i;
        /* dsentry dp = */ localSoup->censusSoup(seedroot, ss.str(), cfier);

        if (i % 100000 == 0) { std::cout << i << " soups processed..." << std::endl; }

        (*j)++;

        /*
        if (dp.first >= difficulty) {
            bestSoup = i; // this transcends the difficulty target
            running = false; // we can now exit
            std::cerr << "Block won: " << seedroot << i << " contains a " << dp.second << std::endl;
            std::cerr << "Target: " << difficulty << "; attained: " << dp.first << std::endl;
        }
        */

    }

}

void threadSearch(uint64_t n, int m, std::string payoshaKey, std::string seed,
                    int local_log, std::atomic<bool> &running) {

    SoupSearcher globalSoup;

    populateLuts();

    uint64_t completed[m];
    for (int i = 0; i < m; i++) { completed[i] = 0; }

    uint64_t maxcount = 0;

    while (running) {

        maxcount += n;

        std::vector<SoupSearcher> localSoups(m);
        std::vector<std::thread> lsthreads(m);

        for (int i = 0; i < m; i++) {
            lsthreads[i] = std::thread(partialSearch, maxcount, m, i, seed, &(localSoups[i]), completed + i, &running);
        }

        for (int i = 0; i < m; i++) {
            lsthreads[i].join();
            globalSoup.aggregate(&(localSoups[i].census), &(localSoups[i].alloccur));
        }

        uint64_t totalSoups = 0;

        for (int i = 0; i < m; i++) { totalSoups += completed[i]; }

        std::cout << "----------------------------------------------------------------------" << std::endl;
        std::cout << totalSoups << " soups completed." << std::endl;
        std::cout << "Attempting to contact payosha256." << std::endl;
        std::string payoshaResponse = globalSoup.submitResults(payoshaKey, seed, totalSoups, local_log, 0);

        if (payoshaResponse.length() == 0) {
            std::cout << "Connection was unsuccessful." << std::endl;
        } else {
            std::cout << "Connection was successful." << std::endl;
            running = false;
        }

        if (running) { std::cout << "Continuing search..." << std::endl; }
    }

}

bool parallelSearch(uint64_t n, int m, std::string payoshaKey, std::string seed, int local_log) {

    std::atomic<bool> running(true);

    threadSearch(n, m, payoshaKey, seed, local_log, running);

    bool was_running = running;

    return was_running;

}

bool runSearch(int n, std::string payoshaKey, std::string seed, int local_log, bool testing) {

    struct termios ttystate;

    // turn on non-blocking reads
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag &= ~ICANON;
    ttystate.c_cc[VMIN] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

    SoupSearcher soup;
    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);

    clock_t start = clock();
    clock_t overall_start = start;
    clock_t current = start;
    clock_t last_current = start;

    std::cout << "Running " << n << " soups per haul:" << std::endl;

    int64_t i = 0;
    int64_t lasti = 0;

    bool finishedSearch = false;
    bool quitByUser = false;

    while ((finishedSearch == false) && (quitByUser == false)) {

        std::ostringstream ss;
        ss << i;

        soup.censusSoup(seed, ss.str(), cfier);

        i += 1;

        last_current = current;
        current = clock();
        double elapsed = ((double) (current - start)) / CLOCKS_PER_SEC;
        double current_elapsed = ((double) (current - last_current)) / CLOCKS_PER_SEC;
        double overall_elapsed = ((double) (current - overall_start)) / CLOCKS_PER_SEC;

        if ((elapsed >= 10.0) || ((current_elapsed >= 1.0) && (i == (lasti + 1)))) {
            std::cout << RULESTRING << "/" << SYMMETRY << ": " << i << " soups completed (" << std::fixed << std::setprecision(3) << ((i - lasti) / elapsed) << " soups/second current, " << (i / overall_elapsed) << " overall)." << std::endl;
            lasti = i;
            start = clock();

            if(keyWaiting()) {
                char c = fgetc(stdin);
                if ((c == 'q') || (c == 'Q'))
                    quitByUser = true;
            }
            
        }

        if ((i % n == 0) || quitByUser) {
            last_current = current;
            current = clock();
            double elapsed = ((double) (current - start)) / CLOCKS_PER_SEC;
            double overall_elapsed = ((double) (current - overall_start)) / CLOCKS_PER_SEC;

            std::cout << RULESTRING << "/" << SYMMETRY << ": " << i << " soups completed (" << std::fixed << std::setprecision(3) << ((i - lasti) / elapsed) << " soups/second current, " << (i / overall_elapsed) << " overall)." << std::endl;
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
    
    // turn on blocking reads
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag |= ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

    return quitByUser;

}


