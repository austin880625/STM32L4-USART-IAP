#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

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
#define MAX_LEN 65536
int send_source(int fd, char *filename) {
	FILE *src;
	if (!(src = fopen(filename, "rb"))) {
		printf("Open source file error!\n");
		exit(1);
	}
	unsigned int checksum = 0;
	static char buf[MAX_LEN];
	char *ptr = buf;
	memset(buf, 0, sizeof(buf));
	int len = 0;
	while (1) {
		size_t sz = fread(ptr, sizeof(char), 1024, src);
		len += sz;
		if (sz != 1024)
			break;
		ptr += 1024;
	}
	fclose(src);
	while (len % 4) len++;
	ptr = buf;
	while (ptr - buf < len) {
		unsigned int tmp = 0;
		for (int i = 0 ; i < 4 ; i++) {
			tmp <<= 8;
			tmp |= ptr[i];
		}
		checksum += tmp;
		ptr += 4;
	}
	write(fd, &checksum, sizeof(checksum));
	printf("checksum: %d\n", checksum);
	write(fd, buf, sizeof(char) * len);
	puts("write done");
	read(fd, buf, sizeof(char));
	printf("get return code: %d\n", buf[0]);
	return buf[0];
}
int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		exit(0);
	}
	int fd = open_port("/dev/ttyUSB0");
	termios_init(fd);
	while (send_source(fd, argv[1]));
	close(fd);
}
