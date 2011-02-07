#ifndef MPDTAGGER_LINKER_H
#define MPDTAGGER_LINKER_H

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

#include "symlinkaccess.h"
#include "mediaaccess.h"

namespace mpdtagger {
	class LinkerError : public std::runtime_error {
	public:
	LinkerError(const std::string& what) :
		std::runtime_error(what) { }
	};

	class Linker {
	public:
	Linker(const std::string& in_dir, const std::string& out_dir)
		: in_dir(in_dir), out_dir(out_dir) { }

		bool calculate_changes();
		void print_changes() const;
		void apply_changes();

	private:
		const std::string in_dir, out_dir;

		std::list<symlink_rating_t> unrated_to_rating;
		std::list<symlink_rating_t> rating_to_unrated;
		std::list<symlink_ratings_t> rating_change;
	};
}

#endif
