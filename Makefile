CXX=clang++
CXXFLAGS +=\
	-std=c++11 -stdlib=libc++\
	-l readline\
	-Wall -Wpedantic -Werror

CXXFLAGS += -g

sesh: sesh.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<
