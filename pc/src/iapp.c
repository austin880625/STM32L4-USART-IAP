#include "iapp.h"
#include "io.h"

#define MAX_PACKET 64
#define MAX_LEN 65536

void dump_iapp(struct iapp_header_t *send_iapp_header, uint8_t *send_iapp_payload, size_t send_iapp_payload_size) {
	size_t buf_size = sizeof(struct iapp_header_t) + send_iapp_payload_size;
	uint8_t *buf = concat((uint8_t*)send_iapp_header, sizeof(struct iapp_header_t), send_iapp_payload, send_iapp_payload_size);
	dump("iapp", buf, buf_size);
	free(buf);
}

size_t iapp(int fd, struct iapp_header_t *send_iapp_header, uint8_t *send_iapp_payload, size_t send_iapp_payload_size, uint8_t *recv_packet_payload) {
	size_t send_packet_payload_size = sizeof(struct iapp_header_t) + send_iapp_payload_size;
	while (send_packet_payload_size % MAX_PACKET)
		send_packet_payload_size++;
	dump_iapp(send_iapp_header, send_iapp_payload, send_iapp_payload_size);
	uint8_t *send_packet_payload = (uint8_t*)malloc(send_packet_payload_size);
	memset(send_packet_payload, 0, send_packet_payload_size);
	memcpy(send_packet_payload, (uint8_t*)send_iapp_header, sizeof(struct iapp_header_t));
	memcpy(send_packet_payload + sizeof(struct iapp_header_t), send_iapp_payload, send_iapp_payload_size);
	int packet_stamp = 0;
	while (1) {
		uint8_t *send_packet_payload_ptr = send_packet_payload;
		struct packet_header_t recv_packet_header;
		while (send_packet_payload_ptr - send_packet_payload < send_iapp_payload_size) {
			struct packet_header_t send_packet_header = {
				.checksum = cal_checksum(send_packet_payload_ptr, MAX_PACKET),
				.size = MAX_PACKET,
				.control = (send_packet_payload_ptr + MAX_PACKET - send_packet_payload == send_packet_payload_size ? 0x2 : 0x0)
			};
			printf("send packet #%d\n", packet_stamp);
			send_packet(fd, &send_packet_header, send_packet_payload_ptr);

			printf("recv packet #%d\n", packet_stamp);
			recv_packet(fd, &recv_packet_header, recv_packet_payload);

			if ((recv_packet_header.control & 0x1))
				continue;
			send_packet_payload_ptr += MAX_PACKET;
			packet_stamp++;
		}

		if (recv_packet_header.checksum == cal_checksum(recv_packet_payload, recv_packet_header.size)) {
			free(send_packet_payload);
			return recv_packet_header.size;
		}
	}
}
