#include "io.h"

uint8_t receive_byte(int fd) {
	uint8_t ret = 0;
	printf("wait...\n");
	read(fd, &ret, sizeof(uint8_t));
	printf("get byte: %u\n", ret);
	return ret;
}
uint32_t receive_int(int fd) {
	uint32_t ret_int = 0;
	for (int i = 0 ; i < 4 ; i++) {
		uint8_t part = receive_byte(fd);
		ret_int |= (uint32_t)part << (8 * i);
	}
	return ret_int;
}
void send_byte(int fd, uint8_t target) {
	write(fd, &target, sizeof(uint8_t));
}
void send_int(int fd, uint32_t target) {
	for(int i = 0 ; i < 4 ; i++)
		send_byte(fd, target >> ((3 - i) * 8));
}

