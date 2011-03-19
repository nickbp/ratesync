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

#include "sink-mpd.h"
#include "config.h"

#include <mpd/client.h>
#include <sstream>
#include <assert.h>

namespace {
	static const char* RATING_STICKER = "rating";

	bool rating_get(struct mpd_connection* conn, const ratesync::song_t& song,
					ratesync::rating_t& out) {
		struct mpd_pair* mpd_rating_pair;
		if (mpd_send_sticker_get(conn, "song", song.c_str(), RATING_STICKER) &&
			(mpd_rating_pair = mpd_recv_sticker(conn)) != NULL) {
			//extract rating (1-5) from sticker
			if (!mpd_response_finish(conn)) {
				ratesync::config::error("Failed to close sticker query");
				return false;
			}
			ratesync::rating_t mpd_rating;
			std::istringstream mpd_rating_stream(mpd_rating_pair->value);
			mpd_rating_stream >> mpd_rating;
			if (mpd_rating_stream.fail() || mpd_rating < 1 || mpd_rating > 5) {
				mpd_return_sticker(conn,mpd_rating_pair);
				ratesync::config::error("MPD Song '%s': Unknown rating value '%s'",
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
				ratesync::config::error("Failed to get sticker");
				return false;
			}
			out = UNRATED;
			return true;
		}
	}
}

ratesync::sink::Mpd::Mpd(const std::string& host, size_t port)
	: host(host), port(port), conn(NULL) { }

ratesync::sink::Mpd::~Mpd() {
	if (conn != NULL) {
		mpd_connection_free(conn);
		conn = NULL;
	}
}

bool ratesync::sink::Mpd::Get(std::map<song_t,rating_t>& out_rating) {
	conn = mpd_connection_new(host.c_str(), port, 0);
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		config::error("Unable to connect to MPD Server @ %s:%d: %s",
					  host.c_str(), port,
					  mpd_connection_get_error_message(conn));
		return false;
	}

	if (!mpd_send_list_all(conn,"")) {
		config::error("Got error when retrieving list of MPD songs: %s",
					  mpd_connection_get_error_message(conn));
		return false;
	}

	struct mpd_song* song_orig = NULL;
	while ((song_orig = mpd_recv_song(conn)) != NULL) {
		song_t song = mpd_song_get_uri(song_orig);
		mpd_song_free(song_orig);

		rating_t r;
		if (!rating_get(conn, song, r)) {
			return false;//immediately abort
		}

		out_rating.insert(std::make_pair(song, r));
	}

	return true;
}

bool ratesync::sink::Mpd::Set(const song_ratings_t& song) {
	std::ostringstream file_rating_ss;
	file_rating_ss << song.rating_new;
	const char* file_rating_s = file_rating_ss.str().c_str();

	assert(conn != NULL);
	if (!mpd_run_sticker_set(conn, "song", song.path.c_str(), RATING_STICKER, file_rating_s)) {
		config::error("MPD Song '%s' : Error setting rating sticker", song.path.c_str());
		return false;
	}

	return true;
}

bool ratesync::sink::Mpd::Clear(const song_rating_t& song) {
	assert(conn != NULL);
	if (!mpd_run_sticker_delete(conn, "song", song.path.c_str(), RATING_STICKER)) {
		config::error("MPD Song '%s': Error clearing rating sticker", song.path.c_str());
		return false;
	}

	return true;
}
