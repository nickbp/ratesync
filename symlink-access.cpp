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

#include "symlink-access.h"

#include <sstream>

bool ratesong::symlink::Access::symlinks(std::map<symlink_t, rating_t>& out_rating,
										 std::list<symlink_t>& out_dangling) const {
	return true;//TODO go through outdir's expected rating paths and see what symlinks are present.
	//return 'ratings' according to what rating subdir things are in
}

bool ratesong::symlink::Access::symlink_clear(const symlink_t& song) {
	return true;//TODO delete symlink
}

bool ratesong::symlink::Access::symlink_set(const symlink_t& song, rating_t rating) {
	return true;//TODO delete old symlink (if present), create new
}

bool ratesong::symlink::Access::symlink_add(const song_t& song, rating_t rating) {
	return true;//TODO create new (based off path of song)
}