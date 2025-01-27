#include "builder.c"
#include "headers.h"

const char endl[] = "\r\n";

uint8_t *init_response(int capacity) {
  uint8_t *response = calloc(sizeof(uint8_t), capacity);
  return response;
}

bool file_exists(char *path) {
  FILE *file;
  if ((file = fopen(path, "r"))) {
    fclose(file);
    return true;
  }
  return false;
}

uint8_t *normal_response(request_t *req, uint64_t *response_length) {
  uint64_t content_length = 0;
  uint8_t *content = grab_file(req->path, &content_length);

  char content_length_header[100] = "Content-Length: 0\r\n";
  if (content != NULL)
    sprintf(content_length_header, "Content-Length: %llu\r\n", content_length);

  char content_type_header[100];
  sprintf(content_type_header, "Content-Type: %s\r\n",
          FILETYPE_MAPPING[req->filetype]);

  char start_line[] = "HTTP/1.1 200 OK\r\n";

  uint64_t capacity = 1024;
  uint8_t *response = init_response(capacity);

  add(&response, &capacity, response_length, (uint8_t *)start_line,
      strlen(start_line));
  add(&response, &capacity, response_length, (uint8_t *)content_length_header,
      strlen(content_length_header));
  add(&response, &capacity, response_length, (uint8_t *)content_type_header,
      strlen(content_type_header));
  add(&response, &capacity, response_length, (uint8_t *)endl, strlen(endl));
  add(&response, &capacity, response_length, content, content_length);

  if (content != NULL)
    free(content);

  return response;
}

uint8_t *not_found(request_t *req, uint64_t *response_length) {
  uint64_t capacity = 100;
  uint8_t *response = init_response(capacity);

  const char start_line[] = "HTTP/1.1 404 Not Found\r\n";
  const char content_type_header[] = "Content-Type: text/html\r\n";

  const char content[] = "<h1>404 file not found :(</h1>";

  char content_length_header[100];
  snprintf(content_length_header, 100, "Content-Length: %lu\r\n",
           strlen(content));

  add(&response, &capacity, response_length, (uint8_t *)start_line,
      strlen(start_line));
  add(&response, &capacity, response_length, (uint8_t *)content_length_header,
      strlen(content_length_header));
  add(&response, &capacity, response_length, (uint8_t *)content_type_header,
      strlen(content_type_header));
  add(&response, &capacity, response_length, (uint8_t *)endl, strlen(endl));
  add(&response, &capacity, response_length, (uint8_t *)content,
      strlen(content));

  return response;
}

uint8_t *not_implemented(request_t *req) {
  uint64_t capacity = 100;
  uint8_t *response = init_response(capacity);
  uint64_t response_length = 0;

  const char start_line[] = "HTTP/1.1 501 Not Implemented\r\n";

  add(&response, &capacity, &response_length, (uint8_t *)start_line,
      strlen(start_line));
  add(&response, &capacity, &response_length, (uint8_t *)endl, strlen(endl));

  return response;
}

uint8_t *get(request_t *req, uint64_t *response_length) {
  if (!file_exists(req->path))
    return not_found(req, response_length);
  return normal_response(req, response_length);
}
