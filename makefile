CC=g++

CFLAGS=-c -Wall -Wextra -pedantic -O3 -Ofast -flto -funsafe-loop-optimizations -Wunsafe-loop-optimizations -frename-registers -march=native --std=c++11

LDFLAGS=-flto -pthread

SOURCES=main.cpp includes/md5.cpp includes/happyhttp.cpp

OBJECTS=$(SOURCES:.cpp=.o)
OBJECTS_PROFILE=$(SOURCES:.cpp=.O)
EXECUTABLE=apgluxe
EXECUTABLE_PROFILE=$(EXECUTABLE)O
THREADS=4

.SUFFIXES: .cpp .o .O

# Compile:
all: $(SOURCES) $(EXECUTABLE_PROFILE) $(EXECUTABLE)
	rm $(OBJECTS_PROFILE) $(EXECUTABLE_PROFILE) *.gcda */*.gcda
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
	$(CC) $(LDFLAGS) -fprofile-use -fprofile-correction $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -fprofile-use -fprofile-correction $< -o $@

# Making profiler executable to do further optimization:

$(EXECUTABLE_PROFILE): $(OBJECTS_PROFILE)
	$(CC) $(LDFLAGS) -fprofile-generate $(OBJECTS_PROFILE) -o $@
	true        Generating optimization profile, this may take some time...
	./$@ -n 100000 -k "#anon" -p $(THREADS) -i 1
	true        done!

.cpp.O:
	$(CC) $(CFLAGS) -fprofile-generate $< -o $@
