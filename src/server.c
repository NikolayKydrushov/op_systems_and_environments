#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "pthread.h"
#include "common.h"

void *ThreadFactorial(void *args) {
    struct FactorialArgs *fargs = (struct FactorialArgs *)args;
    uint64_t *result = malloc(sizeof(uint64_t));
    *result = Factorial(fargs);
    return (void *)result;
}

// Функция для разбиения диапазона между потоками
void DivideRange(uint64_t begin, uint64_t end, int tnum, 
                 struct FactorialArgs *args, uint64_t mod) {
    uint64_t total_numbers = end - begin + 1;
    uint64_t numbers_per_thread = total_numbers / tnum;
    uint64_t remainder = total_numbers % tnum;

    uint64_t current_start = begin;
    for (int i = 0; i < tnum; i++) {
        uint64_t current_end = current_start + numbers_per_thread - 1;
        
        // ИСПРАВЛЕНИЕ: приведение типов для сравнения
        if (i < (int)remainder) {
            current_end++;
        }

        if (current_end > end) {
            current_end = end;
        }

        args[i].begin = current_start;
        args[i].end = current_end;
        args[i].mod = mod;

        printf("   Thread %d: calculating range [%lu-%lu] (%lu numbers)\n", 
               i, current_start, current_end, current_end - current_start + 1);

        current_start = current_end + 1;
        
        if (current_start > end) {
            break;
        }
    }
}

int main(int argc, char **argv) {
    int tnum = -1;
    int port = -1;

    while (true) {
        static struct option options[] = {{"port", required_argument, 0, 0},
                                          {"tnum", required_argument, 0, 0},
                                          {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
        case 0: {
            switch (option_index) {
            case 0:
                port = atoi(optarg);
                if (port <= 0 || port > 65535) {
                    fprintf(stderr, "Invalid port number: %d\n", port);
                    return 1;
                }
                break;
            case 1:
                tnum = atoi(optarg);
                if (tnum <= 0) {
                    fprintf(stderr, "Number of threads must be positive: %d\n", tnum);
                    return 1;
                }
                break;
            default:
                printf("Index %d is out of options\n", option_index);
            }
        } break;

        case '?':
            printf("Unknown argument\n");
            break;
        default:
            fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (port == -1 || tnum == -1) {
        fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
        return 1;
    }

    printf("🚀 Starting server on port %d with %d threads\n", port, tnum);

    int server_fd = CreateServerSocket(port);
    if (server_fd < 0) {
        return 1;
    }

    printf("✅ Server successfully started and listening on port %d\n", port);
    printf("📡 Waiting for client connections...\n\n");

    while (true) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

        if (client_fd < 0) {
            fprintf(stderr, "Could not establish new connection\n");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("🔗 New client connected: %s:%d\n", 
               client_ip, ntohs(client.sin_port));

        while (true) {
            unsigned int buffer_size = sizeof(uint64_t) * 3;
            char from_client[buffer_size];
            int read_bytes = recv(client_fd, from_client, buffer_size, 0);

            if (read_bytes == 0) {
                printf("🔌 Client %s:%d disconnected\n", client_ip, ntohs(client.sin_port));
                break;
            }
            
            if (read_bytes < 0) {
                fprintf(stderr, "Client read failed\n");
                break;
            }
            
            // ИСПРАВЛЕНИЕ: приведение типов для сравнения
            if (read_bytes < (int)buffer_size) {
                fprintf(stderr, "Client send wrong data format (expected %u bytes, got %d)\n", 
                        buffer_size, read_bytes);
                break;
            }

            uint64_t begin = 0;
            uint64_t end = 0;
            uint64_t mod = 0;
            memcpy(&begin, from_client, sizeof(uint64_t));
            memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
            memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

            printf("📨 Received from client %s:%d:\n", client_ip, ntohs(client.sin_port));
            printf("   Range: [%lu - %lu]\n", begin, end);
            printf("   Modulus: %lu\n", mod);
            printf("   Total numbers: %lu\n", end - begin + 1);

            if (begin > end) {
                fprintf(stderr, "Invalid range: begin (%lu) > end (%lu)\n", begin, end);
                uint64_t error_result = 0;
                char buffer[sizeof(error_result)];
                memcpy(buffer, &error_result, sizeof(error_result));
                send(client_fd, buffer, sizeof(error_result), 0);
                continue;
            }

            if (mod == 0) {
                fprintf(stderr, "Invalid modulus: cannot be zero\n");
                uint64_t error_result = 0;
                char buffer[sizeof(error_result)];
                memcpy(buffer, &error_result, sizeof(error_result));
                send(client_fd, buffer, sizeof(error_result), 0);
                continue;
            }

            if (begin == 0 && end == 0) {
                uint64_t result = 1;
                char buffer[sizeof(result)];
                memcpy(buffer, &result, sizeof(result));
                send(client_fd, buffer, sizeof(result), 0);
                printf("   Special case: 0! = 1\n");
                continue;
            }

            if (begin == 0) {
                begin = 1;
                printf("   Adjusted range: [1 - %lu] (0 excluded)\n", end);
            }

            pthread_t threads[tnum];
            struct FactorialArgs args[tnum];

            DivideRange(begin, end, tnum, args, mod);

            int actual_threads = tnum;
            if (end - begin + 1 < (uint64_t)tnum) {
                actual_threads = end - begin + 1;
            }

            printf("   Starting %d threads for calculation...\n", actual_threads);

            for (int i = 0; i < actual_threads; i++) {
                if (pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i])) {
                    fprintf(stderr, "Error: pthread_create failed for thread %d!\n", i);
                    args[i].begin = 1;
                    args[i].end = 1;
                    args[i].mod = mod;
                }
            }

            uint64_t total = 1;
            for (int i = 0; i < actual_threads; i++) {
                uint64_t *thread_result = NULL;
                pthread_join(threads[i], (void **)&thread_result);
                if (thread_result != NULL) {
                    total = MultModulo(total, *thread_result, mod);
                    free(thread_result);
                }
            }

            printf("✅ Total result for range [%lu-%lu] mod %lu = %lu\n", 
                   begin, end, mod, total);

            char buffer[sizeof(total)];
            memcpy(buffer, &total, sizeof(total));
            int send_result = send(client_fd, buffer, sizeof(total), 0);
            if (send_result < 0) {
                fprintf(stderr, "Can't send data to client\n");
                break;
            }

            printf("📤 Result sent to client\n\n");
        }

        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
        printf("🔒 Connection with client %s:%d closed\n\n", 
               client_ip, ntohs(client.sin_port));
    }

    close(server_fd);
    return 0;
}
