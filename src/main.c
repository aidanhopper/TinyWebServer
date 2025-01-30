#include "handler.c"
#include "headers.h"

int32_t open_listen_socket();

const int32_t BACKLOG = 5;
const char *PORT = "80";

void route1(const routes_t *routes, const request_t *req, response_t *res,
            const char *request_target) {

  printf("%s\n", request_target);
  set_protocol(res, "HTTP/1.1");
  if (!serve_file(res, req, "testsite", request_target)) {
    serve_route(routes, req, res, "/404");
  }
}

void route2(const routes_t *routes, const request_t *req, response_t *res,
            const char *request_target) {
  set_protocol(res, "HTTP/1.1");
  set_status_code(res, 200);
  set_header(res, "Content-Length", "0");
}

void notfound(const routes_t *routes, const request_t *req, response_t *res,
              const char *request_target) {
  set_protocol(res, "HTTP/1.1");
  set_status_code(res, 404);
  set_content(res, "<h1>404</h1>\n"
                   "not found\n");
  set_header(res, "Content-Type", "text/html");
}

int32_t main(int32_t argc, char *argv[]) {
  routes_t routes;
  init_routes(&routes);

  create_route(&routes, "/", route1);
  create_route(&routes, "/test/route", route1);
  create_route(&routes, "/asdf", route2);
  create_route(&routes, "/404", notfound);

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
