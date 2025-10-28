#ifndef FIND_MIN_MAX_H
#define FIND_MIN_MAX_H

struct MinMax {
  int min;
  int max;
};

struct MinMax GetMinMax(int *array, int start, int end);

#endif
