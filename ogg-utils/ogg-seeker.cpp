#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

#include <getopt.h>

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


struct Parameters
{
	int verbosity = 0;
	int first_arg;
};


void process_file(const char* filename, int verbosity);

Parameters parse_args(int argc, char* const argv[]);
void print_help();


int main(int argc, char* argv[])
{
	const Parameters params = parse_args(argc, argv);
	for(int i = params.first_arg; i < argc; ++i)
		process_file(argv[i], params.verbosity);
}

//----------------------------------------------------------------------------
void process_file(const char* filename, int verbosity)
{
	Streams streams;
	OggSeeker os(filename);
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
		for(auto j = i->second.cbegin(); j != i->second.cend(); ++j)
		{
			std::cout << "bitstream #" << i->first << ": " << j->size << " bytes in " << j->pages << " pages, spans: " << std::showbase << std::hex;
			for(auto k = j->spans.cbegin(); k != j->spans.cend(); ++k)
				std::cout << k->begin << "-" << k->end << " ";
			std::cout << std::dec << std::noshowbase << "\n";
		}
	}
}

//----------------------------------------------------------------------------
Parameters parse_args(int argc, char* const argv[])
{
	static const char short_options[] = "hv";
	static const struct option long_options[] = {
		{"help",    no_argument, 0, 'h'},
		{"verbose", no_argument, 0, 'v'},
		{0,         0,           0, 0  }
	};

	Parameters res;
	for(;;)
	{
		int c = getopt_long(argc, argv, short_options, long_options, nullptr);
		if(c == -1)
			break;
		switch(c)
		{
			case 'h':
				print_help();
				std::exit(0);
			case 'v':
				++res.verbosity;
				break;
			default:
				std::cout << "Try ogg-seeker --help.\n";
				std::exit(1);
		}
	}
	res.first_arg = optind;
	return res;
}

//----------------------------------------------------------------------------
void print_help()
{
	std::cout << "Usage: ogg-seeker [options] FILE...\n\n"
		"Options:\n"
		"  -h, --help     display this help and exit.\n"
		"  -v, --verbose  increase verbosity, multiple -v options produce more output." << std::endl;
}
