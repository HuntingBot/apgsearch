CPP_COMPILER=g++
C_COMPILER=gcc
LINKER=g++

C_FLAGS=-c -Wall -Wextra -pedantic -O3 -march=native -fomit-frame-pointer

LD_FLAGS=-pthread

CPP_FLAGS=-c -Wall -Wextra -pedantic -O3 -march=native --std=c++11

CPP_SOURCES=main.cpp includes/sha256.cpp includes/md5.cpp includes/happyhttp.cpp

ifdef LIFECOIN
    C_SOURCES=dilithium/fips202.c dilithium/packing.c dilithium/polyvec.c dilithium/rounding.c dilithium/ntt.c dilithium/poly.c dilithium/reduce.c dilithium/sign.c coincludes/sha3/sha3.c
    CPP_FLAGS += -DLIFECOIN
endif

ifdef FULLNODE
    CPP_FLAGS += -lmicrohttpd -DFULLNODE
    EXECUTABLE="lifecoin-server"
else
ifdef LIFECOIN
    EXECUTABLE="lifecoin"
else
    EXECUTABLE="apgluxe"
endif
endif

OBJECTS=$(CPP_SOURCES:.cpp=.o) $(C_SOURCES:.c=.o)

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
	rm -f includes/*.o dilithium/*.o coincludes/sha3/*.o *.o
	echo Clean done

$(EXECUTABLE): $(OBJECTS) 
	$(LINKER) $(LD_FLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CPP_COMPILER) $(CPP_FLAGS) $< -o $@

.c.o:
	$(C_COMPILER) $(C_FLAGS) $< -o $@

