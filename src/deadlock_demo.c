#include <stdio.h>
#include <pthread.h>

pthread_mutex_t A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t B = PTHREAD_MUTEX_INITIALIZER;

void* thread1(void* arg) {
    pthread_mutex_lock(&A);
    printf("П1: взял A, жду B...\n");
    pthread_mutex_lock(&B); // Deadlock!
    printf("П1: взял B\n");
    pthread_mutex_unlock(&B);
    pthread_mutex_unlock(&A);
    return NULL;
}

void* thread2(void* arg) {
    pthread_mutex_lock(&B);
    printf("П2: взял B, жду A...\n");
    pthread_mutex_lock(&A); // Deadlock!
    printf("П2: взял A\n");
    pthread_mutex_unlock(&A);
    pthread_mutex_unlock(&B);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    
    printf("Простой deadlock:\n");
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);
    
    pthread_join(t1, NULL); // Бесконечное ожидание
    pthread_join(t2, NULL);
    
    return 0;
}
