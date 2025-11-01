#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr

int main(int argc, char *argv[]) {
  // Значения по умолчанию
  int serv_port = 20001;
  int bufsize = 1024;

  // Парсинг аргументов командной строки
  if (argc > 1) {
    serv_port = atoi(argv[1]);
  }
  if (argc > 2) {
    bufsize = atoi(argv[2]);
  }

  // Проверка корректности значений
  if (serv_port < 1 || serv_port > 65535) {
    fprintf(stderr, "Invalid port number: %d. Port must be between 1 and 65535\n", serv_port);
    exit(1);
  }

  if (bufsize < 1) {
    fprintf(stderr, "Invalid buffer size: %d. Buffer size must be positive\n", bufsize);
    exit(1);
  }

  printf("Starting UDP server on port %d with buffer size %d\n", serv_port, bufsize);

  int sockfd, n;
  char mesg[bufsize], ipadr[16];  // Используем переменную вместо константы
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  const size_t slen = sizeof(struct sockaddr_in);

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem");
    exit(1);
  }

  memset(&servaddr, 0, slen);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(serv_port);  // Используем переменную вместо SERV_PORT

  if (bind(sockfd, (SADDR *)&servaddr, slen) < 0) {
    perror("bind problem");
    exit(1);
  }
  
  printf("SERVER starts...\n");

  while (1) {
    unsigned int len = slen;

    if ((n = recvfrom(sockfd, mesg, bufsize, 0, (SADDR *)&cliaddr, &len)) < 0) {  // Используем переменную вместо BUFSIZE
      perror("recvfrom");
      exit(1);
    }
    mesg[n] = 0;

    printf("REQUEST %s      FROM %s : %d\n", mesg,
           inet_ntop(AF_INET, (void *)&cliaddr.sin_addr.s_addr, ipadr, 16),
           ntohs(cliaddr.sin_port));

    if (sendto(sockfd, mesg, n, 0, (SADDR *)&cliaddr, len) < 0) {
      perror("sendto");
      exit(1);
    }
  }
}
