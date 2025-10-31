#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/time.h>  // Добавляем для struct timeval
#include <arpa/inet.h> // Добавляем для inet_ntoa

struct Server {
  char ip[255];
  int port;
};

// Структура для передачи данных в поток
struct ThreadData {
  struct Server server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
  uint64_t result;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }

  return result % mod;
}

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

// Функция для чтения серверов из файла
int ReadServers(const char *filename, struct Server **servers, int *servers_num) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Cannot open servers file: %s\n", filename);
    return -1;
  }

  // Считаем количество серверов
  int count = 0;
  char line[256];
  while (fgets(line, sizeof(line), file)) {
    count++;
  }

  // Выделяем память
  *servers = malloc(sizeof(struct Server) * count);
  *servers_num = count;

  // Читаем данные серверов
  rewind(file);
  count = 0;
  while (fgets(line, sizeof(line), file)) {
    // Убираем символ новой строки
    line[strcspn(line, "\n")] = 0;
    
    // Парсим ip:port
    char *colon = strchr(line, ':');
    if (colon) {
      *colon = '\0';
      strcpy((*servers)[count].ip, line);
      (*servers)[count].port = atoi(colon + 1);
      count++;
    }
  }

  fclose(file);
  return 0;
}

// Функция для подключения к серверу и получения результата
void *ConnectToServer(void *arg) {
  struct ThreadData *data = (struct ThreadData *)arg;
  
  struct hostent *hostname = gethostbyname(data->server.ip);
  if (hostname == NULL) {
    fprintf(stderr, "gethostbyname failed with %s\n", data->server.ip);
    data->result = 1; // Нейтральный элемент для умножения
    return NULL;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(data->server.port);
  
  // ИСПРАВЛЕНИЕ: правильное копирование адреса из h_addr_list
  if (hostname->h_addr_list[0] != NULL) {
    server_addr.sin_addr = *((struct in_addr *)hostname->h_addr_list[0]);
  } else {
    fprintf(stderr, "No address found for %s\n", data->server.ip);
    data->result = 1;
    return NULL;
  }

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "Socket creation failed for %s:%d!\n", data->server.ip, data->server.port);
    data->result = 1;
    return NULL;
  }

  // Устанавливаем таймауты
  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    fprintf(stderr, "Connection to %s:%d failed\n", data->server.ip, data->server.port);
    close(sockfd);
    data->result = 1;
    return NULL;
  }

  // Подготавливаем задачу для сервера
  char task[sizeof(uint64_t) * 3];
  memcpy(task, &data->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &data->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &data->mod, sizeof(uint64_t));

  // Отправляем задачу
  if (send(sockfd, task, sizeof(task), 0) < 0) {
    fprintf(stderr, "Send to %s:%d failed\n", data->server.ip, data->server.port);
    close(sockfd);
    data->result = 1;
    return NULL;
  }

  // Получаем результат
  char response[sizeof(uint64_t)];
  ssize_t bytes_received = recv(sockfd, response, sizeof(response), 0);
  if (bytes_received < 0) {
    fprintf(stderr, "Receive from %s:%d failed\n", data->server.ip, data->server.port);
    close(sockfd);
    data->result = 1;
    return NULL;
  }

  if (bytes_received != sizeof(uint64_t)) {
    fprintf(stderr, "Invalid response size from %s:%d\n", data->server.ip, data->server.port);
    close(sockfd);
    data->result = 1;
    return NULL;
  }

  memcpy(&data->result, response, sizeof(uint64_t));
  
  printf("✓ Server %s:%d returned: %lu for range [%lu-%lu]\n", 
         data->server.ip, data->server.port, data->result, data->begin, data->end);

  close(sockfd);
  return NULL;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers_file[255] = {'\0'};

  while (true) {
    // Убираем неиспользуемую переменную
    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        break;
      case 2:
        memcpy(servers_file, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == (uint64_t)-1 || mod == (uint64_t)-1 || !strlen(servers_file)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  // Проверяем особые случаи
  if (k == 0) {
    printf("0! mod %lu = 1\n", mod);
    return 0;
  }

  // Читаем серверы из файла
  struct Server *servers = NULL;
  int servers_num = 0;
  if (ReadServers(servers_file, &servers, &servers_num) != 0) {
    return 1;
  }

  printf("🔍 Found %d servers in file: %s\n", servers_num, servers_file);
  printf("🎯 Calculating: %lu! mod %lu\n", k, mod);

  // Если серверов нет, используем локальный вычисления
  if (servers_num == 0) {
    fprintf(stderr, "No servers found in file\n");
    return 1;
  }

  // Разбиваем задачу между серверами
  pthread_t threads[servers_num];
  struct ThreadData thread_data[servers_num];

  uint64_t numbers_per_server = k / servers_num;
  uint64_t remainder = k % servers_num;

  uint64_t current_start = 1;
  for (int i = 0; i < servers_num; i++) {
    uint64_t current_end = current_start + numbers_per_server - 1;
    
    // Распределяем остаток по первым серверам
    if (i < (int)remainder) {
      current_end++;
    }

    // Заполняем данные для потока
    thread_data[i].server = servers[i];
    thread_data[i].begin = current_start;
    thread_data[i].end = current_end;
    thread_data[i].mod = mod;
    thread_data[i].result = 1; // Инициализируем нейтральным элементом

    printf("📡 Server %d (%s:%d): calculating range [%lu-%lu]\n", 
           i, servers[i].ip, servers[i].port, current_start, current_end);

    // Создаем поток для подключения к серверу
    if (pthread_create(&threads[i], NULL, ConnectToServer, &thread_data[i]) != 0) {
      fprintf(stderr, "❌ Failed to create thread for server %d\n", i);
      thread_data[i].result = 1; // Нейтральный элемент для умножения
    }

    current_start = current_end + 1;
  }

  // Ожидаем завершения всех потоков
  for (int i = 0; i < servers_num; i++) {
    pthread_join(threads[i], NULL);
  }

  // Агрегируем результаты
  uint64_t total_result = 1;
  int successful_servers = 0;
  for (int i = 0; i < servers_num; i++) {
    if (thread_data[i].result != 1) { // Если результат не нейтральный
      total_result = MultModulo(total_result, thread_data[i].result, mod);
      successful_servers++;
    }
  }

  printf("\n📊 Summary:\n");
  printf("   Successful servers: %d/%d\n", successful_servers, servers_num);
  printf("   Numbers calculated: 1 to %lu\n", k);
  printf("   Modulus: %lu\n", mod);
  
  printf("\n🎉 === FINAL RESULT ===\n");
  printf("   %lu! mod %lu = %lu\n", k, mod, total_result);

  // Проверяем, все ли серверы отработали успешно
  if (successful_servers < servers_num) {
    printf("⚠️  Warning: %d servers failed to respond\n", servers_num - successful_servers);
  }

  free(servers);
  return 0;
}
