# Makefile for Job Matching System
# Compiler settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -g
TARGET = job_matcher
SOURCES = main.cpp ArrayJobMatcher.cpp LinkedListJobMatcher.cpp
HEADERS = ArrayJobMatcher.hpp LinkedListJobMatcher.hpp
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Compile source files to object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -DDEBUG -g3
debug: $(TARGET)

# Release build
release: CXXFLAGS += -DNDEBUG -O3
release: clean $(TARGET)

# Install dependencies (if needed)
install-deps:
	@echo "No external dependencies required for this project"

# Help target
help:
	@echo "Available targets:"
	@echo "  all      - Build the project (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  run      - Build and run the program"
	@echo "  debug    - Build with debug symbols"
	@echo "  release  - Build optimized release version"
	@echo "  help     - Show this help message"

# Phony targets
.PHONY: all clean run debug release install-deps help

