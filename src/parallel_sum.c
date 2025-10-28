#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include <getopt.h>

#include "utils.h"        // Из лабораторной работы №3
#include "sum_utils.h"    // Наша новая библиотека

int main(int argc, char **argv) {
    // Переменные для аргументов командной строки
    int threads_num = -1;
    int seed = -1;
    int array_size = -1;
    
    // Парсинг аргументов командной строки
    while (1) {
        int current_optind = optind ? optind : 1;
        
        static struct option options[] = {
            {"threads_num", required_argument, 0, 0},
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {0, 0, 0, 0}
        };
        
        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);
        
        if (c == -1) break;
        
        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        threads_num = atoi(optarg);
                        if (threads_num <= 0) {
                            printf("threads_num must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 1:
                        seed = atoi(optarg);
                        if (seed <= 0) {
                            printf("seed must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 2:
                        array_size = atoi(optarg);
                        if (array_size <= 0) {
                            printf("array_size must be a positive number\n");
                            return 1;
                        }
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case '?':
                printf("Unknown option\n");
                break;
            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }
    
    // Проверка обязательных аргументов
    if (threads_num == -1 || seed == -1 || array_size == -1) {
        printf("Usage: %s --threads_num \"num\" --seed \"num\" --array_size \"num\"\n",
               argv[0]);
        return 1;
    }
    
    printf("=== Параллельное вычисление суммы массива ===\n");
    printf("Количество потоков: %d\n", threads_num);
    printf("Seed для генерации: %d\n", seed);
    printf("Размер массива: %d\n", array_size);
    
    // Выделяем память под массив
    int *array = malloc(sizeof(int) * array_size);
    if (array == NULL) {
        printf("Memory allocation failed for array\n");
        return 1;
    }
    
    // Генерируем массив (не входит в замер времени)
    printf("Генерация массива...\n");
    GenerateArray(array, array_size, seed);
    
    // Проверяем сумму последовательно (для верификации)
    long long sequential_sum = compute_total_sum(array, array_size);
    printf("Последовательная сумма: %lld\n", sequential_sum);
    
    // === НАЧАЛО ЗАМЕРА ВРЕМЕНИ ===
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    // Создаем потоки и структуры данных для них
    pthread_t threads[threads_num];
    struct thread_data thread_data_array[threads_num];
    
    // Вычисляем размер части массива для каждого потока
    int chunk_size = array_size / threads_num;
    int remainder = array_size % threads_num;
    
    printf("\nРаспределение работы между потоками:\n");
    
    // Создаем и запускаем потоки
    int current_start = 0;
    for (int i = 0; i < threads_num; i++) {
        // Вычисляем границы для текущего потока
        int current_end = current_start + chunk_size;
        
        // Распределяем остаток по первым потокам
        if (i < remainder) {
            current_end++;
        }
        
        // Заполняем структуру данных для потока
        thread_data_array[i].start_index = current_start;
        thread_data_array[i].end_index = current_end;
        thread_data_array[i].array = array;
        thread_data_array[i].partial_sum = 0;
        
        printf("Поток %d: индексы [%d-%d], элементов: %d\n", 
               i, current_start, current_end - 1, current_end - current_start);
        
        // Создаем поток
        if (pthread_create(&threads[i], NULL, compute_partial_sum, 
                          &thread_data_array[i]) != 0) {
            perror("pthread_create failed");
            free(array);
            return 1;
        }
        
        current_start = current_end;
    }
    
    // Ожидаем завершения всех потоков
    long long parallel_sum = 0;
    for (int i = 0; i < threads_num; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join failed");
            free(array);
            return 1;
        }
        
        // Суммируем частичные суммы
        parallel_sum += thread_data_array[i].partial_sum;
        printf("Поток %d завершился, частичная сумма: %lld\n", 
               i, thread_data_array[i].partial_sum);
    }
    
    // === КОНЕЦ ЗАМЕРА ВРЕМЕНИ ===
    gettimeofday(&end_time, NULL);
    
    // Вычисляем время выполнения
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (end_time.tv_usec - start_time.tv_usec) / 1000.0;
    
    // Выводим результаты
    printf("\n=== РЕЗУЛЬТАТЫ ===\n");
    printf("Параллельная сумма: %lld\n", parallel_sum);
    printf("Последовательная сумма: %lld\n", sequential_sum);
    printf("Совпадение результатов: %s\n", 
           (parallel_sum == sequential_sum) ? "ДА ✓" : "НЕТ ✗");
    printf("Время вычисления: %.3f мс\n", elapsed_time);
    
    // Освобождаем память
    free(array);
    
    return 0;
}
