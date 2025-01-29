#include "handler.c"
#include "headers.h"

int32_t open_listen_socket();

const int32_t BACKLOG = 5;
const char *PORT = "80";

void init_routes(routes_t *routes) {
  routes->route_list_length = 0;
  routes->route_list_capacity = 1;
  routes->route_list = calloc(routes->route_list_capacity, sizeof(route_t *));
}

void create_route(routes_t *routes, const char *path, route_handler handler) {
  route_t route;

  strncat(route.path, path, SERVER_MEMBER_LENGTH);
  route.handler = handler;

  if (routes->route_list_capacity <= routes->route_list_length) {
    routes->route_list =
        realloc(routes->route_list, routes->route_list_capacity * 2);
    routes->route_list_capacity *= 2;
  }

  routes->route_list[routes->route_list_length++] = route;
}

void route1(const request_t *req, response_t *res) {
  printf("THIS IS ROUTE 1!\n"); //
}

void route2(const request_t *req, response_t *res) {
  printf("THIS IS ROUTE 2!\n"); //
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

void handle_route(routes_t *routes, const request_t *req, response_t *res) {
  if (routes->route_list_length == 0)
    return;

  // grab the path
  const char *path = req->path;

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

  best_route->handler(req, res);
}

int32_t main(int32_t argc, char *argv[]) {
  routes_t routes;
  init_routes(&routes);

  create_route(&routes, "/", route1);
  create_route(&routes, "/asdf", route2);

  request_t req;
  strcpy(req.path, "/");
  response_t res;
  handle_route(&routes, &req, &res);
  strcpy(req.path, "/asdf/123");
  handle_route(&routes, &req, &res);

  exit(0);
  printf("Starting server on port %s\n", PORT);

  // Open a socket to listen for connections
  uint32_t s = open_listen_socket();
  if (s == -1) {
    close(s);
    return 1;
  }

  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;

  // Connection loop
  while (1) {
    // Accept the connection
    int32_t connection = accept(s, (struct sockaddr *)&their_addr, &addr_size);

    // Enter if child process
    if (!fork()) {
      // Not needed in child process
      close(s);

      handle_connection(&routes, connection);

      // Clean up
      close(connection);
      exit(0);
    }

    // Not needed in parent process anymoree
    close(connection);
  }

  close(s);

  return 0;
}

// returns a socket
int32_t open_listen_socket() {
  int32_t status;
  int32_t s;
  struct addrinfo hints;
  struct addrinfo *srvinfo;
  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  status = getaddrinfo(NULL, PORT, &hints, &srvinfo);
  if (status != 0) {
    fprintf(stderr, "Error with getaddrinfo: %d\n", status);
    return -1;
  }

  s = socket(srvinfo->ai_family, srvinfo->ai_socktype, srvinfo->ai_protocol);

  status = bind(s, srvinfo->ai_addr, srvinfo->ai_addrlen);
  if (status != 0) {
    fprintf(stderr, "Error with bind: %d\n", status);
    return -1;
  }

  status = listen(s, BACKLOG);
  if (status != 0) {
    fprintf(stderr, "Error with listen: %d\n", status);
    return -1;
  }

  freeaddrinfo(srvinfo);

  return s;
}
