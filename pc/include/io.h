#ifndef __IO_H
#define __IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

uint8_t receive_byte(int fd);
uint32_t receive_int(int fd);
void send_byte(int fd, uint8_t target);
void send_int(int fd, uint32_t target);

#endif
