CPP_COMPILER=g++
C_COMPILER=gcc
LINKER=g++

CPP_FLAGS=-c -Wall -Wextra -pedantic -O3 -Ofast -flto -funsafe-loop-optimizations -Wunsafe-loop-optimizations -frename-registers -march=native --std=c++11
C_FLAGS=-c -Wall -Wextra -pedantic -O3 -march=native -fomit-frame-pointer
LD_FLAGS=-flto -pthread

CPP_SOURCES=main.cpp includes/md5.cpp includes/happyhttp.cpp

ifdef PROFILE_APGLUXE
PROFILE_DEPENDENCIES=$(EXECUTABLE_PROFILE)
PROFILE_ARGS=-fprofile-use -fprofile-correction
endif

OBJECTS=$(CPP_SOURCES:.cpp=.o) $(C_SOURCES:.c=.o)
OBJECTS_PROFILE=$(OBJECTS:.o=.op)
EXECUTABLE=apgluxe
EXECUTABLE_PROFILE=$(EXECUTABLE)-profile
THREADS=4

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
	rm -f $(OBJECTS) $(EXECUTABLE)
	echo Clean done

$(EXECUTABLE): $(OBJECTS)
	$(LINKER) $(LD_FLAGS) $(PROFILE_ARGS) $(OBJECTS) -o $@

.cpp.o:
	$(CPP_COMPILER) $(CPP_FLAGS) $(PROFILE_ARGS) $< -o $@

# Making profiler executable to do further optimization:

$(EXECUTABLE_PROFILE): $(OBJECTS_PROFILE)
	$(LINKER) $(LD_FLAGS) -fprofile-generate $(OBJECTS_PROFILE) -o $@
	true        Generating optimization profile, this may take some time...
	./$@ -n 100000 -k "#anon" -p $(THREADS) -i 1
	true        done!

.cpp.op:
	$(CPP_COMPILER) $(CPP_FLAGS) -fprofile-generate $< -o $@
