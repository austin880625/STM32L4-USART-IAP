#include "io.h"

uint8_t base16m_encode(uint8_t x) {
	return x + 'A';
}

uint8_t base16m_decode(uint8_t x) {
	return x - 'A';
}

void recv_byte(int fd, uint8_t *buf) {
	uint8_t bl, br;
	read(fd, &bl, sizeof(uint8_t));
	read(fd, &br, sizeof(uint8_t));
	bl = base16m_decode(bl);
	br = base16m_decode(br);
	*buf = (bl << 4) | (br & 0xF);
}

void recv_nbyte(int fd, uint8_t *buf, int size) {
	for (int i = 0 ; i < size ; i++)
		recv_byte(fd, buf + i);
}

void recv_packet_header(int fd, struct packet_header_t *packet_header) {
	recv_nbyte(fd, (uint8_t*)packet_header, sizeof(struct packet_header_t));
}

void recv_packet(int fd, struct packet_header_t *packet_header, uint8_t *payload) {
	recv_packet_header(fd, packet_header);
	recv_nbyte(fd, payload, packet_header->size);
	size_t buf_size = sizeof(struct packet_header_t) + packet_header->size;
	uint8_t *buf = concat((uint8_t*)packet_header, sizeof(struct packet_header_t), payload, packet_header->size);
	dump("packet", buf, buf_size);
	free(buf);
}

void send_byte(int fd, uint8_t *buf) {
	uint8_t bl, br;
	br = base16m_encode(*buf & 0xF);
	bl = base16m_encode((*buf >> 4) & 0xF);
	write(fd, &bl, sizeof(uint8_t));
	write(fd, &br, sizeof(uint8_t));
}

void send_nbyte(int fd, uint8_t *buf, int size) {
	for (int i = 0 ; i < size ; i++)
		send_byte(fd, buf + i);
}

void send_packet_header(int fd, struct packet_header_t *packet_header) {
	send_nbyte(fd, (uint8_t*)packet_header, sizeof(struct packet_header_t));
}

void send_packet(int fd, struct packet_header_t *packet_header, uint8_t *payload) {
	send_packet_header(fd, packet_header);
	send_nbyte(fd, payload, packet_header->size);
	size_t buf_size = sizeof(struct packet_header_t) + packet_header->size;
	uint8_t *buf = concat((uint8_t*)packet_header, sizeof(struct packet_header_t), payload, packet_header->size);
	dump("packet", buf, buf_size);
	free(buf);
}

uint32_t cal_checksum(uint8_t *buf, size_t size) {
	uint32_t checksum = size;
	uint8_t *ptr = buf;
	while (ptr - buf < size) {
		checksum += *((uint32_t*)ptr);
		ptr += 4;
	}
	return checksum;
}

void dump(const char *msg, uint8_t *buf, size_t size) {
	puts(msg);
	const int N = 16;
	for (int i = 0, j ; i < size ; i += N) {
		printf("%08x: ", i);
		for (j = 0 ; j < N && i + j < size ; j++) {
			printf("%02x", buf[i + j]);
			if (j % 2)
				printf(" ");
		}
		for (; j < N ; j++) {
			printf("  ");
			if (j % 2)
				printf(" ");
		}
		printf(" ");
		for (j = 0 ; j < N && i + j < size ; j++) {
			if (isprint(buf[i + j]))
				printf("%c", buf[i + j]);
			else
				printf(".");
		}
		for (; j < N ; j++) {
			printf("  ");
			if (j % 2)
				printf(" ");
		}
		puts("");
	}
	puts("");
}

uint8_t* concat(uint8_t *a, size_t a_size, uint8_t *b, size_t b_size) {
	uint8_t *buf = (uint8_t*)malloc(a_size + b_size);
	memcpy(buf         , a, a_size);
	memcpy(buf + a_size, b, b_size);
	return buf;
}
