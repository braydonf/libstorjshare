noinst_PROGRAMS = tests
tests_SOURCES = tests.c tests.h $(top_builddir)/src/storjshare.h
tests_LDADD = $(top_builddir)/src/libstorjshare.la
tests_LDFLAGS = -Wall -g

if BUILD_STORJ_DLL
tests_LDFLAGS += -lmicrohttpd
else
tests_LDFLAGS += -static -lmicrohttpd
endif

TESTS = tests
