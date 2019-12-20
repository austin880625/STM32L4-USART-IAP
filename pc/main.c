#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

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
	uint32_t checksum = 0;
	static uint8_t buf[MAX_LEN];
	uint8_t *ptr = buf;
	memset(buf, 0, sizeof(buf));
	int len = 0;
	while (1) {
		size_t sz = fread(ptr, sizeof(uint8_t), 1024, src);
		len += sz;
		if (sz != 1024)
			break;
		ptr += 1024;
	}
	fclose(src);
	while (len % 4) len++;
	ptr = buf;
	while (ptr - buf < len) {
		uint32_t tmp = 0;
		for (int i = 0 ; i < 4 ; i++) {
			tmp |= ((uint32_t)ptr[i] << ((3-i) * 8));
		}
		checksum += tmp;
		ptr += 4;
	}
	for(int i=0; i<4; i++) {
		uint8_t part = checksum >> ((3-i)*8);
		write(fd, &part, sizeof(uint8_t));
	}
	printf("checksum: %u %u %u %u %u\n", checksum, (checksum & 0xFF), ((checksum >> 8) & 0xFF), ((checksum >> 16) & 0xFF), ((checksum >> 24) & 0xFF));
	for(int i=0; i<len; i++) {
		//int kkk;
		//scanf("%d", &kkk);
		write(fd, &buf[i], sizeof(uint8_t));
		//printf("a byte is sent\n");
	}
	puts("write done");
	uint32_t ret_checksum = 0;
	for(int i=0; i<1; i++) {
		uint8_t part = 0;
		read(fd, &part, sizeof(uint8_t));
		ret_checksum |= (uint32_t)part << (8 * i);
	}
	printf("get return code: %u\n", ret_checksum);
	if(ret_checksum == 0) {
		printf("Code is correct\n");
	} else {
		printf("Corrupted, trying to resend\n");
	}
	return ret_checksum;
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
