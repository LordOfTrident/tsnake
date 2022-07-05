SRC  = $(wildcard src/*.c)
DEPS = $(wildcard src/*.h)
OBJ  = $(addsuffix .o,$(subst src/,bin/,$(basename $(SRC))))

OUT = ./bin/app

CC = gcc
C_VER = c99
C_FLAGS = -O3 -std=$(C_VER) -Wall -Wextra -Werror \
          -pedantic -Wno-deprecated-declarations
C_LIBS = -lncurses -ltinfo

compile: ./bin $(OBJ) $(SRC)
	$(CC) $(C_FLAGS) -o $(OUT) $(OBJ) $(C_LIBS)

bin/%.o: src/%.c $(DEPS)
	$(CC) -c $< $(C_FLAGS) -o $@

./bin:
	mkdir -p bin

clean:
	rm -r ./bin/*

all:
	@echo compile, clean
