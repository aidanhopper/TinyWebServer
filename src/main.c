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
  route_t *route = calloc(1, sizeof(route_t));
  strncat(route->path, path, SERVER_MEMBER_LENGTH);
  route->handler = handler;

  if (routes->route_list_capacity <= routes->route_list_length) {
    routes->route_list =
        realloc(routes->route_list, routes->route_list_capacity * 2);
    routes->route_list_capacity *= 2;
  }

  routes->route_list[routes->route_list_length++] = route;
}

void route1(const request_t *req, response_t *res) {
  printf("HELLO WORLD !\n"); //
}

void handle_route(routes_t *routes, const request_t *req, response_t *res) {
  // grab the path
  const char *path = req->path;

  // break it into tokens
  // ex: /thing/box  -->  / - thing - box  -->  3 tokens
  // find the best match within the routes

  // search througth the routes list
  for (int i = 0; i < routes->route_list_length; i++) {
    const route_t *route = routes->route_list[i];
    const char *route_path = route->path;

  }
}

int32_t main(int32_t argc, char *argv[]) {
  printf("Starting server on port %s\n", PORT);

  // Open a socket to listen for connections
  uint32_t s = open_listen_socket();
  if (s == -1) {
    close(s);
    return 1;
  }

  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;

  routes_t routes;
  init_routes(&routes);

  create_route(&routes, "/", route1);
  create_route(&routes, "/asdf", route1);

  response_t res;
  request_t req;

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
