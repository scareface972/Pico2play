CC=gcc
CFLAGS=-fPIC -Wall -Wextra -O2 -g -I lib
LDFLAGS=-lm lib/libttspico.a /usr/lib/arm-linux-gnueabihf/libportaudio.so -lpthread -lasound -ljack 
EXEC=pico2play

all: $(EXEC)

pico2play: pico2play.o
	$(CC) -o pico2play pico2play.o $(LDFLAGS)

pico2play.o: bin/pico2play.c
	$(CC) -o pico2play.o -c bin/pico2play.c $(CFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)