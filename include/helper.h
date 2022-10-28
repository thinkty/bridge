/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   helper.h
 * Author: wang3623 inspired by abc1986
 *
 * Created on September 8, 2018, 3:49 PM
 */

#ifndef HELPER_H
#define HELPER_H
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

#define LISTEN_Q_LEN    5
#define CMD_OPEN        "OPEN"
#define CMD_READ        "READ"
#define CMD_BACK        "BACK"
#define CMD_CLOS        "CLOS"
#define INVALID_CMD     "Invalid command\n"
#define BUFF_SIZE       1500
#define CMD_SIZE        5

int h_listen(unsigned short);
int h_accept(int);
int h_connect(const char *, const char *);

#endif /* HELPER_H */

