CXX=g++
CXXFLAGS=-std=c++17 -g -static -Wall
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

9cc: $(OBJS)
		$(CXX) -o 9cc $(OBJS) $(CXXFLAGS)

$(OBJS): 9cc.hpp

test: 9cc
		./test.sh

clean:
		rm -f 9cc *.o *~ tmp*

.PHONY: test clean
