#ifndef RATESONG_MEDIA_ACCESS_H
#define RATESONG_MEDIA_ACCESS_H

/*
  ratesong - Synchronizes metadata between MPD stickers and media files.
  Copyright (C) 2010  Nicholas Parker

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>
#include <list>
#include <map>
#include <set>

#include "song.h"

namespace ratesong {
	namespace media {
		class Access {
		public:
			Access(const std::string& music_dir)
				: music_dir(music_dir) { }

			bool ratings(std::map<song_t,rating_t>& out_rating,
						 std::set<song_t>& out_unrated);
			bool ratings(const std::list<song_t>& songs,
						 std::map<song_t,rating_t>& out_rating,
						 std::set<song_t>& out_unrated);
		private:
			const std::string music_dir;
		};
	}
}

#endif
