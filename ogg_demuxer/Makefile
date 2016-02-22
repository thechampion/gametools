CXX = g++
CXXFLAGS = -Wall -std=c++11 -g
LD = g++
LDFLAGS = -Wall
RM = rm -f
ANALYZE_LIBS = -lboost_program_options
OGG_DEMUXER_LIBS =


all: analyze ogg_demuxer

analyze: analyze.o ogg_page.o
	$(LD) $(LDFLAGS) $(ANALYZE_LIBS) -o $@ $^

analyze.o: analyze.cpp ogg_page.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

ogg_demuxer: ogg_demuxer.o ogg_page.o
	$(CXX) $(CXXFLAGS) $(OGG_DEMUXER_LIBS) -o $@ $^

ogg_demuxer.o: ogg_demuxer.cpp ogg_page.h
	$(CXX) $(CXXFLAGS) -o $@ $^

ogg_page.o: ogg_page.cpp ogg_page.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<


.PHONY: clean
clean:
	$(RM) analyze.o ogg_demuxer.o ogg_page.o
