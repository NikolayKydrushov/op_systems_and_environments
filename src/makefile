# Компилятор и флаги компиляции
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = 

# Целевые программы (исполняемые файлы)
TARGETS = parallel_min_max process_memory

# Правило по умолчанию - собирает все цели
all: $(TARGETS)

# Правило для сборки parallel_min_max
parallel_min_max: parallel_min_max.o find_min_max.o utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

# Правило для сборки process_memory
process_memory: process_memory.o
	$(CC) -o $@ $^ $(LDFLAGS)

# Правило для компиляции object файлов из C файлов
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Зависимости для object файлов
parallel_min_max.o: parallel_min_max.c find_min_max.h utils.h
find_min_max.o: find_min_max.c find_min_max.h
utils.o: utils.c utils.h
process_memory.o: process_memory.c

# Очистка собранных файлов
clean:
	rm -f $(TARGETS) *.o

# Пересборка
rebuild: clean all

# Помощь
help:
	@echo "Доступные цели:"
	@echo "  all     - собрать все программы (по умолчанию)"
	@echo "  clean   - удалить все собранные файлы"
	@echo "  rebuild - пересобрать все программы"
	@echo "  help    - показать эту справку"

# Объявляем phony targets (цели, которые не являются файлами)
.PHONY: all clean rebuild help
