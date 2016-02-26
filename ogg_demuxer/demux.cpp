#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "ogg_page.h"

namespace fs = boost::filesystem;


typedef std::map<decltype(OggPage::stream_serial_number), std::ofstream> Writers;
//typedef std::map<decltype(OggPage::stream_serial_number), uint32_t> Counters;

struct Parameters
{
	fs::path filename;
	fs::path output_dir;
};


Parameters parse_args(int argc, char* argv[]);
void print_help(std::ostream& to);

void create_output_directory(const fs::path& dir);


int main(int argc, char* argv[])
{
	Parameters params = parse_args(argc, argv);

	create_output_directory(params.output_dir);

	std::ifstream reader(params.filename.native(), std::ios_base::binary);
	Writers writers;
	for(OggPage page; OggPage::find_capture_pattern(reader); )
	{
		page.read(reader);
		Writers::iterator i = writers.find(page.stream_serial_number);
		if(i == writers.end())
		{
			std::ostringstream fname;
			fname << "s" << page.stream_serial_number << ".ogg";
			i = writers.emplace(
					page.stream_serial_number,
					std::ofstream((params.output_dir / fname.str()).native(), std::ios_base::binary)
				).first;
		}
		page.write(i->second);
		if(page.is_eos())
			i->second.close();
	}

	for(Writers::iterator i = writers.begin(); i != writers.end(); ++i)
		if(i->second.is_open())
		{
			std::clog << "WARNING: stream " << i->first << " isn't terminated with EOS" << std::endl;
			i->second.close();
		}
}

//----------------------------------------------------------------------------
Parameters parse_args(int argc, char* argv[])
{
	using namespace boost::program_options;

	options_description desc;
	desc.add_options()
		("help,h", "print help and exit")
		//("template,t", value<std::string>()->default_value())
		("verbose,v", bool_switch(), "print more (and more) details") // TODO: -v, -vv, -vvv, etc
		("filename", value<std::string>())
		("output-dir", value<std::string>())
		;
	positional_options_description pos;
	pos.add("filename", 1);
	pos.add("output-dir", 1);

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
		print_help(std::clog);
		std::exit(0);
	}
	if(!vm.count("filename") || !vm.count("output-dir"))
	{
		std::clog << "Missing arguments, try demux --help" << std::endl;
		std::exit(1);
	}

	return Parameters{vm["filename"].as<std::string>(), vm["output-dir"].as<std::string>()};
}

//----------------------------------------------------------------------------
void print_help(std::ostream& to)
{
	to << "Usage: demux [option] FILE OUTDIR\n"
		"\n"
		"Options:\n"
		//"  -t, --template    print addresses not containing OGG page\n"
		//"                    rather than normal output\n"
		"  -v, --verbose     print more details\n"
		"  -h, --help        print help and exit\n";
}

//----------------------------------------------------------------------------
void create_output_directory(const fs::path& dir)
{
	if(!fs::exists(dir))
	{
		fs::create_directory(dir);
		return;
	}
	if(!is_directory(dir))
	{
		std::clog << dir << " exists but is not a directory" << std::endl;
		std::exit(2);
	}
}
