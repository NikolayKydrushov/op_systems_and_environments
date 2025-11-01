#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr

int main(int argc, char **argv) {
  // Значения по умолчанию
  int serv_port = 20001;
  int bufsize = 1024;

  // Проверка минимального количества аргументов
  if (argc < 2) {
    printf("Usage: %s <IP> [port] [bufsize]\n", argv[0]);
    printf("Default port: %d, default buffer size: %d\n", serv_port, bufsize);
    exit(1);
  }

  // Парсинг дополнительных аргументов
  if (argc > 2) {
    serv_port = atoi(argv[2]);
  }
  if (argc > 3) {
    bufsize = atoi(argv[3]);
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

  char *ip = argv[1];
  printf("Connecting to UDP server %s:%d with buffer size %d\n", ip, serv_port, bufsize);

  int sockfd, n;
  char sendline[bufsize], recvline[bufsize + 1];  // Используем переменную вместо константы
  struct sockaddr_in servaddr;
  
  const size_t slen = sizeof(struct sockaddr_in);

  memset(&servaddr, 0, slen);
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(serv_port);  // Используем переменную вместо SERV_PORT

  if (inet_pton(AF_INET, ip, &servaddr.sin_addr) < 0) {
    perror("inet_pton problem");
    exit(1);
  }
  
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem");
    exit(1);
  }

  write(1, "Enter string\n", 13);

  while ((n = read(0, sendline, bufsize)) > 0) {  // Используем переменную вместо BUFSIZE
    if (sendto(sockfd, sendline, n, 0, (SADDR *)&servaddr, slen) == -1) {  // Используем переменную вместо SLEN
      perror("sendto problem");
      exit(1);
    }

    if (recvfrom(sockfd, recvline, bufsize, 0, NULL, NULL) == -1) {  // Используем переменную вместо BUFSIZE
      perror("recvfrom problem");
      exit(1);
    }

    printf("REPLY FROM SERVER= %s\n", recvline);
  }
  
  close(sockfd);
}
