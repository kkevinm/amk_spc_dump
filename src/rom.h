#ifndef ROM_H_
#define ROM_H_

#include "utils.h"

#define INVALID_ADDRESS (0xFFFFFFFF)

typedef enum
{
    MAPPER_INVALID,
    MAPPER_LOROM,
    MAPPER_HIROM,
    MAPPER_SA1ROM,
    MAPPER_FULLSA1ROM,
    MAPPER_SFXROM,
    MAPPER_EXLOROM,
    MAPPER_EXHIROM,
    MAPPER_NOROM
} mapper_t;

typedef struct
{
    buffer_t *buffer;
    mapper_t mapper;
} rom_t;

uint32_t snes_to_pc(uint32_t addr,
                    mapper_t mapper);

uint32_t pc_to_snes(uint32_t addr,
                    mapper_t mapper);

rom_t *rom_create_from_buffer(buffer_t *buffer);

rom_t *rom_create_from_file(const char *path);

void rom_destroy(rom_t *rom);

uint8_t rom_read_byte(const rom_t *rom,
                      uint32_t snes_addr);

uint16_t rom_read_word(const rom_t *rom,
                       uint32_t snes_addr);

uint32_t rom_read_long(const rom_t *rom,
                       uint32_t snes_addr);

buffer_t *rom_read_buffer(const rom_t *rom,
                          uint32_t snes_addr,
                          size_t size);

#endif
