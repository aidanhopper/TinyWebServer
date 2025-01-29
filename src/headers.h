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
#define SERVER_MEMBER_LENGTH 256
#define RESPONSE_MEMBER_LENGTH 256
#define HEADER_MEMBER_LENGTH 256

typedef enum filetype_t {
  HTML = 0,
  CSS,
  JS,
  PNG,
  PDF,
  NOT_SUPPORTED,
} filetype_t;

typedef enum method_t {
  GET = 0,
  POST,
  PUT,
  DELETE,
  PATCH,
  CONNECT,
  TRACE,
  OPTIONS,
  HEAD,
} method_t;

static const char *FILETYPE_MAPPING[] = {"text/html",       "text/css",
                                         "text/js",         "image/png",
                                         "application/pdf", "text/plain"};

typedef enum route_type_t { DIRECTORY, DEFINED } route_type_t;

typedef struct headers_t {
  char connection[HEADER_MEMBER_LENGTH];
  char host[HEADER_MEMBER_LENGTH];
  char user_agent[HEADER_MEMBER_LENGTH];
  char accept[HEADER_MEMBER_LENGTH];
  char content_length[HEADER_MEMBER_LENGTH];
  char content_type[HEADER_MEMBER_LENGTH];
} headers_t;

typedef struct request_t {
  method_t method;
  char request_target[REQUEST_MEMBER_LENGTH];
  char protocol[REQUEST_MEMBER_LENGTH];
  char web_root[REQUEST_MEMBER_LENGTH];
  char path[REQUEST_MEMBER_LENGTH];
  char extension[REQUEST_MEMBER_LENGTH];
  headers_t headers;
} request_t;

typedef struct response_t {
  uint16_t status_code;
  char protocol[RESPONSE_MEMBER_LENGTH];
  uint8_t *content;
  uint64_t content_length;
  headers_t headers;
} response_t;

typedef struct route_t {
  char path[SERVER_MEMBER_LENGTH];
  method_t method;
} route_t;

typedef struct routes_t {
  route_t **get_routes;
  uint64_t get_routes_len;
} routes_t;

typedef struct server_t {
  char web_root[SERVER_MEMBER_LENGTH];
  char not_found_path[SERVER_MEMBER_LENGTH];
} server_t;

#endif
