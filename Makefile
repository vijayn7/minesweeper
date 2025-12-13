# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
INCLUDES = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib
LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Target executable
TARGET = m

# Source files
SOURCES = main.cpp Board.cpp BoardRenderer.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS) $(LIBS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean compiled files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean
