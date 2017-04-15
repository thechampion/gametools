#include "common.h"

bool OggSeeker::next()
{
	for(;;)
	{
		const auto res = ogg_sync_pageseek(&sync_state, &page);
		if(res > 0)
		{
			page_pos_ = base + delta + std::ifstream::off_type(sync_state.returned - res);
			return true;
		}
		if(res == 0)
		{
			delta = sync_state.returned - sync_state.fill;

			char* buffer = ogg_sync_buffer(&sync_state, BufferSize);
			if(!buffer)
				return false;

			base = from.tellg();
			from.read(buffer, BufferSize);
			const auto bytes_read = from.gcount();
			if(!bytes_read && !from)
				return false;
			ogg_sync_wrote(&sync_state, bytes_read);
		}
	}
}

