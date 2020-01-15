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
	dump(stderr, "packet", buf, sizeof(struct packet_header_t), packet_header->size);
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
	dump(stderr, "packet", buf, sizeof(struct packet_header_t), packet_header->size);
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

void dump(FILE *fp, const char *msg, uint8_t *buf, size_t h_size, size_t b_size) {
	size_t size = h_size + b_size;
	#define HEAD_COLOR "\x1b[48;5;166m"
	#define BODY_COLOR "\x1b[48;5;32m"
	#define RESET	   "\e[0m"
	fprintf(fp, msg);
	fprintf(fp, "\n");
	const int N = 16;
	for (int i = 0, j ; i < size ; i += N) {
		fprintf(fp, "%08x: ", i);
		fprintf(fp, "%s", i < h_size ? HEAD_COLOR : BODY_COLOR);
		for (j = 0 ; j < N && i + j < size ; j++) {
			fprintf(fp, "%02x", buf[i + j]);
			if (i + j + 1 == h_size)
				fprintf(fp, RESET);
			if (j % 2 && j + 1 != N && i + j + 1 != size)
				fprintf(fp, " ");
			if (i + j + 1 == h_size)
				fprintf(fp, BODY_COLOR);
		}
		fprintf(fp, RESET);
		for (; j < N ; j++) {
			if (j % 2 == 0 && j + 1 != N)
				fprintf(fp, " ");
			fprintf(fp, "  ");
		}
		fprintf(fp, "  ");
		fprintf(fp, "%s", i < h_size ? HEAD_COLOR : BODY_COLOR);
		for (j = 0 ; j < N && i + j < size ; j++) {
			fprintf(fp, "%c", isprint(buf[i + j]) ? buf[i + j] : '.');
			if (i + j + 1 == h_size) {
				fprintf(fp, RESET);
				fprintf(fp, BODY_COLOR);
			}
		}
		fprintf(fp, RESET);
		for (; j < N ; j++) {
			fprintf(fp, "  ");
			if (j % 2)
				fprintf(fp, " ");
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");
}

uint8_t* concat(uint8_t *a, size_t a_size, uint8_t *b, size_t b_size) {
	uint8_t *buf = (uint8_t*)malloc(a_size + b_size);
	memcpy(buf         , a, a_size);
	memcpy(buf + a_size, b, b_size);
	return buf;
}
