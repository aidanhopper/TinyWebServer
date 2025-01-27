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
#define REQUEST_MEMBER_LENGTH 256
#define WEB_ROOT "testsite"

typedef enum filetype {
  HTML = 0,
  CSS,
  JS,
  PNG,
  PDF,
  NOT_SUPPORTED,
} filetype_t;

typedef struct request_headers_t {
  char connection[REQUEST_MEMBER_LENGTH];
  char host[REQUEST_MEMBER_LENGTH];
  char user_agent[REQUEST_MEMBER_LENGTH];
  char accept[REQUEST_MEMBER_LENGTH];
} request_headers;

typedef struct response_headers_t {

} response_headers;

typedef struct request {
  char method[REQUEST_MEMBER_LENGTH];
  char request_target[REQUEST_MEMBER_LENGTH];
  char protocol[REQUEST_MEMBER_LENGTH];
  request_headers headers;
  char web_root[REQUEST_MEMBER_LENGTH];
  char path[REQUEST_MEMBER_LENGTH];
  char extension[REQUEST_MEMBER_LENGTH];
  filetype_t filetype;
} request_t;

typedef struct response {
  uint8_t *response;
  uint64_t capacity;
  uint64_t length;
} response_t;

static const char *FILETYPE_MAPPING[] = {"text/html",       "text/css",
                                         "text/js",         "image/png",
                                         "application/pdf", "text/plain"};

#endif
