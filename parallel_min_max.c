#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
                printf("seed must be a positive number\n");
                return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
                printf("array_size must be a positive number\n");
                return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
                printf("pnum must be a positive number\n");
                return 1;
            }
            break;
          case 3:
            with_files = true;
            break;

          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--by_files]\n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  
  // Создаем пайпы для каждого дочернего процесса
  int pipes[pnum][2];
  if (!with_files) {
    for (int i = 0; i < pnum; i++) {
      if (pipe(pipes[i]) == -1) {
        printf("Pipe creation failed!\n");
        free(array);
        return 1;
      }
    }
  }

  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  // Вычисляем размер части массива для каждого процесса
  int chunk_size = array_size / pnum;

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      
      if (child_pid == 0) {
        // child process
        
        // Вычисляем границы для текущего процесса
        int start = i * chunk_size;
        int end = (i == pnum - 1) ? array_size : start + chunk_size;
        
        // Находим min и max в своей части массива
        struct MinMax local_min_max = GetMinMax(array, start, end);

        if (with_files) {
          // use files here
          char filename_min[32], filename_max[32];
          sprintf(filename_min, "min_%d.txt", i);
          sprintf(filename_max, "max_%d.txt", i);
          
          // Записываем min в файл
          FILE *file_min = fopen(filename_min, "w");
          if (file_min != NULL) {
            fprintf(file_min, "%d", local_min_max.min);
            fclose(file_min);
          }
          
          // Записываем max в файл
          FILE *file_max = fopen(filename_max, "w");
          if (file_max != NULL) {
            fprintf(file_max, "%d", local_min_max.max);
            fclose(file_max);
          }
        } else {
          // use pipe here
          close(pipes[i][0]); // закрываем чтение в дочернем процессе
          
          // Отправляем min и max через pipe
          write(pipes[i][1], &local_min_max.min, sizeof(int));
          write(pipes[i][1], &local_min_max.max, sizeof(int));
          
          close(pipes[i][1]); // закрываем запись
        }
        free(array);
        exit(0);
      }

    } else {
      printf("Fork failed!\n");
      free(array);
      return 1;
    }
  }

  // Родительский процесс ожидает завершения всех дочерних процессов
  while (active_child_processes > 0) {
    wait(NULL);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // read from files
      char filename_min[32], filename_max[32];
      sprintf(filename_min, "min_%d.txt", i);
      sprintf(filename_max, "max_%d.txt", i);
      
      // Читаем min из файла
      FILE *file_min = fopen(filename_min, "r");
      if (file_min != NULL) {
        fscanf(file_min, "%d", &min);
        fclose(file_min);
        remove(filename_min); // удаляем временный файл
      }
      
      // Читаем max из файла
      FILE *file_max = fopen(filename_max, "r");
      if (file_max != NULL) {
        fscanf(file_max, "%d", &max);
        fclose(file_max);
        remove(filename_max); // удаляем временный файл
      }
    } else {
      // read from pipes
      close(pipes[i][1]); // закрываем запись в родительском процессе
      
      // Читаем min и max из pipe
      read(pipes[i][0], &min, sizeof(int));
      read(pipes[i][0], &max, sizeof(int));
      
      close(pipes[i][0]); // закрываем чтение
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  printf("Array size: %d, Processes: %d, Sync method: %s\n", 
         array_size, pnum, with_files ? "files" : "pipes");
  fflush(NULL);
  return 0;
}
