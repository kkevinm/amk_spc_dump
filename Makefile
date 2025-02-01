CC=gcc
FLAGS=-Wall -Wextra -pedantic -O3
INC=
LD=
SRC=src/amk_dump.c src/main.c src/rom.c src/utils.c
OUT=build
EXE=amk_spc_dump.exe

.PHONY: clean

all:
	$(MAKE) clean
	$(MAKE) build

clean:
	rm -rf $(OUT)

build:
	mkdir -p $(OUT)
	$(CC) $(SRC) -o $(OUT)/$(EXE) $(FLAGS) $(INC) $(LD)
