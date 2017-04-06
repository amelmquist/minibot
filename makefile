all:
	gcc -o jsreader *.c -Wall -Wextra -pedantic -std=c99 `pkg-config --cflags --libs libevdev` -l bcm2835 -g

clean:
	rm -f jsreader
