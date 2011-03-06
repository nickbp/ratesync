#ifndef RATESONG_MPD_ACCESS_H
#define RATESONG_MPD_ACCESS_H

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

struct mpd_connection;

namespace ratesong {
	namespace mpd {
		class Access {
		public:
			Access(const std::string& host, size_t port);
			virtual ~Access();

			bool connect();

			bool ratings(std::list<song_t>& out_all,
						 std::map<song_t,rating_t>& out_rating) const;

			bool rating_clear(const song_t& song);
			bool rating_set(const song_t& song, rating_t rating);
		private:
			const std::string host;
			const size_t port;
			struct mpd_connection* conn;
		};
	}
}

#endif
