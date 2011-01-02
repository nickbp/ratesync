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

#include "mpdaccess.h"

#include <sstream>

namespace {
	static const char* RATING_STICKER = "rating";
}

mpdtagger::MpdAccess::MpdAccess(const std::string& host) {
	conn = mpd_connection_new(host.c_str(),0,0);
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		std::ostringstream oss;
		oss << "Unable to connect to MPD Server @ " << host << ": "
			<< mpd_connection_get_error_message(conn);
		throw MpdError(oss.str());
	}
}
mpdtagger::MpdAccess::~MpdAccess() {
	if (conn != NULL) {
		mpd_connection_free(conn);
		conn = NULL;
	}
}

void mpdtagger::MpdAccess::songs(std::list<mpdtagger::MpdSong>& out) const {
	if (!mpd_send_list_all(conn,"")) {
		std::ostringstream oss;
		oss << "Got error when retrieving list of MPD songs:"
			<< mpd_connection_get_error_message(conn);
		throw MpdError(oss.str());
	}

	struct mpd_song* song = NULL;
	while ((song = mpd_recv_song(conn)) != NULL) {
		out.push_back(MpdSong(conn, mpd_song_get_uri(song)));
		mpd_song_free(song);
	}
}

const std::string& mpdtagger::MpdSong::uri() const {
	return uri_;
}

unsigned int mpdtagger::MpdSong::rating() const {
	struct mpd_pair* mpd_rating_pair;
	if (mpd_send_sticker_get(conn, "song", uri().c_str(), RATING_STICKER) &&
		(mpd_rating_pair = mpd_recv_sticker(conn)) != NULL) {
		//extract rating (1-5) from sticker
		if (!mpd_response_finish(conn)) {
			throw MpdError("Exception when closing sticker query");
		}
		int mpd_rating = MpdAccess::UNRATED;
		std::istringstream mpd_rating_stream(mpd_rating_pair->value);
		mpd_rating_stream >> mpd_rating;
		if (mpd_rating_stream.fail() || mpd_rating < 1 || mpd_rating > 5) {
			std::ostringstream oss;
			oss << "Mpd Song '" << uri() << "' : Unknown rating value '"
				<< mpd_rating_pair->value << "'";
			throw MpdError(oss.str());
		}
		mpd_return_sticker(conn,mpd_rating_pair);
		return mpd_rating;
	} else {
		//requested sticker is unset. clear error state and continue
		if (!mpd_connection_clear_error(conn)) {
			//it was a fatal error (not just unset sticker), abort
			throw MpdError("Exception when getting sticker");
		}
		return MpdAccess::UNRATED;
	}
}

void mpdtagger::MpdSong::rating_clear() {
	if (!mpd_run_sticker_delete(conn, "song", uri().c_str(), RATING_STICKER)) {
		std::ostringstream oss;
		oss << "Mpd Song '" << uri() << "' : Error clearing rating sticker";
		//TODO any err string to get? any errstate to clear?
		throw MpdError(oss.str());
	}
}

void mpdtagger::MpdSong::rating_set(unsigned int rating) {
	std::ostringstream file_rating_ss;
	file_rating_ss << rating;
	const char* file_rating_s = file_rating_ss.str().c_str();

	if (!mpd_run_sticker_set(conn, "song", uri().c_str(), RATING_STICKER, file_rating_s)) {
		std::ostringstream oss;
		oss << "Mpd Song '" << uri() << "' : Error setting rating sticker";
		//TODO any err string to get? any errstate to clear?
		throw MpdError(oss.str());
	}
}
