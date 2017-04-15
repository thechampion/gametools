CXX = g++
CXXFLAGS = -Wall -std=c++11 -g -c
LD = g++
LDFLAGS = -Wall
RM = rm -f
ANALYZE_LIBS = -lboost_program_options
DEMUX_LIBS = -lboost_filesystem -lboost_program_options -lboost_system


all: analyze demux

analyze: analyze.o ogg_page.o
	$(LD) $(LDFLAGS) $(ANALYZE_LIBS) -o $@ $^

analyze.o: analyze.cpp ogg_page.h
	$(CXX) $(CXXFLAGS) -o $@ $<

demux: demux.o ogg_page.o
	$(CXX) $(LDFLAGS) $(DEMUX_LIBS) -o $@ $^

demux.o: demux.cpp ogg_page.h
	$(CXX) $(CXXFLAGS) -o $@ $<

ogg_page.o: ogg_page.cpp ogg_page.h
	$(CXX) $(CXXFLAGS) -o $@ $<


.PHONY: clean
clean:
	$(RM) analyze.o demux.o ogg_page.o
