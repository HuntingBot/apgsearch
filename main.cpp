#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <unistd.h>
#include <termios.h>

#ifdef USE_OPEN_MP
#include <omp.h>
#endif

#include "lifelib/upattern.h"
#include "lifelib/classifier.h"
#include "lifelib/incubator.h"

#define APG_VERSION "v4.43-" LIFELIB_VERSION

#include "includes/params.h"
#include "includes/sha256.h"
#include "includes/md5.h"
#include "includes/payosha256.h"
#include "includes/hashsoup2.h"

#define LIFETREE_MEM 100

#include "includes/detection.h"
#include "includes/stabilise.h"
#include "includes/searcher.h"
#include "includes/verification.h"
#include "includes/searching.h"

#ifdef LIFECOIN
#include "includes/lifecoin.h"
#endif

#include "includes/apgluxe.h"

#ifdef LIFECOIN

int main(int argc, char* argv[]) {

    std::string modus_operandi = "";
    if (argc >= 2) {
        modus_operandi = argv[1];
    }

    if (modus_operandi == "mine") {
        // Remove first argument:
        return run_apgluxe(argc - 1, argv + 1);
    } else if (modus_operandi == "addrgen") {
        std::cerr << "Enter passwords line-by-line, with Ctrl+D to exit" << std::endl;
        apg::addrgen();
    } else {
        std::cerr << "Usage: ./lifecoin MODUS_OPERANDI [OPTIONS]" << std::endl;
        std::cerr << "    where MODUS_OPERANDI is one of the following:\n" << std::endl;

        std::cerr << "    mine : run the apgluxe soup searcher" << std::endl;
        std::cerr << "    addrgen : convert password(s) in stdin to address(es) in stdout" << std::endl;
        return 1;
    }

    return 0;
}

#else

int main(int argc, char* argv[]) {
    return run_apgluxe(argc, argv);
}

#endif
