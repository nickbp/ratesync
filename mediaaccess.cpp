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

}

bool mpdtagger::media::File::rating(rating_t& out) {
	const char* songpath_s = path.c_str();
	//TODO determine tag type properly by checking null tag (dont depend on filename)
	if (string_ends_with(path,".ogg")) {
		TagLib::Ogg::Vorbis::File oggfile(songpath_s);
		TagLib::Ogg::XiphComment* xiphcomment = oggfile.tag();
		if (xiphcomment) {
			return xiph_rating(xiphcomment, out);
		}
	} else if (string_ends_with(path,".flac")) {
		TagLib::FLAC::File flacfile(songpath_s);
		TagLib::Ogg::XiphComment* xiphcomment = flacfile.xiphComment();
		if (xiphcomment) {
			return xiph_rating(xiphcomment, out);
		}
	} else if (string_ends_with(path,".mp3")) {
		TagLib::MPEG::File mpegfile(songpath_s);
		TagLib::ID3v2::Tag* id3v2tag = mpegfile.ID3v2Tag();
		if (id3v2tag) {
			return id3v2_rating(id3v2tag, out);
		}
	}
	return false;
}

mpdtagger::media::Access::Access(const std::string& dir) : dir(dir) {
	check_file(dir, true, true);
}

void mpdtagger::media::Access::ratings(const std::list<mpd::Song>& mpd_songs,
									   std::map<mpd::Song,rating_t>& out_ratings,
									   std::set<mpd::Song>& out_unrated) {
	for (std::list<mpd::Song>::const_iterator it = mpd_songs.begin();
		 it != mpd_songs.end(); it++) {
		std::string songpath(dir+it->uri());
		check_file(songpath, false, true);

		try {
			File media(songpath);
			rating_t r;
			if (media.rating(r)) {
				config::debug("RATING %s = %d", it->uri().c_str(), r);
				out_ratings.insert(std::make_pair(*it,r));
			} else {
				config::debug("RATING %s = UNRATED", it->uri().c_str(), r);
				out_unrated.insert(*it);
			}
		} catch (...) {
			config::log("Unsupported file: ", songpath.c_str());
		}
	}
}

bool mpdtagger::media::Access::check_file(const std::string& filepath,
										bool isdir, bool throw_err/*=false*/) {
	struct stat sb;
	if (stat(filepath.c_str(), &sb) == -1) {
		if (throw_err) {
			std::ostringstream oss;
			oss << "File not found: " << filepath;
			throw Error(oss.str());
		}
		return false;
	} else if (isdir && !S_ISDIR(sb.st_mode)) {
		if (throw_err) {
			std::ostringstream oss;
			oss << "Not a directory: " << filepath;
			throw Error(oss.str());
		}
		return false;
	} else if (!isdir && !S_ISREG(sb.st_mode)) {
		if (throw_err) {
			std::ostringstream oss;
			oss << "Not a regular file: " << filepath;
			throw Error(oss.str());
		}
		return false;
	}
	return true;
}
