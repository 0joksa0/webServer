//
// Created by aleksandar on 12.12.23..
//

#ifndef SERVER_H
#define SERVER_H

#endif //SERVER_H

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>


#define PORT 9090
#define BUFFER_SIZE 104857600
