#include "pc.h"

int connect(int argc, char **argv) {
	if (argc < 2) {
		printf("Usage: %s <dev>\n", argv[0]);
		exit(0);
	}
	int fd = open_port(argv[1]);
	termios_init(fd);
	return fd;
}

void disconnect(int fd) {
	close(fd);
}
