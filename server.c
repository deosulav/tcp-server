#include <stdio.h>

#include "socket/socket.h"
#include <string.h>
#include <time.h>

int main() {
  if (socketSetup() < 0) {
    fprintf(stderr, "Failed to initialize.\n");
  }

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo* bindAddress;
  int error = getaddrinfo(0, "8080", &hints, &bindAddress);
  if (error) {
    fprintf(stderr , "getaddrinfo() failed\n");
    return 1;
  }

  SOCKET socketListen;
  socketListen = socket(bindAddress->ai_family, bindAddress->ai_socktype, bindAddress->ai_protocol);
  if (!isValidSocket(socketListen)) {
      fprintf(stderr, "socket() failed(% d)\n", getSocketErrNo());
      return 1;
  }

  int option = 0;
  if (setsockopt(socketListen, IPPROTO_IPV6, IPV6_V6ONLY,
      (void*)&option, sizeof(option))) {
      fprintf(stderr, "setsockopt() failed. (%d)\n", getSocketErrNo());
      return 1;
  }

  if (bind(socketListen, bindAddress->ai_addr, bindAddress->ai_addrlen)) {
      fprintf(stderr, "bind() failed, (%d)\n", getSocketErrNo());
      return 1;
  }
  freeaddrinfo(bindAddress);
  printf("Listening...\n");
  if (listen(socketListen, 10) < 0) {
      fprintf(stderr, "listen() failed. (%d)\n", getSocketErrNo());
      return 1;
  }
  printf("Waiting for connection...\n");
  struct sockaddr_storage clientAddress;
  socklen_t client_len = sizeof(clientAddress);
  SOCKET socketClient = accept(socketListen,
      (struct sockaddr*)&clientAddress, &client_len);
  if (!isValidSocket(socketClient)) {
      fprintf(stderr, "accept() failed. (%d)\n", getSocketErrNo());
      return 1;
  }
  printf("Client is connected... ");
  char addressBuffer[100];
  getnameinfo((struct sockaddr*)&clientAddress,
      client_len, addressBuffer, sizeof(addressBuffer), 0, 0,
      NI_NUMERICHOST);
  printf("%s\n", addressBuffer);

  printf("Reading request.. \n");
  char request[1024];
  int bytesReceived = recv(socketClient, request, 1024, 0);
  printf("Recieved %.*s\n", bytesReceived, request);

  printf("Sending response...\n");
  const char* response =
      "HTTP/1.1 200 OK\r\n"
      "Connection: close\r\n"
      "Content-Type: text/plain\r\n\r\n"
      "Local time is: ";
  int bytesSent = send(socketClient, response, strlen(response), 0);
  printf("Send %d of %d bytes.\n", bytesSent, (int)strlen(response));

  time_t timer;
  time(&timer);
  char* timeMsg = ctime(&timer);
  bytesSent = send(socketClient, timeMsg, strlen(timeMsg), 0);
  
  printf("Send %d of %d bytes.\n", bytesSent, (int)strlen(timeMsg));
  printf("Closing connection...\n");
  closeSocket(socketClient);

  socketTeardown();
  return 0;
}
