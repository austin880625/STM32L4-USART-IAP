C_FILES		:= $(shell find src/ -name '*.c')
OBJ_FILES	:= $(C_FILES:%.c=%.o)

all: pc.out

pc.out: main.c $(OBJ_FILES)
	gcc main.c $(OBJ_FILES) -Iinclude -o $@

%.o: %.c
	gcc -c $^ -Iinclude -o $@

clean:
	rm -f $(OBJ_FILES) pc.out

test:
	make && ./pc.out /dev/ttyUSB0 && make clean
