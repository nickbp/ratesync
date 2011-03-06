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

#include "mpd-access.h"

#include "config.h"

#include <mpd/client.h>
#include <sstream>

namespace {
	static const char* RATING_STICKER = "rating";

	bool rating_get(struct mpd_connection* conn, const ratesong::song_t& song,
					ratesong::rating_t& out) {
		struct mpd_pair* mpd_rating_pair;
		if (mpd_send_sticker_get(conn, "song", song.c_str(), RATING_STICKER) &&
			(mpd_rating_pair = mpd_recv_sticker(conn)) != NULL) {
			//extract rating (1-5) from sticker
			if (!mpd_response_finish(conn)) {
				ratesong::config::error("Failed to close sticker query");
				throw;
			}
			ratesong::rating_t mpd_rating;
			std::istringstream mpd_rating_stream(mpd_rating_pair->value);
			mpd_rating_stream >> mpd_rating;
			if (mpd_rating_stream.fail() || mpd_rating < 1 || mpd_rating > 5) {
				std::ostringstream oss;
				oss << "Mpd Song '" << song << "' : Unknown rating value '"
					<< mpd_rating_pair->value << "'";
				mpd_return_sticker(conn,mpd_rating_pair);
				ratesong::config::error("Mpd Song '%s': Unknown rating value '%s'",
										song.c_str(), mpd_rating_pair->value);
				return false;
			}
			mpd_return_sticker(conn,mpd_rating_pair);
			out = mpd_rating;
			return true;
		} else {
			//requested sticker is unset. clear error state and continue
			if (!mpd_connection_clear_error(conn)) {
				//it was a fatal error (not just unset sticker), abort
				ratesong::config::error("Failed to get sticker");
				throw;
			}
			return false;
		}
	}
}

ratesong::mpd::Access::Access(const std::string& host, size_t port)
	: host(host), port(port) { }

ratesong::mpd::Access::~Access() {
	if (conn != NULL) {
		mpd_connection_free(conn);
		conn = NULL;
	}
}

bool ratesong::mpd::Access::connect() {
	conn = mpd_connection_new(host.c_str(), port, 0);
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		config::error("Unable to connect to MPD Server @ %s:%d: %s",
					  host.c_str(), port, mpd_connection_get_error_message(conn));
		return false;
	}
	return true;
}

bool ratesong::mpd::Access::ratings(std::list<song_t>& out_all,
									std::map<song_t,rating_t>& out_rating) const {
	if (conn == NULL) {
		config::error("INTERNAL ERROR: called songs() before connect()");
		return false;
	}

	if (!mpd_send_list_all(conn,"")) {
		config::error("Got error when retrieving list of MPD songs: %s",
					  mpd_connection_get_error_message(conn));
		return false;
	}

	struct mpd_song* song = NULL;
	while ((song = mpd_recv_song(conn)) != NULL) {
		out_all.push_back(mpd_song_get_uri(song));
		mpd_song_free(song);
	}

	rating_t r;
	for (std::list<song_t>::const_iterator iter = out_all.begin();
		 iter != out_all.end(); iter++) {
		const song_t& song = *iter;
		try {
			if (rating_get(conn, song, r)) {
				out_rating.insert(std::make_pair(song,r));
			}
		} catch (...) {
			return false;
		}
	}

	return true;
}

bool ratesong::mpd::Access::rating_clear(const song_t& song) {
	if (conn == NULL) {
		config::error("INTERNAL ERROR: called rating_clear() before connect()");
		return false;
	}

	if (!mpd_run_sticker_delete(conn, "song", song.c_str(), RATING_STICKER)) {
		config::error("Mpd Song '%s': Error clearing rating sticker", song.c_str());
		return false;
	}

	return true;
}

bool ratesong::mpd::Access::rating_set(const song_t& song, rating_t rating) {
	if (conn == NULL) {
		config::error("INTERNAL ERROR: called rating_set() before connect()");
		return false;
	}

	std::ostringstream file_rating_ss;
	file_rating_ss << rating;
	const char* file_rating_s = file_rating_ss.str().c_str();

	if (!mpd_run_sticker_set(conn, "song", song.c_str(), RATING_STICKER, file_rating_s)) {
		config::error("Mpd Song '%s' : Error setting rating sticker", song.c_str());
		return false;
	}

	return true;
}
