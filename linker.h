#ifndef RATESONG_LINKER_H
#define RATESONG_LINKER_H

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

#include "symlink-access.h"
#include "media-access.h"

namespace ratesong {
	class Linker {
	public:
	Linker(const std::string& in_dir, const std::string& out_dir)
		: in_dir(in_dir), out_dir(out_dir) { }

		bool calculate_changes();
		bool has_changes() const;
		void print_changes() const;
		bool apply_changes();

	private:
		const std::string in_dir, out_dir;

		std::list<symlink_rating_t> unrated_to_rating;
		std::list<symlink_rating_t> rating_to_unrated;
		std::list<symlink_ratings_t> rating_change;
	};
}

#endif
