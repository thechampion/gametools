#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "common.h"


int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		std::cerr << "Usage: ogg-extractor FILE..." << std::endl;
		std::exit(1);
	}

	std::map<int, std::ofstream> streams;
	OggSeeker os(argv[1]);
	OggPage& page = os.current_page();
	while(os.next())
	{
		auto i = streams.find(page.serialno());
		if(i == streams.end())
		{
			std::ostringstream name;
			name << page.serialno() << ".ogg";
			i = streams.emplace(page.serialno(), std::ofstream(name.str(), std::ios_base::binary | std::ios_base::trunc)).first;
		}
		OggPage::write(i->second, page);
	}
}
