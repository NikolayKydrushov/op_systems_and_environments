#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
void GenerateArray(int *array, int array_size, int seed) {
  srand(seed);
  for (int i = 0; i < array_size; i++) {
    array[i] = rand();
  }
}
