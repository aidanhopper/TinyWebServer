#include "builder.c"
#include "headers.h"

const char endl[] = "\r\n";

void get_file_extension(char *extension, const char *request_target) {
  int32_t dot_index = -1;
  for (int i = 0; i < strlen(request_target); i++)
    if (request_target[i] == '.') {
      dot_index = i;
      break;
    }

  if (dot_index != -1) {
    strcpy(extension, request_target + dot_index + 1);
  } else {
    strcpy(extension, "html");
  }
}

filetype_t get_filetype(char *extension) {
  if (strcmp(extension, "html") == 0)
    return HTML;
  else if (strcmp(extension, "css") == 0)
    return CSS;
  else if (strcmp(extension, "js") == 0)
    return JS;
  else if (strcmp(extension, "png") == 0)
    return PNG;
  else if (strcmp(extension, "pdf") == 0)
    return PDF;
  return NOT_SUPPORTED;
}

bool file_exists(const char *path) {
  FILE *file;
  if ((file = fopen(path, "r"))) {
    fclose(file);
    return true;
  }
  return false;
}

void set_header(response_t *res, const char *header, const char *value) {
  if (strcmp(header, "Content-Type") == 0)
    snprintf(res->headers.content_type, HEADER_MEMBER_LENGTH,
             "Content-Type: %s\r\n", value);
  else if (strcmp(header, "Content-Length") == 0)
    snprintf(res->headers.content_length, HEADER_MEMBER_LENGTH,
             "Content-Length: %s\r\n", value);
  else if (strcmp(header, "Connection") == 0)
    snprintf(res->headers.connection, HEADER_MEMBER_LENGTH,
             "Connection: %s\r\n", value);
}

void set_protocol(response_t *res, const char *protocol) {
  strcpy(res->protocol, protocol);
}

void set_status_code(response_t *res, const uint16_t status_code) {
  res->status_code = status_code;
}

void set_content(response_t *res, const char *path) {
  res->content_length = 0;
  res->content = grab_file(path, &res->content_length);

  char extension[100];
  get_file_extension(extension, path);

  char content_length_str[100];
  snprintf(content_length_str, 100, "%llu", res->content_length);

  set_header(res, "Content-Type", FILETYPE_MAPPING[get_filetype(extension)]);
  set_header(res, "Content-Length", content_length_str);
}

char *map_status_code(uint16_t s, char buf[RESPONSE_MEMBER_LENGTH]) {
  switch (s) {
  case 200:
    strcpy(buf, "200 OK");
    break;
  case 404:
    strcpy(buf, "404 Not Found");
    break;
  case 501:
    strcpy(buf, "501 Not Implemented");
    break;
  }

  return buf;
}

uint8_t *init_response(int capacity) {
  return calloc(sizeof(uint8_t), capacity);
}

uint8_t *construct_response(response_t *res, uint64_t *response_length) {
  uint64_t capacity = 1024;
  uint8_t *response = init_response(capacity);

  char start_line[RESPONSE_MEMBER_LENGTH];
  char status[RESPONSE_MEMBER_LENGTH];
  snprintf(start_line, RESPONSE_MEMBER_LENGTH, "%s %s\r\n", res->protocol,
           map_status_code(res->status_code, status));

  add(&response, &capacity, response_length, (uint8_t *)start_line,
      strlen(start_line));

  char *header = (char *)&res->headers;
  for (; header < (char *)&res->headers + sizeof(headers_t);
       header += HEADER_MEMBER_LENGTH)
    if (header[0] != '\0') {
      add(&response, &capacity, response_length, (uint8_t *)header,
          strlen(header));
    }

  add(&response, &capacity, response_length, (uint8_t *)endl, strlen(endl));
  add(&response, &capacity, response_length, res->content, res->content_length);

  return response;
}

void init_response_t(response_t *res) {
  memset(&res->headers, 0, sizeof(headers_t));
}

uint8_t *not_implemented(request_t *req, uint64_t *response_length) {
  response_t res;
  init_response_t(&res);

  set_protocol(&res, "HTTP/1.1");
  set_status_code(&res, 501);
  set_header(&res, "Content-Length", "0");
  set_header(&res, "Connection", "close");

  return construct_response(&res, response_length);
}

uint8_t *get(request_t *req, uint64_t *response_length) {
  response_t res;
  init_response_t(&res);

  set_protocol(&res, "HTTP/1.1");

  if (!file_exists(req->path)) {
    char error_path[100];
    sprintf(error_path, "%s/%s", req->web_root, "404.html");
    set_content(&res, error_path);
    set_status_code(&res, 404);
  } else {
    set_content(&res, req->path);
    set_status_code(&res, 200);
  }

  return construct_response(&res, response_length);
}
