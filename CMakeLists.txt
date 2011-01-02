cmake_minimum_required (VERSION 2.6)

project(mpdtagger)

SET(SRCS
	main.cpp
	mediaaccess.h
	mediaaccess.cpp
	mpdaccess.h
	mpdaccess.cpp
	tagger.h
	tagger.cpp)

add_executable(mpdtagger ${SRCS})
target_link_libraries(mpdtagger tag mpdclient)
