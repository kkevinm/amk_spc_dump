#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdint.h>

typedef struct
{
    uint8_t *data;
    size_t size;
} buffer_t;

buffer_t *buffer_create(size_t size);

buffer_t *buffer_copy(const buffer_t *buffer);

void buffer_destroy(buffer_t *buffer);

FILE *file_open(const char *path,
                const char *mode);

size_t file_size(FILE *file);

buffer_t *file_read_all(FILE *file);

void file_write_all(FILE *file,
                    const buffer_t *buffer);

buffer_t *file_open_and_read_all(const char *path);

void file_open_and_write_all(const char *path,
                             const buffer_t *buffer);

#endif
