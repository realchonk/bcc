SUBDIRS = cpp

bin_PROGRAMS = bcc
bcc_SOURCES = src/bcc.c src/cpp.c src/error.c src/expr.c src/func.c src/ir.c src/irgen.c src/lex.c \
				  src/main.c src/optim_expr.c src/optim_ir.c src/optim_stmt.c src/scope.c src/stmt.c \
				  src/strdb.c src/strint.c src/target.c src/token.c src/unit.c src/value.c src/vtype.c

bcc_CPPFLAGS = -DBCC_ARCH=\"$(TARGET)\" -I$(top_srcdir)/include

include src/x86_64/Makefile.am

print-config:
	@echo target: $(TARGET)
	@echo enable-fp: $(ENABLE_FP)
	@echo sources: $(bcc_SOURCES)

man1_MANS = bcc.1

clean-local:
	rm -f Makefile Makefile.in configure src/.dirstamp src/$(TARGET)/.dirstamp
	rm -f config.h config.h.in config.log config.status stamp-h1 aclocal.m4
	rm -rf autom4te.cache build-aux src/.deps .deps src/$(TARGET)/.deps
