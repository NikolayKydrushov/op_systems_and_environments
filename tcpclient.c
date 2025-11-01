#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr

int main(int argc, char *argv[]) {
  // Значения по умолчанию
  int bufsize = 100;

  // Проверка минимального количества аргументов
  if (argc < 3) {
    printf("Usage: %s <IP> <port> [bufsize]\n", argv[0]);
    printf("Default buffer size: %d\n", bufsize);
    exit(1);
  }

  // Парсинг дополнительных аргументов
  if (argc > 3) {
    bufsize = atoi(argv[3]);
  }

  // Проверка корректности значений
  if (bufsize < 1) {
    fprintf(stderr, "Invalid buffer size: %d. Buffer size must be positive\n", bufsize);
    exit(1);
  }

  char *ip = argv[1];
  int port = atoi(argv[2]);

  if (port < 1 || port > 65535) {
    fprintf(stderr, "Invalid port number: %d. Port must be between 1 and 65535\n", port);
    exit(1);
  }

  printf("Connecting to %s:%d with buffer size %d\n", ip, port, bufsize);

  int fd;
  int nread;
  char buf[bufsize];  // Используем переменную вместо константы
  struct sockaddr_in servaddr;
  
  const size_t size = sizeof(struct sockaddr_in);

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket creating");
    exit(1);
  }

  memset(&servaddr, 0, size);
  servaddr.sin_family = AF_INET;

  if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
    perror("bad address");
    exit(1);
  }

  servaddr.sin_port = htons(port);

  if (connect(fd, (SADDR *)&servaddr, size) < 0) {
    perror("connect");
    exit(1);
  }

  printf("Connection established. Input message to send:\n");
  while ((nread = read(0, buf, bufsize)) > 0) {  // Используем переменную вместо BUFSIZE
    if (write(fd, buf, nread) < 0) {
      perror("write");
      exit(1);
    }
  }

  if (nread < 0) {
    perror("read from stdin");
    exit(1);
  }

  close(fd);
  printf("Connection closed.\n");
  exit(0);
}
