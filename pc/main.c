#include "pc.h"

#define MAX_LEN 65536
#define MAX_PACKET 64
int load_source(char *filename, uint8_t *buf, uint32_t *len) {
	FILE *src;
	if (!(src = fopen(filename, "rb")))
		return 1;
	memset(buf, 0, sizeof(uint8_t) * MAX_LEN);
	uint8_t *ptr = buf; 
	*len = 0;
	while (1) {
		size_t sz = fread(ptr, sizeof(uint8_t), 1024, src);
		*len += sz;
		if (sz != 1024)
			break;
		ptr += 1024;
	}
	while (*len % 4) *len++;
	fclose(src);
	return 0;
}
void print(uint32_t target) {
	printf("%u (%u, %u, %u, %u)\n", target, (target >> 24) & 0xFF, (target >> 16) & 0xFF, (target >> 8) & 0xFF, target & 0xFF);
}
int cnt = 0;
int send_packet(int fd, uint8_t *buf, uint32_t sz) {
	printf("send packet %d(%d bytes)\n", cnt, sz);
	uint8_t *ptr = buf;
	uint32_t local_checksum = sz;
	while (ptr - buf < sz) {
		local_checksum += *((uint32_t*)ptr);
		ptr += 4;
	}
	printf("local  checksum: ");
	print(local_checksum);
	send_int(fd, local_checksum);
	send_int(fd, sz);
	ptr = buf;
	while (ptr - buf < sz)
		send_byte(fd, *ptr), ptr++;
	uint32_t return_checksum = receive_int(fd);
	printf("return checksum: ");
	print(return_checksum);
	uint32_t remote_checksum = receive_int(fd);
	printf("remote checksum: ");
	print(remote_checksum);
	uint32_t return_code = receive_int(fd);
	printf("get return code: %u\n", return_code);
	return return_code;
}
void send_source(int fd, char *filename) {
	static uint8_t buf[MAX_LEN];
	uint32_t len = 0;
	if (load_source(filename, buf, &len)) {
		printf("Open source file error!\n");
		exit(1);
	}
	uint8_t *ptr = buf;
	while (ptr - buf < len) {
		while (send_packet(fd, ptr, MAX_PACKET)) {
			printf("Corrupted, trying to resend\n\n\n");
		}
		cnt++;
		printf("Code is correct\n\n\n");
		ptr += MAX_PACKET;
	}
	while (send_packet(fd, ptr, 0))
		printf("Last Corrupted, trying to resend\n\n\n");
	printf("Last Code is correct\n\n\n");
}

int main(int argc, char **argv) {
	int fd = connect(argc, argv);
	send_source(fd, argv[1]);
	disconnect(fd);
}
