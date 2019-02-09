
COMPILER_FLAGS=-c -Wall -Wextra -pedantic -O3 -march=native
LD_FLAGS=-pthread

ifdef USE_MINGW
CPP_COMPILER=x86_64-w64-mingw32-g++
C_COMPILER=x86_64-w64-mingw32-gcc
EXTRA_LIBS=-static -lwinpthread -lwsock32 -lws2_32 -static-libstdc++
else
COMPILER_FLAGS += -flto
LD_FLAGS += -flto
CPP_COMPILER=g++
C_COMPILER=gcc
endif

LINKER=$(CPP_COMPILER)

CPP_FLAGS=$(COMPILER_FLAGS) --std=c++11
C_FLAGS=$(COMPILER_FLAGS) -fomit-frame-pointer

CPP_SOURCES=main.cpp includes/md5.cpp includes/happyhttp.cpp

OBJECTS=$(CPP_SOURCES:.cpp=.o) $(C_SOURCES:.c=.o)
OBJECTS_PROFILE=$(OBJECTS:.o=.op)
EXECUTABLE=apgluxe
EXECUTABLE_PROFILE=$(EXECUTABLE)-profile
THREADS=4

ifdef PROFILE_APGLUXE
CPP_FLAGS += -funsafe-loop-optimizations -Wunsafe-loop-optimizations
PROFILE_DEPENDENCIES=$(EXECUTABLE_PROFILE)
PROFILE_ARGS=-fprofile-use -fprofile-correction
endif

.SUFFIXES: .cpp .o .op

# Compile:
all: $(CPP_SOURCES) $(PROFILE_DEPENDENCIES) $(EXECUTABLE)
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
	rm -f *.o */*.o *.op */*.op *.gdca */*.gcda $(EXECUTABLE)
	echo Clean done

$(EXECUTABLE): $(OBJECTS)
	$(LINKER) $(LD_FLAGS) $(PROFILE_ARGS) $(OBJECTS) $(EXTRA_LIBS) -o $@

.cpp.o:
	$(CPP_COMPILER) $(CPP_FLAGS) $(PROFILE_ARGS) $< -o $@

# Making profiler executable to do further optimization:

$(EXECUTABLE_PROFILE): $(OBJECTS_PROFILE)
	$(LINKER) $(LD_FLAGS) -fprofile-generate $(OBJECTS_PROFILE) -o $@
	true        Generating optimization profile, this may take some time...
	./$@ -n 100000 -t 1 -s l_kEwHfF3ArtPb -p $(THREADS) -i 1 -v 0
	true        done!

.cpp.op:
	$(CPP_COMPILER) $(CPP_FLAGS) -fprofile-generate $< -o $@
