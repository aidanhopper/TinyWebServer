#ifndef HEADERS_H
#define HEADERS_H

#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define REQUEST_LENGTH 1024
#define WEB_ROOT "testsite"

typedef struct request_headers_t {
  char connection[REQUEST_LENGTH];
  char host[REQUEST_LENGTH];
} request_headers;

typedef struct response_headers_t {

} response_headers;

typedef struct request_t {
  char method[REQUEST_LENGTH];
  char request_target[REQUEST_LENGTH];
  char protocol[REQUEST_LENGTH];
  request_headers headers;
  char web_root[REQUEST_LENGTH];
} request;

#endif
