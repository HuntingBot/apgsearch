#include "includes/base.h"

#ifdef USING_GPU
#ifdef NEW_GPU_ALGO
#include "lifelib/cuda2/gs_def.h"
#else
#include "lifelib/cuda/gs_def.h"
#endif
#endif

#ifdef _POSIX_SOURCE
#include <signal.h>
#define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#endif

#include "includes/verification.h"
#include "includes/searching.h"
#include "includes/apgluxe.h"

int main (int argc, char *argv[]) {

    return run_apgluxe(argc, argv);

}
