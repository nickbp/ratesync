CC=g++
CFLAGS=-c -Wall -Wextra -pedantic -g
LDFLAGS=-ltag -lmpdclient

PROG=mpdtagger
SOURCES=mpdaccess.cpp mediaaccess.cpp tagger.cpp main.cpp
OBJFILES=$(SOURCES:.cpp=.o)

$(PROG): $(OBJFILES)
	$(CC) $(LDFLAGS) $(OBJFILES) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

all: $(SOURCES) $(PROG)

debug: CFLAGS := $(CFLAGS) -DDEBUGMSG
debug: all

clean: 
	rm -f $(OBJFILES) $(PROG)
