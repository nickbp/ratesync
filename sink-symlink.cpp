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

#include "sink-symlink.h"
#include "config.h"

#include <sstream>

#include <sys/stat.h>
#include <unistd.h>

namespace {
	bool check_symlink(const std::string& linkpath, bool show_err = true) {
		struct stat sb;
		if (stat(linkpath.c_str(), &sb) != 0) {
			ratesync::config::error("Unable to stat file %s.",
									linkpath.c_str());
			return false;
		}

		if (!S_ISLNK(sb.st_mode)) {
			ratesync::config::error("Not a symlink: %s",
									linkpath.c_str());
			return false;
		}

		return true;
	}
}

bool ratesync::sink::Symlink::Get(std::map<song_t,rating_t>& out_rating) {

	//TODO create dir if doesnt exist. wipe dangling symlinks if it does.
	//return false if dir creation failed or dir exists but is inaccessible

	return true;//TODO go through outdir's expected rating paths and see what symlinks are present.
	//return 'ratings' according to what rating subdir things are in
}

bool ratesync::sink::Symlink::Set(const song_ratings_t& song) {
	symlink_t oldpath = link_path(song.path, song.rating_old);
	if (check_symlink(oldpath, false) && unlink(oldpath.c_str()) != 0) {
		config::error("Unable to delete old symlink: %s", oldpath.c_str());
		return false;
	}
	return true;//TODO create new
}

bool ratesync::sink::Symlink::Clear(const song_rating_t& song) {
	symlink_t linkpath = link_path(song.path, song.rating);
	return check_symlink(linkpath) && (unlink(linkpath.c_str()) == 0);
}

ratesync::sink::Symlink::symlink_t
ratesync::sink::Symlink::link_path(const song_t& song, const rating_t rating) {
	//remove music_dir from song.path = abspath
	//concat link_dir+rating+abspath, return
	return std::string();//TODO
}
