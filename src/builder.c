#include "headers.h"
#include <stdint.h>

uint8_t *grab_file(char *path, uint64_t *length) {
  FILE *f = fopen(path, "rb");

  if (f == NULL) {
    fprintf(stderr, "File was NULL\n");
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  *length = ftell(f);
  rewind(f);

  uint8_t *buffer = calloc(*length, sizeof(uint8_t));

  if (buffer == NULL) {
    fprintf(stderr, "Buffer was NULL\n");
    fclose(f);
    return NULL;
  }

  fread(buffer, sizeof(uint8_t), *length, f);
  /*if (bytes_read != *length) {*/
  /*  free(buffer);*/
  /*  fclose(f);*/
  /*  return NULL;*/
  /*}*/

  fclose(f);
  return buffer;
}

// Super strcat that will resize a dynamically allocated destination when needed
void add(uint8_t **dst, uint64_t *capacity, uint64_t *dst_len, uint8_t *src,
         uint64_t src_len) {
  if (*dst_len + src_len + 1 > *capacity) {
    uint64_t new_capacity = *dst_len + src_len + 1;
    *dst = realloc(*dst, new_capacity * 2);
    *capacity = new_capacity * 2;
  }

  memcpy(*dst + *dst_len, src, src_len);

  *dst_len += src_len;
}
