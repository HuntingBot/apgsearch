
COMPILER_FLAGS=-c -Wall -Wextra -pedantic -O3 -pthread
LD_FLAGS=-pthread

ifdef USE_MINGW
ifeq "$(shell uname -s | grep -o CYGWIN)" "CYGWIN"
CPP_COMPILER=x86_64-w64-mingw32-g++
C_COMPILER=x86_64-w64-mingw32-gcc
else
CPP_COMPILER=x86_64-w64-mingw32-g++-posix
C_COMPILER=x86_64-w64-mingw32-gcc-posix
endif
EXTRA_LIBS=-static -lwinpthread -lwsock32 -lws2_32 -static-libstdc++
else
COMPILER_FLAGS += -flto -march=native
LD_FLAGS += -flto
CPP_COMPILER=g++
C_COMPILER=gcc
endif

LINKER=$(CPP_COMPILER)

CPP_FLAGS=$(COMPILER_FLAGS) --std=c++11
C_FLAGS=$(COMPILER_FLAGS) -fomit-frame-pointer

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

OBJECTS=$(CPP_SOURCES:.cpp=.o) $(C_SOURCES:.c=.o)
OBJECTS_PROFILE=$(OBJECTS:.o=.op)
EXECUTABLE_PROFILE=$(EXECUTABLE)-profile

.SUFFIXES: .c .cpp .o .op

PROF_MERGER=true

ifdef PROFILE_APGLUXE
PROFILE_DEPENDENCIES=$(EXECUTABLE_PROFILE)
PROFILE_ARGS=-fprofile-use -fprofile-correction
ifeq "$(shell $(CPP_COMPILER) --version | grep -o clang)" "clang"
PROF_MERGER=$(shell bash -c 'compgen -c' | grep 'llvm-profdata' | sort -V | tail -n 1)
else
CPP_FLAGS += -funsafe-loop-optimizations -Wunsafe-loop-optimizations
endif
ifeq "$(PROF_MERGER)" ""
PROF_MERGER=xcrun llvm-profdata
endif
endif

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
	rm -f *.o */*.o *.op */*.op *.gdca */*.gcda *.profraw *.profdata $(EXECUTABLE)
	echo Clean done

$(EXECUTABLE): $(OBJECTS)
	$(LINKER) $(LD_FLAGS) $(PROFILE_ARGS) $(OBJECTS) $(EXTRA_LIBS) -o $@

.cpp.o:
	$(CPP_COMPILER) $(CPP_FLAGS) $(PROFILE_ARGS) $< -o $@

.c.o:
	$(C_COMPILER) $(C_FLAGS) $(PROFILE_ARGS) $< -o $@

# Making profiler executable to do further optimization:

$(EXECUTABLE_PROFILE): $(OBJECTS_PROFILE)
	true        Using merger $(PROF_MERGER)
	$(LINKER) $(LD_FLAGS) -fprofile-generate $(OBJECTS_PROFILE) -o $@
	true        Generating optimization profile, this may take some time...
	./$@ $(PROFILE_PARAMS)
	$(PROF_MERGER) merge -o default.profdata *.profraw
	true        done!

.cpp.op:
	$(CPP_COMPILER) $(CPP_FLAGS) -fprofile-generate $< -o $@

.c.op:
	$(C_COMPILER) $(C_FLAGS) -fprofile-generate $< -o $@
