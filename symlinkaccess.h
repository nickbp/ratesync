#ifndef MPDTAGGER_MPDACCESS_H
#define MPDTAGGER_MPDACCESS_H

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

#include <stdexcept>
#include <string>
#include <list>
#include <map>
#include <set>

#include "song.h"

namespace mpdtagger {
	namespace symlink {
		class Error : public std::runtime_error {
		public:
		Error(const std::string& what) :
			std::runtime_error(what) { }
		};

		class Access {
		public:
			Access(const std::string& symlink_dir)
				: symlink_dir(symlink_dir) { }

			void symlinks(std::map<symlink_t, rating_t>& out_rating,
						  std::list<symlink_t>& out_dangling) const;

			void symlink_clear(const symlink_t& song);
			void symlink_set(const symlink_t& song, rating_t rating);
			void symlink_add(const song_t& song, rating_t rating);
		private:
			const std::string symlink_dir;
		};
	}
}

#endif