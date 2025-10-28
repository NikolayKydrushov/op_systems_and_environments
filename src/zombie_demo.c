#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main() {
    printf("=== Демонстрация зомби-процессов ===\n");
    printf("Родительский процесс PID: %d\n", getpid());
    
    pid_t child_pid = fork();
    
    if (child_pid == -1) {
        perror("fork failed");
        return 1;
    }
    else if (child_pid == 0) {
        // Дочерний процесс
        printf("Дочерний процесс PID: %d (родитель: %d)\n", getpid(), getppid());
        printf("Дочерний процесс завершается через 5 секунд...\n");
        sleep(5);
        printf("Дочерний процесс завершился, но станет зомби!\n");
        exit(0);
    }
    else {
        // Родительский процесс
        printf("Родитель создал дочерний процесс с PID: %d\n", child_pid);
        printf("Родитель НЕ вызывает wait(), поэтому дочерний процесс станет зомби\n");
        printf("Родитель спит 30 секунд...\n");
        
        // Родитель спит, не вызывая wait() - дочерний процесс станет зомби
        sleep(30);
        
        printf("Родитель просыпается и завершается\n");
        printf("Когда родитель завершится, зомби-процесс будет убран\n");
    }
    
    return 0;
}
