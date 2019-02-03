CPP_COMPILER=g++
C_COMPILER=gcc
LINKER=g++

CPP_FLAGS=-c -Wall -Wextra -pedantic -O3 -flto -funsafe-loop-optimizations -Wunsafe-loop-optimizations -frename-registers -march=native --std=c++11
C_FLAGS=-c -Wall -Wextra -pedantic -O3 -flto -march=native -fomit-frame-pointer
LD_FLAGS=-flto -pthread

THREADS=4
CPP_SOURCES=main.cpp includes/md5.cpp includes/happyhttp.cpp

ifdef LIFECOIN
    C_SOURCES=dilithium/fips202.c dilithium/packing.c dilithium/polyvec.c dilithium/rounding.c dilithium/ntt.c dilithium/poly.c dilithium/reduce.c dilithium/sign.c coincludes/sha3/sha3.c
    CPP_FLAGS += -DLIFECOIN
    EXECUTABLE="lifecoin"
    PROFILE_PARAMS=search -n 100000 -t 1 -s l_kEwHfF3ArtPb -p $(THREADS) -i 1 -v 0
else
    EXECUTABLE="apgluxe"
    PROFILE_PARAMS=-n 100000 -t 1 -s l_kEwHfF3ArtPb -p $(THREADS) -i 1 -v 0
endif

ifdef PROFILE_APGLUXE
PROFILE_DEPENDENCIES=$(EXECUTABLE_PROFILE)
PROFILE_ARGS=-fprofile-use -fprofile-correction
endif

OBJECTS=$(CPP_SOURCES:.cpp=.o) $(C_SOURCES:.c=.o)
OBJECTS_PROFILE=$(OBJECTS:.o=.op)
EXECUTABLE_PROFILE=$(EXECUTABLE)-profile

.SUFFIXES: .c .cpp .o .op

# Compile:
all: $(CPP_SOURCES) $(C_SOURCES) $(PROFILE_DEPENDENCIES) $(EXECUTABLE)
	rm -f $(OBJECTS_PROFILE) $(EXECUTABLE_PROFILE) *.gcda */*.gcda
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
	rm -f *.o */*.o *.op */*.op *.gcda */*.gcda $(EXECUTABLE)
	echo Clean done

$(EXECUTABLE): $(OBJECTS)
	$(LINKER) $(LD_FLAGS) $(PROFILE_ARGS) $(OBJECTS) -o $@

.cpp.o:
	$(CPP_COMPILER) $(CPP_FLAGS) $(PROFILE_ARGS) $< -o $@

.c.o:
	$(C_COMPILER) $(C_FLAGS) $(PROFILE_ARGS) $< -o $@

# Making profiler executable to do further optimization:

$(EXECUTABLE_PROFILE): $(OBJECTS_PROFILE)
	$(LINKER) $(LD_FLAGS) -fprofile-generate $(OBJECTS_PROFILE) -o $@
	true        Generating optimization profile, this may take some time...
	./$@ $(PROFILE_PARAMS)
	true        done!

.cpp.op:
	$(CPP_COMPILER) $(CPP_FLAGS) -fprofile-generate $< -o $@

.c.op:
	$(C_COMPILER) $(C_FLAGS) -fprofile-generate $< -o $@
