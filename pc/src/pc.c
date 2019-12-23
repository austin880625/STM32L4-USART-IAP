#include "pc.h"

int connect(int argc, char **argv) {
	if (argc != 3) {
		printf("Usage: %s <filename> <dev>\n", argv[0]);
		exit(0);
	}
	int fd = open_port(argv[2]);
	termios_init(fd);
	return fd;
}

void disconnect(int fd) {
	close(fd);
}
