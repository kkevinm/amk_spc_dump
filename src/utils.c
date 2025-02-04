#include "utils.h"
#include <stdlib.h>
#include <string.h>

buffer_t *buffer_create(size_t size)
{
    buffer_t *buffer;
    //******************
    buffer = (buffer_t *)malloc(sizeof(buffer_t));
    if (buffer != NULL)
    {
        buffer->data = (uint8_t *)malloc(size);
        if (buffer->data == NULL)
        {
            buffer->size = 0;
        }
        else
        {
            buffer->size = size;
        }
    }
    return buffer;
}

buffer_t *buffer_copy(const buffer_t *buffer)
{
    buffer_t *copy;
    //******************
    if (buffer == NULL)
    {
        return NULL;
    }
    copy = buffer_create(buffer->size);
    if (copy != NULL)
    {
        memcpy(copy->data,
               buffer->data,
               buffer->size);
    }
    return copy;
}

void buffer_destroy(buffer_t *buffer)
{
    if (buffer != NULL)
    {
        free(buffer->data);
        free(buffer);
    }
}

FILE *file_open(const char *path,
                const char *mode)
{
    FILE *file;
    //******************
    file = fopen(path,
                 mode);
    if (file == NULL)
    {
        fprintf(stderr,
                "Cannot open file %s\n",
                path);
    }
    return file;
}

size_t file_get_size(FILE *file)
{
    size_t size;
    long offset;
    //******************
    offset = ftell(file);
    fseek(file,
          0,
          SEEK_END);
    size = ftell(file);
    fseek(file,
          offset,
          SEEK_SET);
    return size;
}

buffer_t *file_read_all(FILE *file,
                        size_t max_size)
{
    buffer_t *buffer;
    size_t read_size;
    //******************
    read_size = file_get_size(file);
    if (read_size > max_size)
    {
        read_size = max_size;
    }
    buffer = buffer_create(read_size);
    if (buffer != NULL)
    {
        fread(buffer->data,
              1,
              buffer->size,
              file);
    }
    return buffer;
}

void file_write_all(FILE *file,
                    const buffer_t *buffer)
{
    if (buffer != NULL)
    {
        fwrite(buffer->data,
               1,
               buffer->size,
               file);
    }
}

buffer_t *file_open_and_read_all(const char *path,
                                 size_t max_size)
{
    FILE *file;
    buffer_t *buffer;
    //******************
    file = file_open(path,
                     "r");
    buffer = file_read_all(file,
                           max_size);
    fclose(file);
    return buffer;
}

void file_open_and_write_all(const char *path,
                             const buffer_t *buffer)
{
    FILE *file;
    //******************
    file = file_open(path,
                     "w");
    file_write_all(file,
                   buffer);
    fclose(file);
}
