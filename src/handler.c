#include "headers.h"

#include "methods.c"

bool parse(request_t *req, uint8_t buffer[REQUEST_LENGTH]);
bool parse_start_line(request_t *req, char buffer[REQUEST_LENGTH]);
uint8_t *handle_response(server_t *server, uint8_t buffer[REQUEST_LENGTH],
                         uint64_t *response_length);
void init_request(request_t *req);

// Thinking of adding the connecting IP to the arguments
void handle_connection(server_t *server, int32_t connection) {
  uint8_t buffer[REQUEST_LENGTH];
  uint32_t bytes;

  while (true) {
    memset(buffer, 0, REQUEST_LENGTH);

    bytes = recv(connection, buffer, REQUEST_LENGTH - 1, 0);
    if (!bytes)
      return;

    printf("%s\n", buffer);

    uint64_t response_length = 0;
    uint8_t *response = handle_response(server, buffer, &response_length);

    if (response != NULL) {
      printf("%s\n", response);
      send(connection, response, response_length, 0);
      free(response);
    }
  }
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

uint8_t *handle_response(server_t *server, uint8_t buffer[REQUEST_LENGTH],
                         uint64_t *response_length) {
  request_t req;
  init_request(&req);

  // Populate request structure
  if (!parse(&req, buffer))
    return NULL;

  strcpy(req.web_root, server->web_root);
  get_path(req.path, req.request_target, req.web_root);
  get_file_extension(req.extension, req.request_target);

  // Do some validation
  if (strcmp(req.protocol, "HTTP/1.1") != 0)
    return NULL;

  // Handle building the response on the request method
  uint8_t *response = NULL;

  if (strcmp(req.method, "GET") == 0)
    response = get(&req, response_length);
  else if (strcmp(req.method, "POST") == 0)
    response = not_implemented(&req, response_length);
  else if (strcmp(req.method, "DELETE") == 0)
    response = not_implemented(&req, response_length);
  else if (strcmp(req.method, "PATCH") == 0)
    response = not_implemented(&req, response_length);
  else if (strcmp(req.method, "CONNECT") == 0)
    response = not_implemented(&req, response_length);
  else if (strcmp(req.method, "TRACE") == 0)
    response = not_implemented(&req, response_length);
  else if (strcmp(req.method, "OPTIONS") == 0)
    response = not_implemented(&req, response_length);
  else if (strcmp(req.method, "TRACE") == 0)
    response = not_implemented(&req, response_length);
  else if (strcmp(req.method, "HEAD") == 0)
    response = not_implemented(&req, response_length);
  return response;
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
  strncat(req->method, token, REQUEST_MEMBER_LENGTH);

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

void init_request(request_t *req) {
  req->protocol[0] = '\0';
  req->method[0] = '\0';
  req->request_target[0] = '\0';
  req->headers.host[0] = '\0';
  req->headers.connection[0] = '\0';
}
