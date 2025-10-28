#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    // Аргументы по умолчанию
    char *seed = "123";
    char *array_size = "100000";
    
    // Если переданы аргументы в лаунчер, используем их
    if (argc == 3) {
        seed = argv[1];
        array_size = argv[2];
    } else if (argc > 1) {
        printf("Usage: %s [seed array_size]\n", argv[0]);
        printf("Using default values: seed=123, array_size=100000\n");
    }
    
    printf("Launcher: Starting sequential_min_max with seed=%s, array_size=%s\n", 
           seed, array_size);
    
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork failed");
        return 1;
    }
    else if (pid == 0) {
        // Дочерний процесс
        
        // Подготавливаем аргументы для sequential_min_max
        char *args[] = {
            "./sequential_min_max",  // argv[0]
            seed,                    // argv[1] 
            array_size,              // argv[2]
            NULL                     // NULL terminator
        };
        
        printf("Launcher: Executing: ");
        for (int i = 0; args[i] != NULL; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
        
        // Запускаем программу
        execvp("./sequential_min_max", args);
        
        // Если дошли сюда - ошибка
        perror("execvp failed");
        return 1;
    }
    else {
        // Родительский процесс
        printf("Launcher: Created child process with PID: %d\n", pid);
        
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            printf("Launcher: Program execution ");
            
            if (exit_status == 0) {
                printf("completed successfully!\n");
            } else {
                printf("failed with exit code: %d\n", exit_status);
            }
        } else if (WIFSIGNALED(status)) {
            printf("Launcher: Program terminated by signal: %d\n", WTERMSIG(status));
        } else {
            printf("Launcher: Program terminated abnormally\n");
        }
    }
    
    return 0;
}
