#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr

int main(int argc, char **argv) {
  // Значения по умолчанию
  int serv_port = 10050;
  int bufsize = 100;

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

  printf("Starting TCP server on port %d with buffer size %d\n", serv_port, bufsize);

  const size_t kSize = sizeof(struct sockaddr_in);

  int lfd, cfd;
  int nread;
  char buf[bufsize];  // Используем переменную вместо константы
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  memset(&servaddr, 0, kSize);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(serv_port);  // Используем переменную вместо SERV_PORT

  if (bind(lfd, (SADDR *)&servaddr, kSize) < 0) {
    perror("bind");
    exit(1);
  }

  if (listen(lfd, 5) < 0) {
    perror("listen");
    exit(1);
  }

  printf("Server is listening on port %d...\n", serv_port);

  while (1) {
    unsigned int clilen = kSize;

    if ((cfd = accept(lfd, (SADDR *)&cliaddr, &clilen)) < 0) {
      perror("accept");
      exit(1);
    }
    printf("connection established\n");

    while ((nread = read(cfd, buf, bufsize)) > 0) {  // Используем переменную вместо BUFSIZE
      write(1, &buf, nread);
    }

    if (nread == -1) {
      perror("read");
      exit(1);
    }
    close(cfd);
  }
}
