#include "handler.c"
#include "headers.h"

int32_t open_listen_socket();

const int32_t BACKLOG = 5;
const char *PORT = "80";

int32_t main(int32_t argc, char *argv[]) {
  if (argc == 1) {
    fprintf(stderr, "A web root must be specified\n");
    return 1;
  }

  printf("Starting server on port %s\n", PORT);

  // Open a socket to listen for connections
  uint32_t s = open_listen_socket();
  if (s == -1) {
    close(s);
    return 1;
  }

  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;

  char *web_root = argv[1];

  // Connection loop
  while (1) {
    // Accept the connection
    int32_t connection = accept(s, (struct sockaddr *)&their_addr, &addr_size);

    // Enter if child process
    if (!fork()) {
      // Not needed in child process
      close(s);

      handle_connection(connection, web_root);

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
