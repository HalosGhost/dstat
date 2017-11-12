PROGNM =  dstat
PREFIX ?= /usr/local
DOCDIR ?= $(DESTDIR)$(PREFIX)/share/man
LIBDIR ?= $(DESTDIR)$(PREFIX)/lib
BINDIR ?= $(DESTDIR)$(PREFIX)/bin
ZSHDIR ?= $(DESTDIR)$(PREFIX)/share/zsh
BSHDIR ?= $(DESTDIR)$(PREFIX)/share/bash-completions

CC = clang
LDFLAGS = `pkg-config --libs-only-l x11 alsa`
CFLAGS ?= -g -flto -O2 -fPIE -pie -D_FORTIFY_SOURCE=2 -fstack-protector-strong --param=ssp-buffer-size=1 -Weverything -Werror -std=c11 -fsanitize=undefined -fsanitize-trap=undefined -Wl,-z,relro,-z,now

.PHONY: all clean cov-build install uninstall

all: dist
	@$(CC) $(CFLAGS) $(LDFLAGS) src/dstat.c -o dist/$(PROGNM)

clean:
	@rm -rf -- dist cov-int $(PROGNM).tgz make.sh

cov-build: dist clean
	@cov-build --dir cov-int make
	@tar czvf $(PROGNM).tgz cov-int

dist:
	@mkdir -p dist

install:
	@install -Dm755 dist/$(PROGNM) $(BINDIR)/$(PROGNM)

uninstall:
	@rm -f $(BINDIR)/$(PROGNM)
