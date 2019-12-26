#include "io.h"

uint8_t base16m_encode(uint8_t x) {
	return x + 'A';
}

uint8_t base16m_decode(uint8_t x) {
	return x - 'A';
}

uint8_t receive_byte(int fd) {
	uint8_t ret = 0;
	uint8_t bl, br;
	read(fd, &bl, sizeof(uint8_t));
	read(fd, &br, sizeof(uint8_t));
	bl = base16m_decode(bl);
	br = base16m_decode(br);
	ret = (bl << 4) | (br & 0xF);
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
	uint8_t bl, br;
	br = base16m_encode(target & 0xF);
	bl = base16m_encode((target >> 4) & 0xF);
	write(fd, &bl, sizeof(uint8_t));
	write(fd, &br, sizeof(uint8_t));
}
void send_int(int fd, uint32_t target) {
	for(int i = 0 ; i < 4 ; i++)
		send_byte(fd, target >> ((3 - i) * 8));
}

