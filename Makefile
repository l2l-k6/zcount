CFLAGS = -O2 -Wall
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

INSTALL = /usr/bin/install

.PHONY: all clean install

all: zcount

install: all
	$(INSTALL) --owner=root --group=root --mode=0755 zcount $(BINDIR)

clean:
	$(RM) -f *~ *.o zcount

zcount: zcount.o
	$(CC) $(CFLAGS) -o $@ $<

zcount.o: zcount.c
	$(CC) $(CFLAGS) -c -o $@ $<

