#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <unistd.h>

#ifdef _POSIX_SOURCE
#include <signal.h>
#define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#endif

#include "lifelib/upattern.h"
#include "lifelib/classifier.h"
#include "lifelib/incubator.h"

#define APG_VERSION "v5.0-" LIFELIB_VERSION

#include "includes/params.h"

#ifdef USING_GPU
#include "lifelib/cuda/gs_def.h"
#endif

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

int main (int argc, char *argv[]) {

    return run_apgluxe(argc, argv);

}
