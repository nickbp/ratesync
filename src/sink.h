#ifndef RATESYNC_SINK_H
#define RATESYNC_SINK_H

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

#include <map>
#include "song.h"

namespace ratesync {
	class ISink {
	public:
		virtual bool Get(std::map<song_t,rating_t>& out_rating) = 0;
		virtual bool Set(const song_ratings_t& song) = 0;
		virtual bool Clear(const song_rating_t& song) = 0;
	};
}

#endif
