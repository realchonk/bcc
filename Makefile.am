SUBDIRS = cpp

ARCHDIR=$(top_srcdir)/src/$(ARCH)

bin_PROGRAMS = bcc
bcc_SOURCES = src/bcc.c src/cpp.c src/error.c src/expr.c src/func.c src/ir.c src/irgen.c src/lex.c \
				  src/main.c src/optim_expr.c src/optim_ir.c src/optim_stmt.c src/scope.c src/stmt.c \
				  src/strdb.c src/strint.c src/target.c src/token.c src/unit.c src/value.c src/vtype.c

bcc_CPPFLAGS = -DBCC_ARCH=\"$(ARCH)\" -I$(top_srcdir)/include -DBCC_$(ARCH)=1
bcc_CFLAGS = -Wextra

if ARCH_x86
bcc_SOURCES += $(ARCHDIR)/builtins.c \
					$(ARCHDIR)/gen.c \
					$(ARCHDIR)/optim.c \
					$(ARCHDIR)/target.c
endif
if ARCH_riscv
bcc_SOURCES+=$(ARCHDIR)/builtins.c \
				 $(ARCHDIR)/common.c \
				 $(ARCHDIR)/cpu.c \
				 $(ARCHDIR)/emit_ir.c \
				 $(ARCHDIR)/gen.c \
				 $(ARCHDIR)/optim.c \
				 $(ARCHDIR)/regs.c \
				 $(ARCHDIR)/target.c
endif

print-config:
	@echo target: $(ARCH)
	@echo enable-fp: $(ENABLE_FP)
	@echo sources: $(bcc_SOURCES)

man1_MANS = bcc.1

clean-local:
	rm -f Makefile Makefile.in configure src/.dirstamp src/$(ARCH)/.dirstamp
	rm -f config.h config.h.in config.h.in~ config.log config.status stamp-h1 aclocal.m4
	rm -rf autom4te.cache build-aux src/.deps .deps src/$(ARCH)/.deps
	rm -f include/help_options.h bcc.1 configure~

test: all
	$(MAKE) -C test

check: test

.PHONY: clean-local test
