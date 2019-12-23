#include "usart.h"

int open_port(const char *src) {
	int fd = open(src, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		printf("open_port: Unable to open %s - ", src);
		exit(-1);
	}
	return fd;
}

void termios_init(int fd) {
	struct termios options;
	tcgetattr(fd, &options);

	// baud rate
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);

	options.c_cflag |= (CLOCAL | CREAD);
	
	// parity checking: 8N1
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	tcsetattr(fd, TCSANOW, &options);

	fcntl(fd, F_SETFL, 0);
}
