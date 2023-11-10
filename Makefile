CC=clang
DFLAGS=
FLAGS=-Wall -Wextra -g
LIBS=-I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -lSDL2 -lSDL2_ttf

all: clean build

build: pingpong

pingpong: pingpong.c queue.c
	${CC} ${FLAGS} ${DFLAGS} ${LIBS} pingpong.c queue.c -o pingpong

clean:
	rm -rf pingpong.dSYM pingpong

.PHONY: build clean
