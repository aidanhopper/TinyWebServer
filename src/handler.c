#include "headers.h"

#include "builder.c"

const char endl[] = "\r\n";

bool parse(request_t *req, uint8_t buffer[REQUEST_LENGTH]);
bool parse_start_line(request_t *req, char buffer[REQUEST_LENGTH]);
void init_request(request_t *req);

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

void set_content_to_file(response_t *res, const char *path) {
  res->content_length = 0;
  res->content = grab_file(path, &res->content_length);

  char extension[100];
  get_file_extension(extension, path);

  char content_length_str[100];
  snprintf(content_length_str, 100, "%llu", res->content_length);

  set_header(res, "Content-Type", FILETYPE_MAPPING[get_filetype(extension)]);
  set_header(res, "Content-Length", content_length_str);
}

void set_content(response_t *res, request_t *req, server_t *server) {
  if (file_exists(req->path)) {
    set_content_to_file(res, req->path);
    set_status_code(res, 200);
  } else {
    set_content_to_file(res, server->not_found_path);
    set_status_code(res, 404);
  }
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

void init_response_t(response_t *res) {
  memset(&res->headers, 0, sizeof(headers_t));
}

void get_path(char *path, char *request_target, char *root) {
  int32_t dot_index = -1;
  for (int i = 0; i < strlen(request_target); i++)
    if (request_target[i] == '.') {
      dot_index = i;
      break;
    }

  if (strcmp(request_target, "/") == 0) {
    sprintf(path, "%s/index.html", root);
  } else if (dot_index == -1) {
    sprintf(path, "%s/%s.html", root, request_target);
  } else {
    sprintf(path, "%s/%s", root, request_target);
  }
}

bool parse_request(request_t *req, server_t *server, uint8_t *buffer) {
  req->protocol[0] = '\0';
  req->request_target[0] = '\0';
  req->headers.host[0] = '\0';
  req->headers.connection[0] = '\0';

  // Populate request structure
  if (!parse(req, buffer))
    return false;

  strcpy(req->web_root, server->web_root);
  get_path(req->path, req->request_target, req->web_root);
  get_file_extension(req->extension, req->request_target);

  return true;
}

void create_response(response_t *res, request_t *req, server_t *server) {
  init_response_t(res);

  switch (req->method) {
  case GET:
    set_protocol(res, "HTTP/1.1");
    set_status_code(res, 200);
    set_content(res, req, server);
    set_header(res, "Connection", "keep-alive");
    break;
  case POST:
  case PUT:
  case DELETE:
  case PATCH:
  case CONNECT:
  case TRACE:
  case OPTIONS:
  case HEAD:
    set_protocol(res, "HTTP/1.1");
    set_status_code(res, 501);
    set_header(res, "Content-Length", "0");
    break;
  }
}

uint8_t *response_to_buf(response_t *res, uint64_t *response_length) {
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

void free_response(response_t *res) {
  if (res->content_length > 0)
    free(res->content);
  res->content = NULL;
}

// Thinking of adding the connecting IP to the arguments
void handle_connection(server_t *server, int32_t connection) {
  uint8_t buffer[REQUEST_LENGTH];
  uint32_t bytes;

  while (true) {
    memset(buffer, 0, REQUEST_LENGTH);

    bytes = recv(connection, buffer, REQUEST_LENGTH - 1, 0);
    if (!bytes)
      return;

    request_t req;
    if (!parse_request(&req, server, buffer)) // invalid request
      return;

    printf("%s\n", buffer);

    response_t res;
    create_response(&res, &req, server);

    uint64_t response_buf_length = 0;
    uint8_t *response_buf = response_to_buf(&res, &response_buf_length);

    free_response(&res);

    if (response_buf != NULL) {
      printf("%s\n", response_buf);
      send(connection, response_buf, response_buf_length, 0);
      free(response_buf);
    }
  }
}

bool parse_header(request_t *req, char buffer[REQUEST_LENGTH]) {
  char *rest = buffer;
  const char *del = " ";

  char *header = strtok_r(rest, del, &rest);
  if (header[strnlen(header, REQUEST_LENGTH) - 1] != ':')
    return false;

  // char *value = header + strnlen(header, REQUEST_LENGTH) + 1;
  header[strnlen(header, REQUEST_LENGTH) - 1] = '\0';

  // printf("Header is %s\nValue is %s\n\n", header, value);

  return true;
}

bool parse(request_t *req, uint8_t buffer[REQUEST_LENGTH]) {
  char *rest_of_buffer = (char *)buffer;
  char *line;
  const char del[] = "\r\n";
  uint32_t line_number = 0;
  bool in_headers = true;

  for (line = strtok_r(rest_of_buffer, del, &rest_of_buffer); line != NULL;
       line = strtok_r(NULL, del, &rest_of_buffer)) {

    // Parses the start-line
    if (line_number == 0 && !parse_start_line(req, line))
      return false;
    else if (in_headers)
      parse_header(req, line);

    // End of headers if true
    if (strnlen(line, REQUEST_LENGTH) == 0) {
      in_headers = false;
      // printf("NO MORE HEADERS\n");
    } else if (!in_headers) {
      // parse the body of the request
    }

    line_number++;
  }

  return true;
}

bool parse_start_line(request_t *req, char line[REQUEST_LENGTH]) {
  char *rest = line;
  char *token;
  const char del[] = " ";

  token = strtok_r(rest, del, &rest);
  if (token == NULL)
    return false;

  if (strcmp(token, "GET") == 0)
    req->method = GET;
  else if (strcmp(token, "PUT") == 0)
    req->method = PUT;
  else if (strcmp(token, "POST") == 0)
    req->method = POST;
  else if (strcmp(token, "DELETE") == 0)
    req->method = DELETE;
  else if (strcmp(token, "PATCH") == 0)
    req->method = PATCH;
  else if (strcmp(token, "CONNECT") == 0)
    req->method = CONNECT;
  else if (strcmp(token, "TRACE") == 0)
    req->method = TRACE;
  else if (strcmp(token, "OPTIONS") == 0)
    req->method = OPTIONS;
  else if (strcmp(token, "HEAD") == 0)
    req->method = HEAD;
  else
    return false;

  token = strtok_r(NULL, del, &rest);
  if (token == NULL)
    return false;
  strncat(req->request_target, token, REQUEST_MEMBER_LENGTH);

  token = strtok_r(NULL, del, &rest);
  if (token == NULL)
    return false;
  strncat(req->protocol, token, REQUEST_MEMBER_LENGTH);

  return true;
}
