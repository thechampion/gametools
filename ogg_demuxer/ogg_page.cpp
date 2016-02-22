#include "ogg_page.h"


template<typename T>
std::istream& raw_read(std::istream& from, T& value)
{
	return from.read(reinterpret_cast<char*>(&value), sizeof(value));
}

std::istream& OggPage::read(std::istream& from, bool read_body)
{
	raw_read(from, capture_pattern);
	raw_read(from, version);
	raw_read(from, header_type);
	raw_read(from, absolute_granule_position);
	raw_read(from, stream_serial_number);
	raw_read(from, page_sequence_no);
	raw_read(from, page_checksum);
	raw_read(from, page_segments);

	segment_table.resize(page_segments);
	from.read(reinterpret_cast<char*>(segment_table.data()), page_segments);

	const std::size_t bsz = body_size();
	if(read_body)
	{
		body.resize(bsz);
		from.read(reinterpret_cast<char*>(body.data()), bsz);
	}
	else
	{
		body.clear();
		from.ignore(bsz);
	}

	return from;
}

template<typename T>
std::ostream& raw_write(std::ostream& to, const T& value)
{
	return to.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

std::ostream& OggPage::write(std::ostream& to) const
{
	raw_write(to, capture_pattern);
	raw_write(to, version);
	raw_write(to, header_type);
	raw_write(to, absolute_granule_position);
	raw_write(to, stream_serial_number);
	raw_write(to, page_sequence_no);
	raw_write(to, page_checksum);
	raw_write(to, page_segments);
	to.write(reinterpret_cast<const char*>(segment_table.data()), segment_table.size());
	to.write(reinterpret_cast<const char*>(body.data()), body.size());
	return to;
}

bool OggPage::find_capture_pattern(std::istream& from)
{
	char ch;
	while(from)
	{
		while(from.get(ch) && (ch != 'O'));
		if(from.get(ch) && (ch != 'g'))
		{
			from.putback(ch);
			continue;
		}
		if(from.get(ch) && (ch != 'g'))
		{
			from.putback(ch);
			continue;
		}
		if(from.get(ch) && (ch != 'S'))
		{
			from.putback(ch);
			continue;
		}
		from.putback('S');
		from.putback('g');
		from.putback('g');
		from.putback('O');
		return bool(from);
	}
	return false;
}
