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

#include "mediaaccess.h"
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
#include <sys/stat.h>

namespace {

	bool string_ends_with(string const haystack, string const needle) {
		if (haystack.size() < needle.size()) {
			return false;
		}
		std::string ending = haystack.substr(haystack.size() - needle.size(),
											 needle.size());
		for (unsigned int i=0; i < ending.length(); i++) {
			ending[i] = tolower(ending[i]);
		}
		return ending.compare(needle) == 0;
	}

	bool string_starts_with(std::wstring const haystack, std::wstring const needle) {
		if (haystack.size() < needle.size()) {
			return false;
		}
		const std::wstring& ending = haystack.substr(0, needle.size());
		return ending.compare(needle) == 0;
	}

	void check_file(const std::string& filepath) {
		struct stat sb;
		if (stat(filepath.c_str(), &sb) != 0) {
			std::ostringstream oss;
			oss << "File not found/unable to get status: " << filepath;
			throw mpdtagger::media::Error(oss.str());
		}

		if (access(filepath.c_str(), R_OK) != 0) {
			std::ostringstream oss;
			oss << "Read access denied: " << filepath;
			throw mpdtagger::media::Error(oss.str());
		}

		if (!S_ISREG(sb.st_mode)) {
			std::ostringstream oss;
			oss << "Not a regular file: " << filepath;
			throw mpdtagger::media::Error(oss.str());
		}
	}

	bool xiph_rating(TagLib::Ogg::XiphComment* xiphcomment,
					 mpdtagger::rating_t& out) {
		TagLib::Ogg::FieldListMap map = xiphcomment->fieldListMap();
		for (TagLib::Ogg::FieldListMap::Iterator it = map.begin();
			 it != map.end(); ++it) {
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
					  mpdtagger::rating_t& out) {
		const TagLib::ID3v2::FrameListMap& map = id3v2tag->frameListMap();
		if (map.contains("POPM")) {
			const TagLib::List<TagLib::ID3v2::Frame*>& vallist = map["POPM"];
			for (TagLib::List<TagLib::ID3v2::Frame*>::ConstIterator frame = vallist.begin();
				 frame != vallist.end(); ++frame) {
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

	bool rating(const mpdtagger::song_t& song, mpdtagger::rating_t& out) {
		const char* songpath_s = song.c_str();
		if (string_ends_with(song, ".mp3")) {
			TagLib::MPEG::File mpegfile(songpath_s, false);
			TagLib::ID3v2::Tag* id3v2tag = mpegfile.ID3v2Tag();
			if (id3v2tag) {
				return id3v2_rating(id3v2tag, out);
			}
		} else if (string_ends_with(song, ".ogg") ||
				   string_ends_with(song, ".oga")) {
			TagLib::Ogg::Vorbis::File oggfile(songpath_s, false);
			TagLib::Ogg::XiphComment* xiphcomment = oggfile.tag();
			if (xiphcomment) {
				return xiph_rating(xiphcomment, out);
			}

			TagLib::Ogg::FLAC::File oggflacfile(songpath_s, false);
			xiphcomment = oggflacfile.tag();
			if (xiphcomment) {
				return xiph_rating(xiphcomment, out);
			}
		} else if (string_ends_with(song, ".flac")) {
			TagLib::FLAC::File flacfile(songpath_s, false);
			TagLib::Ogg::XiphComment* xiphcomment = flacfile.xiphComment();
			if (xiphcomment) {
				return xiph_rating(xiphcomment, out);
			}

			TagLib::ID3v2::Tag* id3v2tag = flacfile.ID3v2Tag();
			if (id3v2tag) {
				return id3v2_rating(id3v2tag, out);
			}
		}
		return false;
	}
}

void mpdtagger::media::Access::ratings(std::map<song_t,rating_t>& out_ratings,
									   std::set<song_t>& out_unrated) {
	std::list<song_t> songs;//TODO get list of songs in directory
	ratings(songs, out_ratings, out_unrated);
}

void mpdtagger::media::Access::ratings(const std::list<song_t>& songs,
									   std::map<song_t,rating_t>& out_ratings,
									   std::set<song_t>& out_unrated) {
	for (std::list<song_t>::const_iterator it = songs.begin();
		 it != songs.end(); it++) {
		std::string songpath(music_dir + *it);//TODO proper path support
		check_file(songpath);

		try {
			rating_t r;
			if (rating(songpath, r)) {
				config::debug("RATING %s = %d", it->c_str(), r);
				out_ratings.insert(std::make_pair(*it,r));
			} else {
				config::debug("RATING %s = UNRATED", it->c_str(), r);
				out_unrated.insert(*it);
			}
		} catch (...) {
			config::log("Unsupported file: ", songpath.c_str());
		}
	}
}
