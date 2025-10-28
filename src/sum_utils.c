#include <stdio.h>
#include "sum_utils.h"

// Функция, которую выполняет каждый поток
void* compute_partial_sum(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    data->partial_sum = 0;
    
    // Вычисляем сумму для своей части массива
    for (int i = data->start_index; i < data->end_index; i++) {
        data->partial_sum += data->array[i];
    }
    
    printf("Поток: сумма элементов [%d-%d] = %lld\n", 
           data->start_index, data->end_index - 1, data->partial_sum);
    
    return NULL;
}

// Последовательное вычисление суммы (для проверки)
long long compute_total_sum(int *array, int array_size) {
    long long total = 0;
    for (int i = 0; i < array_size; i++) {
        total += array[i];
    }
    return total;
}
