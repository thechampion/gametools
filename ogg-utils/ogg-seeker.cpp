#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

#include <unistd.h>

#include "common.h"


struct Span
{
	std::ifstream::pos_type begin;
	std::ifstream::pos_type end;
};
typedef std::vector<Span> Spans;

struct Stream
{
	std::size_t pages;
	std::size_t size;
	Spans spans;
	bool ended = false;
};
typedef std::map<int, std::vector<Stream>> Streams;


int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		std::cerr << "Usage: ogg-seeker FILE..." << std::endl;
		std::exit(1);
	}

	//getopt_long

	Streams streams;
	OggSeeker os(argv[1]);
	OggPage& page = os.current_page();
	while(os.next())
	{
		//if(page.bos())
		//std::cout << "Span{" << std::showbase << std::hex << os.page_pos() << ":"
		//	<< os.page_pos() + std::ifstream::pos_type(page.size() - 1) << "}, size = " << std::dec << page.size() << "\n";

		Streams::iterator i = streams.find(page.serialno());
		if(i == streams.end())
			i = streams.emplace(page.serialno(), std::vector<Stream>()).first;
		if(i->second.empty() || i->second.back().ended)
			i->second.emplace_back(Stream());

		Stream& s = i->second.back();
		++s.pages;
		s.size += page.size();

		const Span span{os.page_pos(), os.page_pos() + std::ifstream::off_type(page.size() - 1)};
		if(!s.spans.empty() && (s.spans.back().end + std::ifstream::off_type(1)) == span.begin)
			s.spans.back().end = span.end;
		else
			s.spans.push_back(span);

		if(page.eos())
			s.ended = true;

	}
	for(auto i = streams.cbegin(); i != streams.cend(); ++i)
	{
		std::cout << "bitstream #" << i->first << "\n";
		for(auto j = i->second.cbegin(); j != i->second.cend(); ++j)
		{
			std::cout << "  " << j->size << " bytes in " << j->pages << " pages, spans: " << std::showbase << std::hex;
			for(auto k = j->spans.cbegin(); k != j->spans.cend(); ++k)
				std::cout << k->begin << "-" << k->end << " ";
			std::cout << std::dec << std::noshowbase << "\n";
		}
	}
}
