# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS =

# Цели по умолчанию
all: tcp-server tcp-client udp-server udp-client

# TCP приложения
tcp-server: tcpserver.c
	$(CC) $(CFLAGS) -o tcp-server tcpserver.c $(LDFLAGS)

tcp-client: tcpclient.c
	$(CC) $(CFLAGS) -o tcp-client tcpclient.c $(LDFLAGS)

# UDP приложения
udp-server: udpserver.c
	$(CC) $(CFLAGS) -o udp-server udpserver.c $(LDFLAGS)

udp-client: udpclient.c
	$(CC) $(CFLAGS) -o udp-client udpclient.c $(LDFLAGS)

# Очистка
clean:
	rm -f tcp-server tcp-client udp-server udp-client

# Пересборка
rebuild: clean all

# Псевдоцели
.PHONY: all clean rebuild
