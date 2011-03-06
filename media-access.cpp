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

#include "media-access.h"
#include "config.h"

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

#include <taglib/mpegfile.h>
#include <taglib/flacfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/speexfile.h>
#include <taglib/vorbisfile.h>

#include <taglib/id3v2tag.h>
#include <taglib/popularimeterframe.h>
#include <taglib/xiphcomment.h>

#include <taglib/tmap.h>
#include <taglib/tlist.h>

#include <sstream>
#include <queue>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace {
	bool string_ends_with_ci(const std::string& haystack, const std::string& needle) {
		const size_t hs = haystack.size(), ns = needle.size();
		if (hs < ns) {
			return false;
		}
		std::string ending = haystack.substr(hs - ns, ns);
		for (size_t i=0; i < ending.size(); i++) {
			ending[i] = tolower(ending[i]);
		}
		return ending.compare(needle) == 0;
	}

	bool string_starts_with(const std::wstring& haystack, const std::wstring& needle) {
		if (haystack.size() < needle.size()) {
			return false;
		}
		const std::wstring& ending = haystack.substr(0, needle.size());
		return ending.compare(needle) == 0;
	}

	bool check_file(const std::string& filepath) {
		struct stat sb;
		if (stat(filepath.c_str(), &sb) != 0) {
			ratesong::config::error("Unable to stat file %s: File not found or unable to get status.",
									 filepath.c_str());
			return false;
		}

		if (access(filepath.c_str(), R_OK) != 0) {
			ratesong::config::error("Unable to access file %s: No read access.",
									 filepath.c_str());
			return false;
		}

		if (!S_ISREG(sb.st_mode) && !S_ISLNK(sb.st_mode)) {
			ratesong::config::error("Unable to access file %s: Not a regular file or symlink.",
									 filepath.c_str());
			return false;
		}

		return true;
	}

	bool xiph_rating(TagLib::Ogg::XiphComment* xiphcomment,
					 ratesong::rating_t& out) {
		TagLib::Ogg::FieldListMap map = xiphcomment->fieldListMap();
		for (TagLib::Ogg::FieldListMap::Iterator
				 it = map.begin(); it != map.end(); ++it) {
			std::wstring key = (*it).first.toWString();
			if (string_starts_with(key,L"RATING:")) {
				const TagLib::List<TagLib::String>& val = (*it).second;
				if (val.size() == 1) {
					//expecting [0.0, 0.2, 0.4, 0.6, 0.8, 1.0]
					const TagLib::String& floatstr = val[0];
					double ogg_rating;
					std::istringstream stream(floatstr.toCString());
					stream >> ogg_rating;
					if (stream.fail()) {
						continue;
					}
					if (ogg_rating == 0.5) {//unrated
						return false;
					} else if (ogg_rating > 0.8) {// (0.8,1.0]
						out = 5;
					} else if (ogg_rating > 0.6) {// (0.6,0.8]
						out = 4;
					} else if (ogg_rating > 0.4) {// (0.4,0.6]
						out = 3;
					} else if (ogg_rating > 0.2) {// (0.2,0.4]
						out = 2;
					} else {// [0.0,0.2]
						out = 1;
					}
					return true;
				}
			}
		}
		return false;
	}

	bool id3v2_rating(TagLib::ID3v2::Tag* id3v2tag,
					  ratesong::rating_t& out) {
		const TagLib::ID3v2::FrameListMap& map = id3v2tag->frameListMap();
		if (map.contains("POPM")) {
			const TagLib::List<TagLib::ID3v2::Frame*>& vallist = map["POPM"];
			for (TagLib::List<TagLib::ID3v2::Frame*>::ConstIterator
					 frame = vallist.begin(); frame != vallist.end(); ++frame) {
				TagLib::ID3v2::PopularimeterFrame* popmframe =
					static_cast<TagLib::ID3v2::PopularimeterFrame*>(*frame);
				int popm_rating = popmframe->rating();
				if (popm_rating == 0) {
					return false;
				} else if (popm_rating < 64) {
					out = 1;
				} else if (popm_rating < 128) {
					out = 2;
				} else if (popm_rating < 192) {
					out = 3;
				} else if (popm_rating < 255) {
					out = 4;
				} else {
					out = 5;
				}
				return true;
			}
		}
		return false;
	}

	enum file_type_t { UNKNOWN, MP3, OGG, FLAC };
	file_type_t get_type(const ratesong::song_t& file) {
		if (string_ends_with_ci(file, ".mp3")) {
			return MP3;
		} else if (string_ends_with_ci(file, ".ogg") ||
				   string_ends_with_ci(file, ".oga")) {
			return OGG;
		} else if (string_ends_with_ci(file, ".flac")) {
			return FLAC;
		}
		return UNKNOWN;
	}

	bool list_dir(const std::string& root, std::list<ratesong::song_t>& songs_out) {
		std::queue<std::string> dirqueue;
		dirqueue.push(root);

		while (!dirqueue.empty()) {
			std::string dir = dirqueue.front();
			dirqueue.pop();

			DIR* dp = opendir(dir.c_str());
			if (dp == NULL) {
				ratesong::config::error("Couldn't open directory %s", dir.c_str());
				return false;
			}

			struct dirent* ep;
			while (ep = readdir(dp)) {
				//"This is the only field you can count on in all POSIX systems":
				ratesong::song_t filepath = dir+ep->d_name;
				ratesong::config::debug(filepath.c_str());

				struct stat sb;
				if (stat(filepath.c_str(), &sb) != 0) {
					ratesong::config::error("Unable to stat file %s.",
											filepath.c_str());
					closedir(dp);
					return false;
				}
				if (S_ISDIR(sb.st_mode)) {
					dirqueue.push(filepath+SEP);
				} else if (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode)) {
					if (get_type(filepath) != UNKNOWN) {
						songs_out.push_back(filepath);
					}
				}
			}
			closedir(dp);
		}

		return true;
	}

	bool rating(const ratesong::song_t& song, file_type_t type,
				ratesong::rating_t& out) {
		switch (type) {
		case MP3:
			{
				TagLib::MPEG::File mpegfile(song.c_str(), false);
				TagLib::ID3v2::Tag* id3v2tag = mpegfile.ID3v2Tag();
				if (id3v2tag) {
					return id3v2_rating(id3v2tag, out);
				}
			}
			break;
		case OGG:
			{
				TagLib::Ogg::Vorbis::File oggfile(song.c_str(), false);
				TagLib::Ogg::XiphComment* xiphcomment = oggfile.tag();
				if (xiphcomment) {
					return xiph_rating(xiphcomment, out);
				}

				TagLib::Ogg::FLAC::File oggflacfile(song.c_str(), false);
				xiphcomment = oggflacfile.tag();
				if (xiphcomment) {
					return xiph_rating(xiphcomment, out);
				}
			}
			break;
		case FLAC:
			{
				TagLib::FLAC::File flacfile(song.c_str(), false);
				TagLib::Ogg::XiphComment* xiphcomment = flacfile.xiphComment();
				if (xiphcomment) {
					return xiph_rating(xiphcomment, out);
				}

				TagLib::ID3v2::Tag* id3v2tag = flacfile.ID3v2Tag();
				if (id3v2tag) {
					return id3v2_rating(id3v2tag, out);
				}
			}
			break;
		case UNKNOWN:
			ratesong::config::error("INTERNAL ERROR: queried rating against UNKNOWN type");
			break;
		default:
			ratesong::config::error("UNKNOWN ENUM: %d", type);
			break;
		}

		return false;
	}
}

bool ratesong::media::Access::ratings(std::map<song_t,rating_t>& out_ratings,
									  std::set<song_t>& out_unrated) {
	std::list<song_t> songs;
	if (!list_dir(music_dir, songs)) {
		return false;
	}
	return ratings(songs, out_ratings, out_unrated);
}

bool ratesong::media::Access::ratings(const std::list<song_t>& songs,
									  std::map<song_t,rating_t>& out_ratings,
									  std::set<song_t>& out_unrated) {
	bool ret = true;

	for (std::list<song_t>::const_iterator
			 it = songs.begin(); it != songs.end(); it++) {
		song_t songpath(music_dir + *it);
		if (!check_file(songpath)) {
			ret = false;
		}

		file_type_t type = get_type(songpath);
		if (type == UNKNOWN) {
			config::log("Unsupported file %s", songpath.c_str());
			continue;
		}

		rating_t r;
		if (rating(songpath, type, r)) {
			config::debug("RATING %s = %d", it->c_str(), r);
			out_ratings.insert(std::make_pair(*it,r));
		} else {
			config::debug("RATING %s = UNRATED", it->c_str(), r);
			out_unrated.insert(*it);
		}
	}

	return ret;
}
