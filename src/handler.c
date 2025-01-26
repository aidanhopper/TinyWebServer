#include "headers.h"

#include "methods.c"

bool parse(request *req, char buffer[REQUEST_LENGTH]);
bool parse_start_line(request *req, char buffer[REQUEST_LENGTH]);
uint8_t *handle_response(request *req, uint64_t *response_length);
void init_request(request *req);

// Thinking of adding the connecting IP to the arguments
void handle_connection(int32_t connection) {
  char buffer[REQUEST_LENGTH];
  uint32_t bytes;
  bool success;

  while (true) {
    memset(buffer, '\0', REQUEST_LENGTH);

    bytes = recv(connection, buffer, REQUEST_LENGTH, 0);
    if (!bytes)
      return;

    request req;
    init_request(&req);
    strcpy(req.web_root, WEB_ROOT);

    printf("%s\n", buffer);

    success = parse(&req, buffer);
    if (!success)
      return;

    uint64_t response_length = 0;
    uint8_t *response = handle_response(&req, &response_length);

    /*memset(msg, '\0', 1024);*/
    /*char content[] = "<h1>THIS IS A HEADER</h1> \*/
    /*                  <p>this is a paragraph</p>";*/
    /*sprintf(msg,*/
    /*        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "*/
    /*        "%d\r\n\r\n%s",*/
    /*        (int32_t)strlen(content), content);*/
    /*strcat(msg, content);*/

    if (response != NULL) {
      printf("%s\n", response);
      send(connection, response, response_length, 0);
      free(response);
    }
  }
}

uint8_t *handle_response(request *req, uint64_t *response_length) {
  uint8_t *response = NULL;

  if (strcmp(req->method, "GET") == 0) {
    response = get(req, response_length);
  }

  else if (strcmp(req->method, "POST") == 0) {
    response = not_implemented(req);
  }

  else if (strcmp(req->method, "PUT") == 0) {
    response = not_implemented(req);
  }

  else if (strcmp(req->method, "DELETE") == 0) {
    response = not_implemented(req);
  }

  else if (strcmp(req->method, "PATCH") == 0) {
    response = not_implemented(req);
  }

  else if (strcmp(req->method, "CONNECT") == 0) {
    response = not_implemented(req);
  }

  else if (strcmp(req->method, "TRACE") == 0) {
    response = not_implemented(req);
  }

  else if (strcmp(req->method, "OPTIONS") == 0) {
    response = not_implemented(req);
  }

  else if (strcmp(req->method, "TRACE") == 0) {
    response = not_implemented(req);
  }

  else if (strcmp(req->method, "HEAD") == 0) {
    response = not_implemented(req);
  }

  return response;
}

bool parse(request *req, char buffer[REQUEST_LENGTH]) {
  char *rest_of_buffer = buffer;
  char *line;
  uint32_t line_number = 0;
  bool success;

  for (line = strtok_r(rest_of_buffer, "\r\n", &rest_of_buffer); line != NULL;
       line = strtok_r(NULL, "\r\n", &rest_of_buffer)) {

    // Parses the start-line
    if (line_number == 0) {
      success = parse_start_line(req, line);
      if (!success)
        return false;
    }

    // Parse the headers

    line_number++;
  }

  return true;
}

bool parse_start_line(request *req, char line[REQUEST_LENGTH]) {
  char *rest = line;
  char *token;

  token = strtok_r(rest, " ", &rest);
  if (token == NULL)
    return false;
  strncat(req->method, token, REQUEST_LENGTH);

  token = strtok_r(NULL, " ", &rest);
  if (token == NULL)
    return false;
  strncat(req->request_target, token, REQUEST_LENGTH);

  token = strtok_r(NULL, " ", &rest);
  if (token == NULL)
    return false;
  strncat(req->protocol, token, REQUEST_LENGTH);

  return true;
}

// bool parse_header(request *req, char line[REQUEST_LENGTH]) {}

void init_request(request *req) {
  req->protocol[0] = '\0';
  req->method[0] = '\0';
  req->request_target[0] = '\0';
  req->headers.host[0] = '\0';
  req->headers.connection[0] = '\0';
}
