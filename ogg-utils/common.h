#ifndef COMMON_H
#define COMMON_H

//#include <algorithm>
//#include <cstdint>
#include <fstream>
//#include <vector>

#include <ogg/ogg.h>


class OggPage
{
private:
	ogg_page* page;

public:
	OggPage(ogg_page* p) : page(p) {}

	long header_size() const { return page->header_len; }
	long body_size() const { return page->body_len; }
	long size() const { return page->header_len + page->body_len; }

	bool bos() const { return ogg_page_bos(page); }
	bool eos() const { return ogg_page_eos(page); }
	bool continued() const { return ogg_page_continued(page); }

	int packets() const { return ogg_page_packets(page); }

	ogg_int64_t granulepos() const { return ogg_page_granulepos(page); }
	int serialno() const { return ogg_page_serialno(page); }
	int pageno() const { return ogg_page_pageno(page); }

	static std::ostream& write(std::ostream& os, const OggPage& page)
	{
		const ogg_page* p = page.page;
		return os.write(reinterpret_cast<const char*>(p->header), p->header_len).write(reinterpret_cast<const char*>(p->body), p->body_len);
	}
};


class OggSeeker
{
private:
	static const std::size_t BufferSize = 1 << 20; // 1MB

	std::ifstream from;

	ogg_sync_state sync_state;
	ogg_page page;

	OggPage current_page_;

	std::ifstream::pos_type base;
	std::ifstream::off_type delta;
	std::ifstream::pos_type page_pos_;

public:
	OggSeeker(const char* filename)
	 : from(filename, std::ios_base::binary), current_page_(&page), delta(0)
	{
		ogg_sync_init(&sync_state);
		ogg_sync_pageout(&sync_state, &page);
	}

	~OggSeeker() { ogg_sync_clear(&sync_state); }

	OggPage& current_page() { return current_page_; }
	std::ifstream::pos_type page_pos() const { return page_pos_; }

	bool next();
};

#endif
