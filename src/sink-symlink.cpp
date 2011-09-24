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
#include <queue>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

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

	bool scan_rating_subdir(const std::string& subdir, ratesync::rating_t rating,
							std::map<ratesync::song_t, ratesync::rating_t>& out_rating) {
		std::queue<std::string> dirqueue;
		dirqueue.push(subdir);

		char symdest_c[1024];
		memset(&symdest_c, 0, sizeof(symdest_c));
		while (!dirqueue.empty()) {
			std::string dir = dirqueue.front();
			dirqueue.pop();

			DIR* dp = opendir(dir.c_str());
			if (dp == NULL) {
				ratesync::config::error("Couldn't open directory %s",
										dir.c_str());
				return false;
			}

			struct dirent* ep;
			while (ep = readdir(dp)) {
				//"This is the only field you can count on in all POSIX systems":
				ratesync::song_t filepath = dir+ep->d_name;
				ratesync::config::debug(filepath.c_str());

				struct stat sb;
				if (stat(filepath.c_str(), &sb) != 0) {
					ratesync::config::error("Unable to stat file %s.",
											filepath.c_str());
					closedir(dp);
					return false;
				}
				if (S_ISDIR(sb.st_mode)) {
					dirqueue.push(filepath+SEP);
				} else if (S_ISLNK(sb.st_mode)) {
					ssize_t len = readlink(filepath.c_str(), symdest_c, sizeof(symdest_c));
					if (len < 0) {
						ratesync::config::error("Unable to read symlink %s.",
												filepath.c_str());
						return false;
					}

					struct stat lsb;
					if (lstat(symdest_c, &lsb) != 0) {//TODO assuming != 0 when dangling
						if (unlink(filepath.c_str()) == 0) {
							continue;
						} else {
							ratesync::config::error("Unable to delete dangling symlink %s -> %s.",
													filepath.c_str(), symdest_c);
							return false;
						}
					}

					std::string symdest(symdest_c);
					std::map<ratesync::song_t, ratesync::rating_t>::iterator
						iter = out_rating.find(symdest);
					if (iter != out_rating.end()) {
						ratesync::config::error("Duplicate symlink to same file: %s -> %s",
												filepath.c_str(), symdest_c);
						continue;
					}
					out_rating.insert(std::make_pair(symdest, rating));
				}
			}
			closedir(dp);
		}
	}
}

bool ratesync::sink::Symlink::Get(std::map<song_t,rating_t>& out_rating) {
	struct stat sb;
	if (stat(symlink_dir.c_str(), &sb) != 0) {//TODO assuming != 0 when doesnt exist
		if (mkdir(symlink_dir.c_str(), 0777) == 0) {
			//just created dir, assume no symlinks
			return true;
		} else {
			//dont bother with recursive creation
			config::error("Unable to create symlink dir: %s", symlink_dir.c_str());
			return false;
		}
	}

	if (!scan_rating_subdir(symlink_dir+"unrated"+SEP, UNRATED, out_rating) ||
		!scan_rating_subdir(symlink_dir+"1"+SEP, 1, out_rating) ||
		!scan_rating_subdir(symlink_dir+"2"+SEP, 2, out_rating) ||
		!scan_rating_subdir(symlink_dir+"3"+SEP, 3, out_rating) ||
		!scan_rating_subdir(symlink_dir+"4"+SEP, 4, out_rating) ||
		!scan_rating_subdir(symlink_dir+"5"+SEP, 5, out_rating)) {
		return false;
	}

	return true;
}

bool ratesync::sink::Symlink::Set(const song_ratings_t& song) {
	symlink_t oldpath = link_path(song.path, song.rating_old);
	if (check_symlink(oldpath, false) && unlink(oldpath.c_str()) != 0) {
		config::error("Unable to delete old symlink: %s", oldpath.c_str());
		return false;
	}
	symlink_t newpath = link_path(song.path, song.rating_new);
	if (symlink(song.path.c_str(), newpath.c_str()) != 0) {
		config::error("Unable to create symlink: %s", newpath.c_str());
		return false;
	}
	return true;
}

bool ratesync::sink::Symlink::Clear(const song_rating_t& song) {
	symlink_t linkpath = link_path(song.path, song.rating);
	return check_symlink(linkpath) && (unlink(linkpath.c_str()) == 0);
}

ratesync::sink::Symlink::symlink_t
ratesync::sink::Symlink::link_path(const song_t& song, const rating_t rating) {
	std::ostringstream oss;
	if (rating == UNRATED) {
		oss << symlink_dir << "unrated" << SEP;
	} else {
		oss << symlink_dir << rating << SEP;
	}
	// /dir1/music_dir/dir2/file -> dir2/file
	oss << song.substr(music_dir.length());
	return oss.str();
}
