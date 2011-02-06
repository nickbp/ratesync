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

#include <mpd/client.h>
#include <sstream>

namespace {
	static const char* RATING_STICKER = "rating";
}

mpdtagger::mpd::Access::Access(const std::string& host, size_t port)
	: host(host), port(port) { }

mpdtagger::mpd::Access::~Access() {
	if (conn != NULL) {
		mpd_connection_free(conn);
		conn = NULL;
	}
}

void mpdtagger::mpd::Access::connect() {
	conn = mpd_connection_new(host.c_str(), port, 0);
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		std::ostringstream oss;
		oss << "Unable to connect to MPD Server @ "
			<< host << ":" << port << ": "
			<< mpd_connection_get_error_message(conn);
		throw Error(oss.str());
	}
}

void mpdtagger::mpd::Access::songs(std::list<song_t>& out) const {
	if (conn == NULL) {
		throw Error("INTERNAL ERROR: called songs() before connect()");
	}
			
	if (!mpd_send_list_all(conn,"")) {
		std::ostringstream oss;
		oss << "Got error when retrieving list of MPD songs:"
			<< mpd_connection_get_error_message(conn);
		throw Error(oss.str());
	}

	struct mpd_song* song = NULL;
	while ((song = mpd_recv_song(conn)) != NULL) {
		out.push_back(mpd_song_get_uri(song));
		mpd_song_free(song);
	}
}

bool mpdtagger::mpd::Access::rating_get(const song_t& song, rating_t& out) const {
	if (conn == NULL) {
		throw Error("INTERNAL ERROR: called rating() before connect()");
	}

	struct mpd_pair* mpd_rating_pair;
	if (mpd_send_sticker_get(conn, "song", song.c_str(), RATING_STICKER) &&
		(mpd_rating_pair = mpd_recv_sticker(conn)) != NULL) {
		//extract rating (1-5) from sticker
		if (!mpd_response_finish(conn)) {
			throw Error("Exception when closing sticker query");
		}
		rating_t mpd_rating;
		std::istringstream mpd_rating_stream(mpd_rating_pair->value);
		mpd_rating_stream >> mpd_rating;
		if (mpd_rating_stream.fail() || mpd_rating < 1 || mpd_rating > 5) {
			std::ostringstream oss;
			oss << "Mpd Song '" << song << "' : Unknown rating value '"
				<< mpd_rating_pair->value << "'";
			mpd_return_sticker(conn,mpd_rating_pair);
			throw Error(oss.str());
		}
		mpd_return_sticker(conn,mpd_rating_pair);
		out = mpd_rating;
		return true;
	} else {
		//requested sticker is unset. clear error state and continue
		if (!mpd_connection_clear_error(conn)) {
			//it was a fatal error (not just unset sticker), abort
			throw Error("Exception when getting sticker");
		}
		return false;
	}
}

void mpdtagger::mpd::Access::rating_clear(const song_t& song) {
	if (conn == NULL) {
		throw Error("INTERNAL ERROR: called rating_clear() before connect()");
	}

	if (!mpd_run_sticker_delete(conn, "song", song.c_str(), RATING_STICKER)) {
		std::ostringstream oss;
		oss << "Mpd Song '" << song << "' : Error clearing rating sticker";
		throw Error(oss.str());
	}
}

void mpdtagger::mpd::Access::rating_set(const song_t& song, rating_t rating) {
	if (conn == NULL) {
		throw Error("INTERNAL ERROR: called rating_set() before connect()");
	}

	std::ostringstream file_rating_ss;
	file_rating_ss << rating;
	const char* file_rating_s = file_rating_ss.str().c_str();

	if (!mpd_run_sticker_set(conn, "song", song.c_str(), RATING_STICKER, file_rating_s)) {
		std::ostringstream oss;
		oss << "Mpd Song '" << song << "' : Error setting rating sticker";
		throw Error(oss.str());
	}
}
