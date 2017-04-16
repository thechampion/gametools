#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include <boost/filesystem.hpp>
#include <getopt.h>

#include "common.h"

namespace bfs = boost::filesystem;


struct Parameters
{
	bfs::path dest_dir;
	bool overwrite = false;
	int first_arg;
};


Parameters parse_args(int argc, char* const argv[]);
void print_help();


int main(int argc, char* argv[])
{
	const Parameters params = parse_args(argc, argv);
	if(!params.dest_dir.empty() && !bfs::is_directory(params.dest_dir))
	{
		std::cout << "Directory " << params.dest_dir << " not found.\n";
		std::exit(2);
	}

	std::map<int, std::ofstream> streams;
	std::map<int, int> encountered;
	for(int i = params.first_arg; i < argc; ++i)
	{
		OggSeeker os(argv[i]);
		OggPage& page = os.current_page();
		while(os.next())
		{
			const auto sno = page.serialno();
			auto i = streams.find(sno);
			if(i == streams.end())
			{
				std::ostringstream name;
				name << sno;
				if(encountered[sno])
					name << "_" << encountered[sno];
				name << ".ogg";

				const std::string filepath = (params.dest_dir / name.str()).string();
				i = streams.emplace(sno, std::ofstream(filepath, std::ios_base::binary | std::ios_base::trunc)).first;
				++encountered[sno];
			}
			page.write(i->second);

			if(page.eos())
			{
				i->second.close();
				streams.erase(i);
			}
		}
	}
}

//----------------------------------------------------------------------------
Parameters parse_args(int argc, char* const argv[])
{
	static const char short_options[] = "d:hw";
	static const struct option long_options[] = {
		{"dest-dir",  required_argument, 0, 'd'},
		{"help",      no_argument,       0, 'h'},
		{"overwrite", no_argument,       0, 'w'},
		{0,           0,                 0, 0  }
	};

	Parameters res;
	for(;;)
	{
		int c = getopt_long(argc, argv, short_options, long_options, nullptr);
		if(c == -1)
			break;
		switch(c)
		{
			case 'd':
				res.dest_dir = optarg;
				break;
			case 'h':
				print_help();
				std::exit(0);
			case 'w':
				res.overwrite = true;
				break;
			default:
				std::cout << "Try ogg-extractor --help.\n";
				std::exit(1);
		}
	}
	res.first_arg = optind;
	return res;
}

//----------------------------------------------------------------------------
void print_help()
{
	std::cout << "Usage: ogg-extractor [options] FILE\n\n"
		"Options:\n"
		"  -d DIR, --dest-dir=DIR  extract files to specified directory.\n"
		"  -h, --help              show this help and exit.\n"
		"  -w, --overwrite         overwrite existing files." << std::endl;
}
