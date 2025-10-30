regular_cat: regular_cat.c
		gcc -o $@ $<

all: regular_cat
.PHONY: clean

clean:
	rm -f regular_cat
