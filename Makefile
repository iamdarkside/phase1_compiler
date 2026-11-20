CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2
SOURCES = src/main.cpp src/lexer.cpp src/parser.cpp src/highlight.cpp

bminor: $(SOURCES) src/common.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o bminor

test: bminor
	bash tests/run.sh

clean:
	rm -f bminor

.PHONY: test clean
