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

#include "lifelib/upattern.h"
#include "lifelib/classifier.h"
#include "lifelib/incubator.h"

#define APG_VERSION "v4.93-" LIFELIB_VERSION

#include "includes/params.h"
#include "includes/md5.h"
#include "includes/payosha256.h"
#include "includes/hashsoup2.h"

#define LIFETREE_MEM 100

#include "includes/detection.h"
#include "includes/stabilise.h"
#include "includes/searcher.h"
#include "includes/verification.h"
#include "includes/searching.h"

#include "includes/apgluxe.h"

#ifdef LIFECOIN

#include "coincludes/entrypoint.h"

#else

int main(int argc, char* argv[]) {
    return run_apgluxe(argc, argv);
}

#endif
