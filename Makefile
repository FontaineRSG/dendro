# Compiler
CXX = clang++
CXXFLAGS = -std=c++23 -Wall -Wextra -g 

INCLUDES = -I/usr/local/include -I/usr/include -Isrc/core -Isrc/parser -Isrc/utils
LIBS =  -lstdc++fs -lstdc++
LDFLAGS = -L/usr/local/lib

# Source files
CORE_SRCS = $(wildcard src/core/*.cpp)
PARSER_SRCS = $(wildcard src/parser/*.cpp)
SRCS = main.cpp

# Output executable
TARGET = dendro


# Build rules
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) $(SRCS) $(CORE_SRCS) $(PARSER_SRCS) -o $(TARGET) $(LIBS)

# Clean build files
clean:
	rm -f $(TARGET) *.o