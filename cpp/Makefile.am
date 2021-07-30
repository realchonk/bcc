bin_PROGRAMS = bcpp
bcpp_SOURCES = src/cpp.c src/dir.c src/eval.c src/if.c src/include.c src/macro.c \
					src/main.c src/strint.c src/token.c src/util.c src/warn.c
bcpp_CPPFLAGS = -I$(top_srcdir)/include
bcpp_CFLAGS = -Wextra -std=c99 -D_XOPEN_SOURCE=700

man1_MANS = bcpp.1

clean-local:
	rm -f Makefile Makefile.in configure src/.dirstamp
	rm -f config.h config.h.in config.log config.status stamp-h1 aclocal.m4
	rm -rf autom4te.cache build-aux src/.deps
	rm -f include/help_options.h bcpp.1

