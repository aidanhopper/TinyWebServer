#include "builder.c"
#include "headers.h"

typedef enum filetype {
  HTML = 0,
  CSS,
  JS,
  PNG,
  PDF,
  NOT_SUPPORTED,
} filetype_t;

const char *FILETYPE_MAPPING[] = {"text/html", "text/css", "text/js",
                                  "image/png", "application/pdf", "text/plain"};

const char endl[10] = "\r\n";

uint8_t *init_response(int capacity) {
  uint8_t *response = calloc(sizeof(uint8_t), capacity);
  return response;
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

uint8_t *get(request *req, uint64_t *response_length) {
  uint64_t capacity = 1024;
  uint8_t *response = init_response(capacity);

  char path[REQUEST_LENGTH];
  get_path(path, req->request_target, req->web_root);

  uint64_t content_length = 0;
  uint8_t *content = grab_file(path, &content_length);

  char content_length_header[100] = "Content-Length: 0\r\n";
  if (content != NULL)
    sprintf(content_length_header, "Content-Length: %llu\r\n", content_length);

  char extension[100];
  get_file_extension(extension, req->request_target);
  filetype_t filetype = get_filetype(extension);

  char content_type_header[100];
  sprintf(content_type_header, "Content-Type: %s\r\n",
          FILETYPE_MAPPING[filetype]);

  char start_line[] = "HTTP/1.1 200 OK\r\n";

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

uint8_t *not_implemented(request *req) {
  uint64_t capacity = 100;
  uint8_t *response = init_response(capacity);
  uint64_t response_length = 0;
  const char start_line[] = "HTTP/1.1 501 Not Implemented\r\n";
  add(&response, &capacity, &response_length, (uint8_t *)start_line,
      strlen(start_line));
  add(&response, &capacity, &response_length, (uint8_t *)endl, strlen(endl));

  return response;
}
