#ifndef OGG_PAGE_H
#define OGG_PAGE_H

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

// See RFC-3533
struct OggPage
{
	static const std::size_t StaticSize = 27;

	// header
	uint32_t capture_pattern;
	uint8_t version;
	uint8_t header_type;
	uint64_t absolute_granule_position;
	uint32_t stream_serial_number;
	uint32_t page_sequence_no;
	uint32_t page_checksum;
	uint8_t page_segments;
	std::vector<uint8_t> segment_table;
	// body
	std::vector<char> body;

	std::size_t header_size() const { return StaticSize + segment_table.size(); }
	std::size_t body_size() const { return std::accumulate(segment_table.begin(), segment_table.end(), uint16_t(0)); }
	std::size_t size() const { return header_size() + body_size(); }

	bool is_continuation() const { return header_type & 0x1; }
	bool is_bos() const { return header_type & 0x2; }
	bool is_eos() const { return header_type & 0x4; }

	std::istream& read(std::istream& from, bool read_body = true);
	std::ostream& write(std::ostream& to) const;

	static bool find_capture_pattern(std::istream& from);
};

#endif
