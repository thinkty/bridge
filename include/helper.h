#ifndef __TCP_HELPER_H
#define __TCP_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>

#define LISTEN_Q_LEN    (5)

int h_listen(unsigned short);
int h_accept(int);
int h_connect(const char *, const char *);

#endif

