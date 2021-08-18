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

ARCHDIR=$(top_srcdir)/src/$(ARCH)

bin_PROGRAMS = bcc
bcc_SOURCES = src/bcc.c src/cpp.c src/error.c src/expr.c src/func.c src/ir.c src/irgen.c src/lex.c \
				  src/main.c src/optim_expr.c src/optim_ir.c src/optim_stmt.c src/scope.c src/stmt.c \
				  src/strdb.c src/strint.c src/target.c src/token.c src/unit.c src/value.c src/vtype.c

bcc_CPPFLAGS = -DBCC_TARGET=\"$(TARGET)\" -DBCC_ARCH=\"$(ARCH)\" -DBCC_FULL_ARCH=\"$(FULL_ARCH)\" \
					-I$(top_srcdir)/include -DBCC_$(FULL_ARCH)=1 -DBITS=$(BITS) \
					-DBCPP_PATH=\"$(bindir)/`echo bcpp | sed '$(transform)'`\" \
					-D_XOPEN_SOURCE=700 -DSBITS=\"$(BITS)\" -DTARGET_INCLUDE_DIR=\"$(includedir)\"
bcc_CFLAGS = -Wextra

if ARCH_x86
bcc_SOURCES += src/x86/builtins.c \
					src/x86/gen.c \
					src/x86/optim.c \
					src/x86/target.c
endif
if ARCH_riscv
bcc_SOURCES += src/riscv/builtins.c \
				 	src/riscv/common.c \
				 	src/riscv/cpu.c \
				 	src/riscv/emit_ir.c \
				 	src/riscv/gen.c \
				 	src/riscv/optim.c \
				 	src/riscv/regs.c \
				 	src/riscv/target.c
endif

include_HEADERS = bcc-include/bcc-config.h	\
						bcc-include/stdbool.h		\
						bcc-include/stddef.h			\
						bcc-include/stdint.h			\
						bcc-include/iso646.h			\
						bcc-include/limits.h

print-config:
	@echo targetdir: $(targetdir)
	@echo includedir: $(includedir)
	@echo arch: $(ARCH)
	@echo target: $(TARGET)
	@echo enable-fp: $(ENABLE_FP)
	@echo sources: $(bcc_SOURCES)

man1_MANS = bcc.1

completionsdir = $(datadir)/bash-completion/completions
transform = @program_transform_name@

install-data-local:
	install -Dm644 util/bcc.bash $(DESTDIR)/$(completionsdir)/`echo bcc | sed '$(transform)'`

uninstall-local:
	rm -f $(DESTDIR)/$(completionsdir)/`echo bcc | sed '$(transform)'`

full-clean: clean
	$(MAKE) -C cpp full-clean
	$(MAKE) -C test clean
	$(MAKE) -C test2 clean
	rm -f Makefile Makefile.in configure src/.dirstamp src/$(ARCH)/.dirstamp
	rm -f config.h config.h.in config.h.in~ config.log config.status stamp-h1 aclocal.m4
	rm -rf autom4te.cache build-aux src/.deps .deps src/$(ARCH)/.deps
	rm -f help_options.h bcc.1 configure~

test: all
	$(MAKE) -C test

check: test

.PHONY: clean-local test
