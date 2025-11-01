#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>

// Структура для передачи аргументов факториала
struct FactorialArgs {
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
};

// Математические функции
uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod);
uint64_t Factorial(const struct FactorialArgs *args);

// Функции для работы с сетью
int CreateServerSocket(int port);
bool ConvertStringToUI64(const char *str, uint64_t *val);

// Функции для работы с серверами
struct Server {
    char ip[255];
    int port;
};

int ReadServers(const char *filename, struct Server **servers, int *servers_num);

#endif // COMMON_H
