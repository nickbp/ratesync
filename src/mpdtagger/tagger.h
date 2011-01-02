#ifndef MPDTAGGER_TAGGER_H
#define MPDTAGGER_TAGGER_H

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

#include "mpdaccess.h"
#include "mediaaccess.h"

namespace mpdtagger {
	class TaggerError : public std::runtime_error {
	public:
	TaggerError(const std::string& what) :
		std::runtime_error(what) { }
	};

	class Tagger {
	public:
		Tagger(std::string host, std::string dir)
			: host(host), dir(dir) { }

		void run();
	private:
		std::string host, dir;
	};
}

#endif
