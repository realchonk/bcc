#  Copyright (C) 2021 Benjamin Stürz
#  
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.

bin_PROGRAMS = bcpp
bcpp_SOURCES = src/cpp.c src/dir.c src/eval.c src/if.c src/include.c src/macro.c \
					src/main.c src/strint.c src/token.c src/util.c src/warn.c src/expand.c \
					src/spec_macros.c

bcpp_CPPFLAGS = -I$(top_srcdir)/include
bcpp_CFLAGS = -Wextra -std=c99 -D_XOPEN_SOURCE=700

man1_MANS = bcpp.1

completionsdir = $(datadir)/bash-completion/completions
transform = @program_transform_name@

install-data-local:
	install -Dm644 bcpp.bash $(DESTDIR)/$(completionsdir)/`echo bcpp | sed '$(transform)'`

uninstall-local:
	rm -f $(DESTDIR)/$(completionsdir)/`echo bcpp | sed '$(transform)'`

full-clean: clean
	rm -f Makefile Makefile.in configure src/.dirstamp
	rm -f config.h config.h.in config.log config.status stamp-h1 aclocal.m4
	rm -rf autom4te.cache build-aux src/.deps
	rm -f help_options.h bcpp.1 configure~

