#include <_stdio.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/_types/_fd_def.h>
#if defined(_WIN32)
#include <conio.h>
#endif

#include "socket/socket.h"
#include <string.h>
#include <time.h>

int main() {
  if (socketSetup() < 0) {
    fprintf(stderr, "Failed to initialize.\n");
  }

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo *bindAddress;
  int error = getaddrinfo("example.com", "80", &hints, &bindAddress);
  if (error) {
    fprintf(stderr, "getaddrinfo() failed\n");
    return 1;
  }

  char addressBuffer[100], serviceBuffer[100];
  getnameinfo(bindAddress->ai_addr, bindAddress->ai_addrlen, addressBuffer,
              sizeof(addressBuffer), serviceBuffer, sizeof(serviceBuffer), 0);
  printf("%s %s\n", addressBuffer, serviceBuffer);

  SOCKET socketConnect;
  socketConnect = socket(bindAddress->ai_family, bindAddress->ai_socktype,
                         bindAddress->ai_protocol);
  if (!isValidSocket(socketConnect)) {
    fprintf(stderr, "socket() failed(% d)\n", getSocketErrNo());
    return 1;
  }

  if (connect(socketConnect, bindAddress->ai_addr, bindAddress->ai_addrlen) <
      0) {
    fprintf(stderr, "connect() failed, (%d) %s\n", getSocketErrNo(),
            strerror(getSocketErrNo()));
    return 1;
  }
  freeaddrinfo(bindAddress);

  printf("Connected...\n");
  while (1) {
    fd_set reads;
    FD_ZERO(&reads);
    FD_SET(socketConnect, &reads);
#if !defined(_WIN32)
    FD_SET(fileno(stdin), &reads);
#endif
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    if (select(socketConnect + 1, &reads, 0, 0, &timeout) < 0) {
      fprintf(stderr, "select() failed. (%d)\n", getSocketErrNo());
      return 1;
    }

    if (FD_ISSET(socketConnect, &reads)) {
      char read[4096];
      int bytes_received = recv(socketConnect, read, 4096, 0);
      if (bytes_received < 1) {
        printf("Connection closed by peer.\n");
        break;
      }
      printf("Received (%d bytes): %.*s", bytes_received, bytes_received, read);
    }
    if (FD_ISSET(0, &reads)) {
      char read[4096];
      if (!fgets(read, 4096, stdin))
        break;
      printf("Sending: %s", read);
      int bytes_sent = send(socketConnect, read, strlen(read), 0);
      printf("Sent %d bytes.\n", bytes_sent);
    }
  }

  closeSocket(socketConnect);

  printf("Closing connection...\n");
  socketTeardown();
  return 0;
}
