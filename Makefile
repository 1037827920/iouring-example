regular_cat: regular_cat.c
		gcc -o $@ $<

iouring_cat: iouring_cat.c
		gcc -o $@ $<

liburing_cat: liburing_cat.c
		gcc -o $@ $< -luring

all: regular_cat iouring_cat liburing_cat
.PHONY: clean

clean:
	rm -f regular_cat iouring_cat liburing_cat
