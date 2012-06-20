CFLAGS = -O2 -Wall

.PHONY: all clean install uninstall

all: zcount

install:

clean:
	$(RM) -f *~ *.o zcount

uninstall:

zcount: zcount.o
	$(CC) $(CFLAGS) -o $@ $<

zcount.o: zcount.c
	$(CC) $(CFLAGS) -c -o $@ $<

