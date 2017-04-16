CXX = g++
CXXFLAGS = -Wall -std=c++14 -g -c
LD = g++
LDFLAGS = -Wall -std=c++14 -g
RM = rm -f
LIBS = $(shell pkg-config --libs ogg)


all: ogg-seeker ogg-extractor

ogg-seeker: ogg-seeker.o common.o
	$(LD) $(LDFLAGS) $(LIBS) -o $@ $^

ogg-seeker.o: ogg-seeker.cpp common.h
	$(CXX) $(CXXFLAGS) -o $@ $<

ogg-extractor: ogg-extractor.o common.o
	$(LD) $(LDFLAGS) $(LIBS) -lboost_filesystem -lboost_system -o $@ $^

ogg-extractor.o: ogg-extractor.cpp common.h
	$(CXX) $(CXXFLAGS) -o $@ $<

common.o: common.cpp common.h
	$(CXX) $(CXXFLAGS) -o $@ $<


.PHONY: clean
clean:
	$(RM) ogg-seeker.o ogg-extractor.o common.o
