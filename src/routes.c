#include "headers.h"
#include <string.h>

void init_routes(routes_t *routes) {
  routes->route_list_length = 0;
  routes->route_list_capacity = 2;
  routes->route_list = calloc(routes->route_list_capacity, sizeof(route_t *));
}

void create_route(routes_t *routes, const char *path, route_handler handler) {
  route_t route;
  strcpy(route.path, path);
  route.handler = handler;

  if (routes->route_list_capacity <= routes->route_list_length + 2) {
    routes->route_list =
        realloc(routes->route_list, routes->route_list_capacity * 2);
    routes->route_list_capacity *= 2;
  }

  routes->route_list[routes->route_list_length] = route;

  routes->route_list_length++;
}

char **tokenize_path(const char *path, uint32_t *tokens_length) {
  uint32_t path_len = strlen(path);
  if (path_len == 0)
    return NULL;

  uint32_t capacity = 8;
  char **tokens = calloc(capacity, sizeof(char *));

  tokens[0] = malloc(sizeof(char) * 2);
  strcpy(tokens[0], "/");

  *tokens_length = 1;

  uint32_t l = 1;
  uint32_t r = 1;

  while (path[l] == '/')
    l++;
  r = l;

  while (r < path_len) {
    if (path[r] == '/' || r == path_len - 1) {
      if (*tokens_length == capacity) {
        tokens = realloc(tokens, capacity * 2 * sizeof(char *));
        capacity *= 2;
      }

      if (r == path_len - 1)
        r++;

      tokens[*tokens_length] = malloc(sizeof(char) * (r - l));
      strncat(tokens[*tokens_length], path + l, r - l);
      (*tokens_length)++;

      l = r;
      while (path[l] == '/')
        l++;
      r = l;
    }

    r++;
  }

  return tokens;
}

void free_tokens(char **tokens, uint32_t tokens_length) {
  for (int i = 0; i < tokens_length; i++)
    free(tokens[i]);
  free(tokens);
}

void serve_route(const routes_t *routes, const request_t *req, response_t *res,
                 const char *path) {
  if (routes->route_list_length == 0)
    return;

  uint32_t path_tokens_length = 0;
  char **path_tokens = tokenize_path(path, &path_tokens_length);
  if (path_tokens == NULL)
    return;

  route_t *best_route = NULL;
  uint32_t best_route_score = 0;

  // search through the routes list
  for (int i = 0; i < routes->route_list_length; i++) {
    route_t *route = &routes->route_list[i];
    const char *route_path = route->path;

    uint32_t route_path_tokens_length = 0;
    char **route_path_tokens =
        tokenize_path(route_path, &route_path_tokens_length);

    if (route_path_tokens_length <= path_tokens_length) {
      uint32_t score = 0;
      for (int j = 0; j < route_path_tokens_length; j++) {
        if (strcmp(route_path_tokens[j], path_tokens[j]) != 0)
          break;
        score++;
      }

      if (score > best_route_score) {
        best_route_score = score;
        best_route = route;
      }
    }

    free_tokens(route_path_tokens, route_path_tokens_length);
  }

  free_tokens(path_tokens, path_tokens_length);

  // TODO
  // The input path needs to be modified to remove matched tokens from the best
  // match route
  best_route->handler(routes, req, res, path);
}
