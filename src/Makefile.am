lib_LTLIBRARIES = libstorjshare.la
libstorjshare_la_SOURCES = storjshare.c shareshare.h
libstorjshare_la_LIBADD = -lcurl -ljson-c -luv -lmicrohttpd -lsqlite3 -lm
libstorjshare_la_LDFLAGS = -Wall
if BUILD_STORJ_DLL
libstorjshare_la_LDFLAGS += -no-undefined
endif
include_HEADERS = storjshare.h

bin_PROGRAMS = storjshare
storjshare_SOURCES = daemon.c storjshare.h
storjshare_LDADD = libstorjshare.la
if BUILD_STORJ_DLL
storjshare_LDFLAGS = -Wall
else
storjshare_LDFLAGS = -Wall -static
endif
