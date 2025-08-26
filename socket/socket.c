#include "socket.h"

int socketSetup() {
#if defined(_WIN32)
  WSADATA d;
  if (WSAStartup(MAKEWORD(2, 2), &d)) return -1;
#endif
  return 0;
}

void socketTeardown() {
#if defined(_WIN32)
  WSACleanup();
#endif
}


