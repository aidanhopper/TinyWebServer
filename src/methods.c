#include "builder.c"
#include "headers.h"

char *init_response(int capacity) {
  char *response = malloc(capacity);
  memset(response, '\0', capacity);
  return response;
}

char *get(request *req) {
  uint32_t capacity = 100;
  char *response = init_response(capacity);

  char *content = NULL;
  char path[REQUEST_LENGTH];
  char filetype[REQUEST_LENGTH] = "html";

  int32_t dot_index = -1;
  for (int i = 0; i < strlen(req->request_target); i++)
    if (req->request_target[i] == '.') {
      dot_index = i;
      break;
    }

  if (dot_index != -1) {
    strcpy(filetype, req->request_target + dot_index + 1);
    printf("FILETYPE: %s\n", req->request_target + dot_index + 1);
  }

  if (strlen(req->request_target) - 1 == dot_index)
    return NULL;

  if (strcmp(req->request_target, "/") == 0) {
    sprintf(path, "%s/index.html", req->web_root);
  } else if (dot_index == -1) {
    sprintf(path, "%s/%s.html", req->web_root, req->request_target);
  } else {
    sprintf(path, "%s/%s", req->web_root, req->request_target);
  }

  content = grab_file(path);

  char content_length_header[100] = "Content-Length: 0\r\n";
  if (content != NULL)
    sprintf(content_length_header, "Content-Length: %lu\r\n", strlen(content));

  char content_type_header[100];
  sprintf(content_type_header, "Content-Type: text/%s\r\n", filetype);

  add(&response, &capacity, "HTTP/1.1 200 OK\r\n");
  add(&response, &capacity, content_length_header);
  add(&response, &capacity, content_type_header);
  add(&response, &capacity, "\r\n");
  add(&response, &capacity, "\r\n");
  add(&response, &capacity, content);

  printf("RESPONSE:\n%s\n", response);

  if (content != NULL)
    free(content);

  return response;
}

char *not_implemented(request *req) {
  uint32_t capacity = 100;
  char *response = init_response(capacity);

  add(&response, &capacity, "HTTP/1.1 501 Not Implemented\r\n");
  add(&response, &capacity, "\r\n");

  return response;
}
