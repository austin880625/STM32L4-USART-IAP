#ifndef __USART_H
#define __USART_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <termios.h>
#include <fcntl.h>

int open_port(const char *src);
void termios_init(int fd);

#endif
