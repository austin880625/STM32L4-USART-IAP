#ifndef __PC_H
#define __PC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "usart.h"
#include "io.h"

int connect(int argc, char **argv);
void disconnect(int fd);

#endif
