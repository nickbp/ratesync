#ifndef MPDTAGGER_MEDIAACCESS_H
#define MPDTAGGER_MEDIAACCESS_H

/*
  mpdtagger - Synchronizes metadata between MPD stickers and media files.
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

#include "mpdaccess.h"

#include <stdexcept>
#include <string>
#include <map>
#include <set>

namespace mpdtagger {
	namespace media {
		class Error : public std::runtime_error {
		public:
		Error(const std::string& what) :
			std::runtime_error(what) { }
		};

		class File {
		public:
		File(const std::string& path)
			: path(path) { }

			bool rating(rating_t& out);
		private:
			const std::string path;
		};

		class Access {
		public:
			Access(const std::string& music_dir);

			void ratings(const std::list<mpd::song_t>& mpd_songs,
						 std::map<mpd::song_t,rating_t>& out_rating,
						 std::set<mpd::song_t>& out_unrated);
		private:
			const std::string music_dir;
		};
	}
}

#endif
