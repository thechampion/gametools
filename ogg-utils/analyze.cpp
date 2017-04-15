#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

#include <boost/program_options.hpp>

#include "ogg_page.h"


struct Span
{
	std::streampos begin;
	std::streampos end;
};

struct Stream
{
	Span span;
	std::size_t size;
	std::vector<Span> pages;

	Stream() : span({-1, -1}), size(0) {}
};
// Assume that incoming file may be not a media container and composed of OGG streams with same serial numbers
typedef std::multimap<decltype(OggPage::stream_serial_number), Stream> Streams;

struct Parameters
{
	enum Action { CollectStats, FindHoles };

	Action action;
	int verbosity;
	std::string filename;
};

Parameters parse_args(int argc, char* argv[]);
void print_help(std::ostream& to);

void collect_stats(std::istream& from, int verbosity = 0);
void find_holes(std::istream& from);


int main(int argc, char* argv[])
{
	const Parameters params = parse_args(argc, argv);

	std::ifstream from(params.filename);
	switch(params.action)
	{
	case Parameters::CollectStats:
		collect_stats(from, params.verbosity);
		break;
	case Parameters::FindHoles:
		find_holes(from);
	}
}

//----------------------------------------------------------------------------
Parameters parse_args(int argc, char* argv[])
{
	using namespace boost::program_options;

	options_description desc;
	desc.add_options()
		("find-holes,F", bool_switch())
		("help,h", "print help and exit")
		("verbose,v", bool_switch(), "print more (and more) details") // TODO: -v, -vv, -vvv, etc
		("filename", value<std::string>())
		;
	positional_options_description pos;
	pos.add("filename", 1);

	variables_map vm;
	try
	{
		store(command_line_parser(argc, argv).options(desc).positional(pos).run(), vm);
		notify(vm);
	}
	catch(error& err)
	{
		std::clog << err.what() << std::endl;
		std::exit(1);
	}

	if(vm.count("help"))
	{
		print_help(std::cout);
		std::exit(0);
	}
	if(!vm.count("filename"))
	{
		std::clog << "File is missing, try analyze --help" << std::endl;
		std::exit(1);
	}

	return Parameters{vm["find-holes"].as<bool>() ? Parameters::FindHoles : Parameters::CollectStats,
		int(vm["verbose"].as<bool>()), vm["filename"].as<std::string>()};
}

//----------------------------------------------------------------------------
void print_help(std::ostream& to)
{
	to << "Usage: analyze [option] FILE\n"
		"\n"
		"Options:\n"
		"  -F, --find-holes  print addresses not containing OGG page\n"
		"                    rather than normal output\n"
		"  -v, --verbose     print more details\n"
		"  -h, --help        print help and exit\n";
}

//----------------------------------------------------------------------------
void collect_stats(std::istream& from, int verbosity)
{
	Streams streams;
	std::size_t busy = 0;
	for(OggPage page; OggPage::find_capture_pattern(from); )
	{
		const auto begin = from.tellg();
		page.read(from, false);

		const auto snn = page.stream_serial_number;
		Streams::iterator curr = streams.end();
		for(auto search = streams.equal_range(snn); search.first != search.second; ++search.first)
			if(search.first->second.span.end == -1)
			{
				curr = search.first;
				break;
			}
		if(curr == streams.end())
		{
			if(!page.is_bos())
				std::clog << "WARNING: stream " << page.stream_serial_number << " doesn't start with BOS" << std::endl;
			curr = streams.emplace(snn, Stream());
			curr->second.span.begin = begin;
		}

		curr->second.pages.emplace_back(Span{begin, begin + std::streamoff(page.size() - 1)});
		curr->second.size += page.size();
		busy += page.size();

		if(page.is_eos())
			curr->second.span.end = curr->second.pages.back().end;
	}

	from.clear();
	from.seekg(0, std::ios_base::end);
	const auto file_size = from.tellg();

	std::cout << streams.size() << " streams found\n";
	if(verbosity)
	{
		for(Streams::const_iterator i = streams.begin(); i != streams.end(); ++i)
		{
			const Stream& s = i->second;
			std::cout << "Stream #" << i->first
				<< ": span " << std::hex << s.span.begin << "-" << s.span.end << std::dec << ", "
				<< s.pages.size() << " pages, " << s.size << " bytes\n";
		}
	}
	std::cout << busy << " of " << file_size << " bytes (" << 100 * busy / file_size << "%) busy" << std::endl;
}

//----------------------------------------------------------------------------
void find_holes(std::istream& from)
{
	// TODO: pretty printing
	std::streampos begin = 0;
	std::cout << std::hex;
	for(OggPage page; OggPage::find_capture_pattern(from); )
	{
		const auto page_begin = from.tellg();
		page.read(from, false);
		if(begin != page_begin)
			std::cout << begin << " " << page_begin - std::streamoff(1) << "\n";
		begin = page_begin + std::streamoff(page.size());
	}
	from.clear();
	from.seekg(0, std::ios_base::end);
	const auto end = from.tellg();
	if(begin != end)
		std::cout << begin << " " << end << "\n";
}
