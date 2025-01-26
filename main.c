#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

const int32_t BACKLOG = 5;
const char *PORT = "80";

int32_t main() {
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
    return 1;
  }

  s = socket(srvinfo->ai_family, srvinfo->ai_socktype, srvinfo->ai_protocol);

  status = bind(s, srvinfo->ai_addr, srvinfo->ai_addrlen);
  if (status != 0) {
    fprintf(stderr, "Error with bind: %d\n", status);
    return 1;
  }

  status = listen(s, BACKLOG);
  if (status != 0) {
    fprintf(stderr, "Error with listen: %d\n", status);
    return 1;
  }

  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;

  int32_t connection = accept(s, (struct sockaddr *)&their_addr, &addr_size);

  char *msg = "HELLO MY CONNECTED FRIEND!\n";
  send(connection, msg, strlen(msg), 0);

  char buffer[1024];

  while (1) {
    memset(buffer, '\0', 1024);
    recv(connection, buffer, 1024, 0);

    if (strcmp(buffer, "exit") == 0) {
      break;
    }
  }

  close(connection);

  close(s);
  freeaddrinfo(srvinfo);

  return 0;
}
