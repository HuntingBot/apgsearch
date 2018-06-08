CPP_COMPILER=g++
C_COMPILER=gcc
LINKER=g++

# Thanks to Andrew Trevorrow for the following routine to handle clang:
ifeq "$(shell uname)" "Darwin"
    # we're on Mac OS X, so check if clang is available
    ifeq "$(shell which clang++)" "/usr/bin/clang++"
        # assume we're on Mac OS 10.9 or later
        MACOSX_109_OR_LATER=1
    endif
endif

C_FLAGS=-c -Wall -Wextra -O3 -march=native -fomit-frame-pointer

ifdef MACOSX_109_OR_LATER
    # g++ is really clang++ and there is currently no OpenMP support
    CPP_FLAGS=-c -Wall -O3 -march=native --std=c++11
else
    # assume we're using gcc with OpenMP support
    CPP_FLAGS=-c -Wall -O3 -march=native -fopenmp -DUSE_OPEN_MP --std=c++11
    LD_FLAGS=-fopenmp
endif

CPP_SOURCES=main.cpp includes/sha256.cpp includes/md5.cpp includes/happyhttp.cpp

ifdef LIFECOIN
    C_SOURCES=dilithium/fips202.c dilithium/packing.c dilithium/polyvec.c dilithium/rounding.c dilithium/ntt.c dilithium/poly.c dilithium/reduce.c dilithium/sign.c
    CPP_FLAGS += -DLIFECOIN
endif

ifdef FULLNODE
    CPP_FLAGS += -lmicrohttpd -DFULLNODE
endif

OBJECTS=$(CPP_SOURCES:.cpp=.o) $(C_SOURCES:.c=.o)
EXECUTABLE=apgluxe

# Compile:
all: $(SOURCES) $(EXECUTABLE)
	true
	true                                                oo o
	true                                                oo ooo
	true                                                      o
	true                                                oo ooo
	true                                                 o o
	true                                                 o o
	true                                                  o

# Clean the build environment by deleting any object files:
clean: 
	rm -f includes/*.o dilithium/*.o *.o
	echo Clean done

$(EXECUTABLE): $(OBJECTS) 
	$(LINKER) $(LD_FLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CPP_COMPILER) $(CPP_FLAGS) $< -o $@

.c.o:
	$(C_COMPILER) $(C_FLAGS) $< -o $@

