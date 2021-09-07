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

SUBDIRS = cpp

tbin_PROGRAMS = bcc
bcc_SOURCES = src/bcc.c src/cmdline.c src/cpp.c src/error.c src/expr.c src/func.c src/ir.c 	\
				  src/irgen.c src/lex.c src/linker.c src/main.c src/optim_expr.c src/optim_ir.c	\
				  src/optim_stmt.c src/scope.c src/stmt.c src/strdb.c src/strint.c src/target.c	\
				  src/token.c src/unit.c src/value.c src/vtype.c

bcc_CPPFLAGS = -DBCPP_PATH=\"$(bindir)/`echo bcpp | sed '$(transform)'`\" \
					-D_XOPEN_SOURCE=700 -DPREFIX=\"${prefix}\" \
					-I$(top_srcdir)/include

bcc_CFLAGS = -Wextra

include_HEADERS = bcc-include/bcc-config.h	\
						bcc-include/stdbool.h		\
						bcc-include/stddef.h			\
						bcc-include/stdint.h			\
						bcc-include/iso646.h			\
						bcc-include/limits.h			\
						bcc-include/stdnoreturn.h

if ARCH_x86
include $(srcdir)/src/x86/Makefile.am
endif

if ARCH_riscv
include $(srcdir)/src/riscv/Makefile.am
endif


print-config:
	@echo targetdir: $(targetdir)
	@echo includedir: $(includedir)
	@echo arch: $(ARCH)
	@echo target: $(TARGET)
	@echo enable-fp: $(ENABLE_FP)
	@echo sources: $(bcc_SOURCES)
	@echo version: $(version)

man1_MANS = bcc.1

completionsdir = $(datadir)/bash-completion/completions

transform = @program_transform_name@

all-bcc: bcc
all: all-recursive all-target-libbcc

install-completions:
	[ $(ENABLE_BASHCOMP) = 0 ] || \
		install -Dm644 util/bcc.bash $(DESTDIR)/$(completionsdir)/`echo bcc | sed '$(transform)'`

uninstall-completions:
	[ $(ENABLE_BASHCOMP) = 0 ] || \
		rm -f $(DESTDIR)/$(completionsdir)/`echo bcc | sed '$(transform)'`


install-data-local: install-target-libbcc install-completions
	install -dm755 $(DESTDIR)/$(tbindir)
	[ $(ENABLE_BCL) = 0 ] || install -m755 $(srcdir)/util/bcl $(DESTDIR)/$(tbindir)/`echo bcl | sed '$(transform)'`
	install -dm755 $(DESTDIR)/$(bindir)
	mv $(DESTDIR)/$(bindir)/`echo bcpp | sed '$(transform)'` $(DESTDIR)/$(tbindir)/
	ln -sf $(tbindir)/bcc  $(DESTDIR)/$(bindir)/`echo bcc  | sed '$(transform)'`
	ln -sf $(tbindir)/bcpp $(DESTDIR)/$(bindir)/`echo bcpp | sed '$(transform)'`
	[ $(ENABLE_BCL) = 0 ] || ln -sf $(tbindir)/bcl  $(DESTDIR)/$(bindir)/`echo bcl  | sed '$(transform)'`
	if [ `echo file | sed '$(transform)'` != file ]; then \
		ln -sf `echo bcc  | sed '$(transform)'` $(DESTDIR)/$(tbindir)/bcc;	\
		ln -sf `echo bcpp | sed '$(transform)'` $(DESTDIR)/$(tbindir)/bcpp;	\
		[ $(ENABLE_BCL) = 0 ] || ln -sf `echo bcl  | sed '$(transform)'` $(DESTDIR)/$(tbindir)/bcl;	\
	fi

uninstall-local: uninstall-target-libbcc uninstall-completions
	[ $(ENABLE_BCL) = 0 ] || rm -f $(DESTDIR)/$(bindir)/bcl

full-clean: clean full-clean-target-libbcc
	$(MAKE) -C cpp full-clean
	$(MAKE) -C test clean
	$(MAKE) -C test2 clean
	rm -f Makefile Makefile.in configure src/.dirstamp src/$(ARCH)/.dirstamp
	rm -f config.h config.h.in config.h.in~ config.log config.status stamp-h1 aclocal.m4
	rm -rf autom4te.cache build-aux src/.deps .deps src/$(ARCH)/.deps
	rm -f help_options.h bcc.1 configure~


if ENABLE_LIBBCC
configure-target-libbcc: bcc
	cd $(srcdir)/libbcc && CC="../bcc -e ../cpp/bcpp" ./configure --host=$(TARGET)
all-target-libbcc: configure-target-libbcc
	$(MAKE) -C libbcc
install-target-libbcc:
	$(MAKE) -C libbcc DESTDIR=$(DESTDIR) install
uninstall-target-libbcc:
	$(MAKE) -C libbcc DESTDIR=$(DESTDIR) uninstall
full-clean-target-libbcc:
	if test -e libbcc/Makefile; then					\
		$(MAKE) -C libbcc full-clean;					\
	else														\
		cd libbcc; 											\
		rm -f Makefile.in aclocal.m4 configure;	\
		rm -rf build-aux/ autom4te.cache;			\
	fi
else
all-target-libbcc: bcc
install-target-libbcc:
uninstall-target-libbcc:
full-clean-target-libbcc:
endif


test: all
	$(MAKE) -C test

check: test

.PHONY: clean-local test
