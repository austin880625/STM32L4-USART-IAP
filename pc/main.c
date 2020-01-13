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
void send_reset(int fd) {
    struct iapp_header_t iapp_header = {
        .command = RESET
    };
    uint8_t recv_payload[MAX_SOURCE];
    uint8_t buf[MAX_SOURCE];
    memset(buf, 0, sizeof(buf));
    size_t recv_payload_size = iapp(fd, &iapp_header, buf, 0, recv_payload);
    dump("recv payload", recv_payload, 0, recv_payload_size);
}
void send_run(int fd) {
    struct iapp_header_t iapp_header = {
        .command = RUN
    };
    uint8_t recv_payload[MAX_SOURCE];
    uint8_t buf[MAX_SOURCE];
    memset(buf, 0, sizeof(buf));
    size_t recv_payload_size = iapp(fd, &iapp_header, buf, 0, recv_payload);
    dump("recv payload", recv_payload, 0, recv_payload_size);
    if (!strncmp(recv_payload, "NO", 2))
        puts("The code on the mcu is corrupt!");
    else
        puts("The mcu is running!");
}
void send_upload(int fd, char *filename) {
	uint8_t source[MAX_SOURCE];
	size_t source_size;
	if (!(source_size = load_source(filename, source))) {
		printf("Open source file error!\n");
		return ;
	}
	struct iapp_header_t iapp_header = {
		.command = UPLOAD
	};
	uint8_t recv_payload[MAX_SOURCE];
	size_t recv_payload_size = iapp(fd, &iapp_header, source, source_size, recv_payload);
	dump("recv payload", recv_payload, 0, recv_payload_size);
}

void send_get(int fd) {
	struct iapp_header_t iapp_header = {
		.command = GET
	};
	uint8_t iapp_payload[] = "";
	size_t iapp_payload_size = 0;
	uint8_t recv_payload[512];
	size_t recv_payload_size = iapp(fd, &iapp_header, iapp_payload, iapp_payload_size, recv_payload);
	dump("recv payload", recv_payload, 0, recv_payload_size);
}
void send_set(int fd, uint32_t value) {
	struct iapp_header_t iapp_header = {
		.command = SET
	};
	uint32_t iapp_payload = value;
	size_t iapp_payload_size = sizeof(iapp_payload);
	uint8_t recv_payload[512];
	size_t recv_payload_size = iapp(fd, &iapp_header, (uint8_t*)(&iapp_payload), iapp_payload_size, recv_payload);
	dump("recv payload", recv_payload, 0, recv_payload_size);
}
void help() {
    printf("\thelp\t\t\tPrint this message\n");
    printf("\treset\t\t\tReset mcu\n");
    printf("\tupload <filename>\tUpload binary file to mcu\n");
    printf("\trun\t\t\tStart the code on the mcu\n");
    printf("\tget\t\t\tGet code information on the mcu\n");
    printf("\tset\t\t\tSet execution options on reset\n");
    printf("\t\t\t\t0: run the program after reset immediately\n");
    printf("\t\t\t\t1: run the program after the \"run\" command\n");
}
int main(int argc, char **argv) {
	int fd = connect(argc, argv);
	while (1) {
		printf("> ");
		char op[128];
		fgets(op, 128, stdin);
		char *ptr = strtok(op, " \n");
		if (!ptr)
			continue;
        if (!strncmp(ptr, "help", 4)) {
            help();
        } else if (!strncmp(ptr, "upload", 6)) {
			ptr = strtok(NULL, " \n");
			send_upload(fd, ptr);
        } else if (!strncmp(ptr, "reset", 5)) {
            send_reset(fd);
        } else if (!strncmp(ptr, "run", 3)) {
            send_run(fd);
        } else if (!strncmp(ptr, "get", 3)) {
            send_get(fd);
        } else if (!strncmp(ptr, "set", 3)) {
            ptr = strtok(NULL, " \n");
            uint32_t value;
            sscanf(ptr, "%u", &value);
            send_set(fd, value);
		} else if (!strncmp(ptr, "q", 1)) {
			exit(0);
		} else goto end;
		continue;
end:;
		printf("command not found\n");
	}
	disconnect(fd);
}
