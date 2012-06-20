.PHONY: all clean install uninstall

all: zcount

install:

clean:
	$(RM) -f *~ *.o zcount

uninstall:

zcount: zcount.o
	$(CC) -o $@ $<

zcount.o: zcount.c
	$(CC) -c -o $@ $<

