PROGNM =  dstat
PREFIX ?= /usr/local
DOCDIR ?= $(DESTDIR)$(PREFIX)/share/man
LIBDIR ?= $(DESTDIR)$(PREFIX)/lib
BINDIR ?= $(DESTDIR)$(PREFIX)/bin
ZSHDIR ?= $(DESTDIR)$(PREFIX)/share/zsh
BSHDIR ?= $(DESTDIR)$(PREFIX)/share/bash-completions

include Makerules
CFLAGS += -Wno-invalid-noreturn
.PHONY: all clean cov-build install uninstall

all: dist
	@$(CC) $(CFLAGS) $(LDFLAGS) src/dstat.c -o dist/$(PROGNM)

clang-analyze:
	@(pushd ./src; clang-check -analyze ./*.c)

clean:
	@rm -rf -- dist cov-int $(PROGNM).tgz make.sh ./src/*.plist

cov-build: dist clean
	@cov-build --dir cov-int make
	@tar czvf $(PROGNM).tgz cov-int

dist:
	@mkdir -p dist

install:
	@install -Dm755 dist/$(PROGNM) $(BINDIR)/$(PROGNM)

uninstall:
	@rm -f $(BINDIR)/$(PROGNM)
