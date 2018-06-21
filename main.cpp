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

#define APG_VERSION "v4.44-" LIFELIB_VERSION

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
#include "coincludes/cryptography.h"
#endif

#include "includes/apgluxe.h"

#ifdef LIFECOIN

#else

int main(int argc, char* argv[]) {
    return run_apgluxe(argc, argv);
}

#endif
