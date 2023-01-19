#ifndef __TCP_HELPER_H
#define __TCP_HELPER_H

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define ERR (-1)
#define OK  (0)
#define LISTEN_Q_LEN (5)
#define BUFFERSIZE (100)

int h_listen(unsigned short);
int h_accept(int, int *);

#endif

