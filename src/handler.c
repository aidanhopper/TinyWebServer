#include "headers.h"

#include "methods.c"

bool parse(request_t *req, char buffer[REQUEST_LENGTH]);
bool parse_start_line(request_t *req, char buffer[REQUEST_LENGTH]);
uint8_t *handle_response(request_t *req, uint64_t *response_length);
void init_request(request_t *req);

// Thinking of adding the connecting IP to the arguments
void handle_connection(int32_t connection, char *web_root) {
  char buffer[REQUEST_LENGTH];
  uint32_t bytes;
  bool success;

  while (true) {
    memset(buffer, 0, REQUEST_LENGTH);

    bytes = recv(connection, buffer, REQUEST_LENGTH, 0);
    if (!bytes)
      return;

    request_t req;
    init_request(&req);
    strcpy(req.web_root, web_root);

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

void get_file_extension(char *extension, char *request_target) {
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

uint8_t *handle_response(request_t *req, uint64_t *response_length) {
  uint8_t *response = NULL;
  get_path(req->path, req->request_target, req->web_root);
  get_file_extension(req->extension, req->request_target);
  req->filetype = get_filetype(req->extension);

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

bool parse_header(request_t *req, char buffer[REQUEST_LENGTH]) {
  char *rest = buffer;
  const char *del = " ";

  char *header;
  char *value;

  header = strtok_r(rest, del, &rest);
  if (header[strnlen(header, REQUEST_LENGTH) - 1] != ':')
    return false;

  value = header + strnlen(header, REQUEST_LENGTH) + 1;
  header[strnlen(header, REQUEST_LENGTH) - 1] = '\0';
  printf("Header is %s\nValue is %s\n\n", header, value);

  return true;
}

bool parse(request_t *req, char buffer[REQUEST_LENGTH]) {
  char *rest_of_buffer = buffer;
  char *line;
  uint32_t line_number = 0;
  bool success;
  bool in_headers = true;

  for (line = strtok_r(rest_of_buffer, "\r\n", &rest_of_buffer); line != NULL;
       line = strtok_r(NULL, "\r\n", &rest_of_buffer)) {

    // Parses the start-line
    if (line_number == 0) {
      success = parse_start_line(req, line);
      if (!success)
        return false;
    } else if (in_headers) {
      // parse a header
      parse_header(req, line);
    }

    // End of headers if true
    if (strnlen(line, REQUEST_LENGTH) == 0) {
      in_headers = false;
      printf("NO MORE HEADERS\n");
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

  token = strtok_r(rest, " ", &rest);
  if (token == NULL)
    return false;
  strncat(req->method, token, REQUEST_MEMBER_LENGTH);

  token = strtok_r(NULL, " ", &rest);
  if (token == NULL)
    return false;
  strncat(req->request_target, token, REQUEST_MEMBER_LENGTH);

  token = strtok_r(NULL, " ", &rest);
  if (token == NULL)
    return false;
  strncat(req->protocol, token, REQUEST_MEMBER_LENGTH);

  return true;
}

// bool parse_header(request *req, char line[REQUEST_LENGTH]) {}

void init_request(request_t *req) {
  req->protocol[0] = '\0';
  req->method[0] = '\0';
  req->request_target[0] = '\0';
  req->headers.host[0] = '\0';
  req->headers.connection[0] = '\0';
}
