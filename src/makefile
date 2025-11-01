CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread
LDFLAGS = -pthread

# Цели по умолчанию
all: server client

# Общая библиотека
common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c -o common.o

# Сервер
server: server.c common.o
	$(CC) $(CFLAGS) server.c common.o -o server $(LDFLAGS)

# Клиент
client: client.c common.o
	$(CC) $(CFLAGS) client.c common.o -o client $(LDFLAGS)

# Очистка
clean:
	rm -f server client *.o

# Пересборка
rebuild: clean all

# Запуск сервера (пример)
run-server: server
	./server --port 20001 --tnum 4

# Запуск клиента (пример) - требует файл servers.txt
run-client: client
	./client --k 1000 --mod 5 --servers servers.txt

.PHONY: all clean rebuild run-server run-client
