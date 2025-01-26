#include "headers.h"

char *grab_file(char *path) {
  FILE *f = fopen(path, "r");

  if (f == NULL) {
    fprintf(stderr, "File was NULL\n");
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  uint32_t length = ftell(f);
  rewind(f);

  char *buffer = calloc(length + 1, sizeof(char));

  if (buffer == NULL) {
    fprintf(stderr, "Buffer was NULL\n");
    fclose(f);
    return NULL;
  }

  fread(buffer, sizeof(char), length, f);

  buffer[length] = '\0';

  return buffer;
}

// Super strcat that will resize a dynamically allocated destination when needed
void add(char **dst, uint32_t *capacity, char *src) {
  uint32_t dst_len = strlen(*dst);
  uint32_t src_len = strlen(src);

  if (dst_len + src_len > *capacity) {
    *dst = realloc(*dst, (dst_len + src_len) * 2);
    *capacity = (dst_len + src_len) * 2;
  }

  strcat(*dst, src);
  memset(*dst + dst_len + src_len, '\0', *capacity - dst_len - src_len);
}
