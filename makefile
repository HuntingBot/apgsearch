CC=g++

CFLAGS=-c -Wall -Wextra -pedantic -O3 -march=native --std=c++11

LDFLAGS=-pthread

SOURCES=main.cpp includes/md5.cpp includes/happyhttp.cpp

OBJECTS=$(SOURCES:.cpp=.o)
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
	rm -f $(OBJECTS)
	echo Clean done

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

