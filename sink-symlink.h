#ifndef RATESYNC_SINK_SYMLINK_H
#define RATESYNC_SINK_SYMLINK_H

/*
  ratesync - Manages songs according their rating metadata.
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

#include "sink.h"

namespace ratesync {
	namespace sink {
		class Symlink : public ISink {
		public:
		Symlink(const std::string& music_dir, const std::string& symlink_dir)
			: music_dir(music_dir), symlink_dir(symlink_dir) { }
			virtual ~Symlink() { }

			bool Get(std::map<song_t,rating_t>& out_rating);
			bool Set(const song_ratings_t& song);
			bool Clear(const song_rating_t& song);

		private:
			typedef std::string symlink_t;
			symlink_t link_path(const song_t& song, const rating_t rating);

			const std::string music_dir, symlink_dir;
		};
	}
}

#endif
