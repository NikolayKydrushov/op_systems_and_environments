#ifndef SUM_UTILS_H
#define SUM_UTILS_H

// Структура для передачи данных в поток
struct thread_data {
    int start_index;      // Начальный индекс части массива
    int end_index;        // Конечный индекс части массива  
    int *array;           // Указатель на массив
    long long partial_sum;// Частичная сумма
};

// Функция для вычисления суммы части массива
void* compute_partial_sum(void *arg);

// Функция для вычисления суммы всего массива (последовательно)
long long compute_total_sum(int *array, int array_size);

#endif
