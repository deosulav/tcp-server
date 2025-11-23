#include "socket/socket.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

SOCKET bindAndListen(const char *restrict port) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *bindAddress;
  int error = getaddrinfo(0, port, &hints, &bindAddress);
  if (error) {
    fprintf(stderr, "getaddrinfo() failed\n");
    return INVALID_SOCKET;
  }

  SOCKET socketListen;
  socketListen = socket(bindAddress->ai_family, bindAddress->ai_socktype,
                        bindAddress->ai_protocol);
  if (!isValidSocket(socketListen)) {
    fprintf(stderr, "socket() failed(% d)\n", getSocketErrNo());
    return INVALID_SOCKET;
  }

  int option = 0;
  if (setsockopt(socketListen, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&option,
                 sizeof(option))) {
    fprintf(stderr, "setsockopt() failed. (%d)\n", getSocketErrNo());
    return INVALID_SOCKET;
  }

  if (bind(socketListen, bindAddress->ai_addr, bindAddress->ai_addrlen)) {
    fprintf(stderr, "bind() failed, (%d) %s\n", getSocketErrNo(),
            strerror(getSocketErrNo()));
    return INVALID_SOCKET;
  }
  freeaddrinfo(bindAddress);
  if (listen(socketListen, 10) < 0) {
    fprintf(stderr, "listen() failed. (%d)\n", getSocketErrNo());
    return INVALID_SOCKET;
  }
  printf("Listening on port %s...\n", port);
  return socketListen;
}

int acceptClient(SOCKET socketListen, fd_set *readsAcc, SOCKET *maxRead) {
  struct sockaddr_storage clientAddress;
  socklen_t client_len = sizeof(clientAddress);
  SOCKET socketClient =
      accept(socketListen, (struct sockaddr *)&clientAddress, &client_len);
  FD_SET(socketClient, readsAcc);
  if (socketClient > *maxRead)
    *maxRead = socketClient;

  if (!isValidSocket(socketClient)) {
    fprintf(stderr, "accept() failed. (%d)\n", getSocketErrNo());
    return -1;
  }
  printf("Client is connected... ");
  char addressBuffer[100];
  getnameinfo((struct sockaddr *)&clientAddress, client_len, addressBuffer,
              sizeof(addressBuffer), 0, 0, NI_NUMERICHOST);
  printf("%s\n", addressBuffer);
  return 0;
}

int serviceClient(SOCKET socketClient, SOCKET socketListen, SOCKET *maxRead,
                  fd_set *reads) {
  char request[1024];
  int bytesReceived = recv(socketClient, request, sizeof(request), 0);
  if (bytesReceived < 1) {
    closeSocket(socketClient);
    return -1;
  }
  printf("%.*s\n", bytesReceived, request);
  for (int j = 1; j <= *maxRead; ++j) {
    if (FD_ISSET(j, reads)) {
      if (j == socketListen || j == socketClient) {
        continue;
      } else {
        send(j, request, bytesReceived, 0);
      }
    }
  }
  return 0;
}

int main() {
  if (socketSetup() < 0) {
    fprintf(stderr, "Failed to initialize.\n");
  }

  SOCKET socketListen = bindAndListen("8080");
  if (!isValidSocket(socketListen)) {
    return 1;
  }

  fd_set readsAcc;
  FD_ZERO(&readsAcc);
  FD_SET(socketListen, &readsAcc);
  SOCKET maxRead = socketListen;

  while (1) {
    fd_set reads;
    reads = readsAcc;
    if (select(maxRead + 1, &reads, 0, 0, 0) < 0) {
      fprintf(stderr, "select() failed. (%d)", getSocketErrNo());
      return 1;
    }

    for (int i = 1; i <= maxRead; i++) {
      if (FD_ISSET(i, &reads)) {
        if (i == socketListen) {
          acceptClient(socketListen, &readsAcc, &maxRead);
        } else {
          int isClosed =
              serviceClient(i, socketListen, &maxRead, &readsAcc) < 0;
          if (isClosed)
            FD_CLR(i, &readsAcc);
        }
      }
    }
  }

  printf("Stop listening");
  closeSocket(socketListen);
  socketTeardown();
  return 0;
}
