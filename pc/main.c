#include "pc.h"
#include "iapp.h"

#define MAX_SOURCE 8192

size_t load_source(char *filename, uint8_t *source) {
	FILE *src;
	if (!(src = fopen(filename, "rb")))
		return 0;
	memset(source, 0, sizeof(uint8_t) * MAX_SOURCE);
	uint8_t *source_ptr = source; 
	size_t source_size = 0;
	while (1) {
		size_t load_size = fread(source_ptr, sizeof(uint8_t), 1024, src);
		source_size += load_size;
		if (load_size != 1024)
			break;
		source_ptr += 1024;
	}
	while (source_size % 4) source_size++;
	fclose(src);
	return source_size;
}

void upload(int fd, char *filename) {
	uint8_t source[MAX_SOURCE];
	size_t source_size;
	if (!(source_size = load_source(filename, source))) {
		printf("Open source file error!\n");
		exit(1);
	}
	dump("upload source file", source, source_size);
	struct iapp_header_t iapp_header = {
		.command = UPLOAD
	};
	uint8_t recv_payload[MAX_SOURCE];
	size_t recv_payload_size = iapp(fd, &iapp_header, source, source_size, recv_payload);
	dump("recv payload", recv_payload, recv_payload_size);
}

void get(int fd) {
	struct iapp_header_t iapp_header = {
		.command = GET
	};
	uint8_t iapp_payload[] = "Hello World!!";
	size_t iapp_payload_size = sizeof(iapp_payload);
	uint8_t recv_payload[512];
	size_t recv_payload_size = iapp(fd, &iapp_header, iapp_payload, iapp_payload_size, recv_payload);

	printf("recv_payload_size: %lu\n", recv_payload_size);
	printf("recv_payload: %s\n", (char*)recv_payload);
}

int main(int argc, char **argv) {
	int fd = connect(argc, argv);
	upload(fd, argv[1]);
	// get(fd);
	disconnect(fd);
}
