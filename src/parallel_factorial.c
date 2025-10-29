#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <getopt.h>

// Глобальные переменные для разделения данных между потоками
long long result = 1;        // Результат (факториал по модулю)
pthread_mutex_t mutex;       // Мьютекс для синхронизации
int mod_value;               // Модуль
int k_value;                 // Число для факториала

// Структура для передачи данных в поток
struct thread_data {
    int start;               // Начальное число для потока
    int end;                 // Конечное число для потока
};

// Функция, которую выполняет каждый поток
void* calculate_partial_factorial(void* arg) {
    struct thread_data* data = (struct thread_data*)arg;
    long long partial_result = 1;
    
    printf("Поток: вычисляю факториал от %d до %d\n", data->start, data->end);
    
    // Вычисляем частичный факториал
    for (int i = data->start; i <= data->end; i++) {
        partial_result = (partial_result * i) % mod_value;
    }
    
    printf("Поток: частичный результат = %lld\n", partial_result);
    
    // Синхронизированно умножаем на общий результат
    pthread_mutex_lock(&mutex);
    result = (result * partial_result) % mod_value;
    pthread_mutex_unlock(&mutex);
    
    free(data);  // Освобождаем память
    return NULL;
}

int main(int argc, char* argv[]) {
    int pnum = 1;  // Количество потоков по умолчанию
    k_value = 1;   // k по умолчанию
    mod_value = 1; // mod по умолчанию
    
    // Парсинг аргументов командной строки
    static struct option long_options[] = {
        {"k", required_argument, 0, 'k'},
        {"pnum", required_argument, 0, 'p'},
        {"mod", required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    while ((c = getopt_long(argc, argv, "k:p:m:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'k':
                k_value = atoi(optarg);
                if (k_value < 0) {
                    printf("Ошибка: k должно быть неотрицательным\n");
                    return 1;
                }
                break;
            case 'p':
                pnum = atoi(optarg);
                if (pnum <= 0) {
                    printf("Ошибка: pnum должно быть положительным\n");
                    return 1;
                }
                break;
            case 'm':
                mod_value = atoi(optarg);
                if (mod_value <= 0) {
                    printf("Ошибка: mod должно быть положительным\n");
                    return 1;
                }
                break;
            default:
                printf("Использование: %s -k <число> --pnum=<потоки> --mod=<модуль>\n", argv[0]);
                return 1;
        }
    }
    
    // Проверка обязательных параметров
    if (k_value == 1 || mod_value == 1) {
        printf("Использование: %s -k <число> --pnum=<потоки> --mod=<модуль>\n", argv[0]);
        printf("Пример: %s -k 10 --pnum=4 --mod=1000000007\n", argv[0]);
        return 1;
    }
    
    // Особые случаи для факториала
    if (k_value == 0) {
        printf("0! mod %d = 1\n", mod_value);
        return 0;
    }
    
    printf("=== Параллельное вычисление факториала ===\n");
    printf("Вычисляем: %d! mod %d\n", k_value, mod_value);
    printf("Количество потоков: %d\n", pnum);
    
    // Инициализация мьютекса
    pthread_mutex_init(&mutex, NULL);
    
    // Создание потоков
    pthread_t threads[pnum];
    int numbers_per_thread = k_value / pnum;
    int remainder = k_value % pnum;
    
    int current_start = 1;
    int threads_created = 0;
    
    // Распределяем работу между потоками
    for (int i = 0; i < pnum; i++) {
        int current_end = current_start + numbers_per_thread - 1;
        
        // Распределяем остаток по первым потокам
        if (i < remainder) {
            current_end++;
        }
        
        // Если текущий поток будет обрабатывать числа
        if (current_start <= k_value) {
            struct thread_data* data = malloc(sizeof(struct thread_data));
            data->start = current_start;
            data->end = current_end;
            
            printf("Создаю поток %d для чисел от %d до %d\n", 
                   i, data->start, data->end);
            
            if (pthread_create(&threads[threads_created], NULL, 
                              calculate_partial_factorial, data) != 0) {
                perror("Ошибка создания потока");
                free(data);
                continue;
            }
            threads_created++;
        }
        
        current_start = current_end + 1;
    }
    
    // Ожидаем завершения всех потоков
    for (int i = 0; i < threads_created; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Уничтожаем мьютекс
    pthread_mutex_destroy(&mutex);
    
    // Выводим результат
    printf("\n=== РЕЗУЛЬТАТ ===\n");
    printf("%d! mod %d = %lld\n", k_value, mod_value, result);
    
    return 0;
}
