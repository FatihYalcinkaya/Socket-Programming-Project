#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

int main()

{
  // creating socket
  int socketFD = socket(AF_INET, SOCK_STREAM, 0);

  char *ip = "142.258.188.46";

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(80); // port adresi
  inet_pton(AF_INET, ip, &address.sin_addr.s_addr);
  address.sin_addr.s_addr;

  int result =
      connect(socketFD, &address, sizeof address); // socket baglandı mı control

  if (result == 0) {
    printf("connection succesfull");
  }

  return 0;
}
