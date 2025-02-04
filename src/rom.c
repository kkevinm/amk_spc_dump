#include "rom.h"
#include <stdlib.h>
#include <string.h>

#define ROM_MIN_SIZE (1 * 1024)
#define ROM_MAX_SIZE (8 * 1024 * 1024)

#define INTERNAL_HEADER_ROM_TYPE_OFFSET (0x7FD5)
#define INTERNAL_HEADER_EXTRA_HW_OFFSET (0x7FD6)
#define INTERNAL_HEADER_ROM_SIZE_OFFSET (0x7FD7)

static uint32_t sa1_banks[8] =
{
    0 << 20, 1 << 20, INVALID_ADDRESS, INVALID_ADDRESS, 2 << 20, 3 << 20, INVALID_ADDRESS, INVALID_ADDRESS
};

uint32_t snes_to_pc(uint32_t addr,
                    mapper_t mapper)
{
    if (addr > 0xFFFFFF)
    {
        return INVALID_ADDRESS;
    }
    switch (mapper)
    {
    case MAPPER_LOROM:
        if (((addr & 0xFE0000) == 0x7E0000)
            || ((addr & 0x408000) == 0x000000)
            || ((addr & 0x708000) == 0x700000))
        {
            return INVALID_ADDRESS;
        }
        else
        {
            return ((addr & 0x7F0000)>>1) | (addr & 0x7FFF);
        }
    case MAPPER_HIROM:
        if (((addr & 0xFE0000) == 0x7E0000)
            || ((addr & 0x408000) == 0x000000))
        {
            return INVALID_ADDRESS;
        }
        else
        {
            return addr & 0x3FFFFF;
        }
    case MAPPER_SA1ROM:
        if ((addr & 0x408000) == 0x008000)
        {
            return sa1_banks[(addr & 0xE00000) >> 21] | ((addr & 0x1F0000) >> 1) | (addr & 0x007FFF);
        }
        else if ((addr & 0xC00000) == 0xC00000)
        {
            return sa1_banks[((addr & 0x100000) >> 20) | ((addr & 0x200000) >> 19)] | (addr & 0x0FFFFF);
        }
        else
        {
            return INVALID_ADDRESS;
        }
        break;
    case MAPPER_FULLSA1ROM:
        if ((addr & 0xC00000) == 0xC00000)
        {
            return (addr & 0x3FFFFF) | 0x400000;
        }
        else if (((addr & 0xC00000) == 0x000000)
                 || ((addr & 0xC00000) == 0x800000))
        {
            if ((addr & 0x008000) == 0x000000)
            {
                return INVALID_ADDRESS;
            }
            else
            {
                return ((addr & 0x800000) >> 2) | ((addr & 0x3F0000) >> 1) | (addr & 0x7FFF);
            }
        }
        else
        {
            return INVALID_ADDRESS;
        }
    case MAPPER_SFXROM:
        if (((addr & 0x600000) == 0x600000)
            || ((addr & 0x408000) == 0x000000)
            || ((addr & 0x800000) == 0x800000))
        {
            return INVALID_ADDRESS;
        }
        else if (addr & 0x400000)
        {
            return addr & 0x3FFFFF;
        }
        else
        {
            return ((addr & 0x7F0000) >> 1) | (addr&0x7FFF);
        }
    case MAPPER_EXLOROM:
        if (((addr & 0xF00000) == 0x700000)
            || ((addr & 0x408000) == 0x000000))
        {
            return INVALID_ADDRESS;
        }
        else if (addr & 0x800000)
        {
            return ((addr & 0x7F0000) >> 1) | (addr & 0x7FFF);
        }
        else
        {
            return (((addr & 0x7F0000) >> 1) | (addr & 0x7FFF)) + 0x400000;
        }
    case MAPPER_EXHIROM:
        if (((addr & 0xFE0000) == 0x7E0000)
            || ((addr & 0x408000) == 0x000000))
        {
            return INVALID_ADDRESS;
        }
        else if ((addr & 0x800000) == 0x000000)
        {
            return (addr & 0x3FFFFF) | 0x400000;
        }
        else
        {
            return addr & 0x3FFFFF;
        }
    case MAPPER_NOROM:
        return addr;
    case MAPPER_INVALID:
    default:
        return INVALID_ADDRESS;
    }
}

uint32_t pc_to_snes(uint32_t addr,
                    mapper_t mapper)
{
    switch (mapper)
    {
    case MAPPER_LOROM:
        if (addr >= 0x400000)
        {
            return INVALID_ADDRESS;
        }
        else
        {
            return ((addr << 1) & 0x7F0000) | (addr & 0x7FFF) | 0x8000 | 0x800000;
        }
    case MAPPER_HIROM:
        if (addr >= 0x400000)
        {
            return INVALID_ADDRESS;
        }
        else
        {
            return addr | 0xC00000;
        }
    case MAPPER_SA1ROM:
        for (uint32_t i = 0; i < sizeof(sa1_banks) / sizeof(*sa1_banks); i++)
        {
            if (sa1_banks[i] == (addr & 0x700000))
            {
                return 0x008000 | (i<<21) | ((addr & 0x0F8000) << 1) | (addr & 0x7FFF);
            }
        }
        return INVALID_ADDRESS;
    case MAPPER_FULLSA1ROM:
        if (addr >= 0x800000)
        {
            return INVALID_ADDRESS;
        }
        else if ((addr & 0x400000) == 0x400000)
        {
            return addr | 0xC00000;
        }
        else if ((addr & 0x600000) == 0x000000)
        {
            return ((addr << 1) & 0x3F0000) | 0x8000 | (addr & 0x7FFF);
        }
        else if ((addr & 0x600000) == 0x200000)
        {
            return 0x800000 | ((addr << 1) & 0x3F0000) | 0x8000 | (addr&0x7FFF);
        }
        else
        {
            return INVALID_ADDRESS;
        }
    case MAPPER_SFXROM:
        if (addr >= 0x200000)
        {
            return INVALID_ADDRESS;
        }
        else
        {
            return ((addr << 1) & 0x7F0000) | (addr & 0x7FFF) | 0x8000;
        }
    case MAPPER_EXLOROM:
        if (addr >= 0x800000)
        {
            return INVALID_ADDRESS;
        }
        if (addr & 0x400000)
        {
            addr -= 0x400000;
            return ((addr << 1) & 0x7F0000) | (addr & 0x7FFF) | 0x8000;
        }
        else
        {
            return ((addr << 1) & 0x7F0000) | (addr & 0x7FFF) | 0x8000 | 0x800000;
        }
    case MAPPER_EXHIROM:
        if (addr >= 0x800000)
        {
            return INVALID_ADDRESS;
        }
        else if (addr & 0x400000)
        {
            return addr;
        }
        else
        {
            return addr | 0xC00000;
        }
    case MAPPER_NOROM:
        return addr;
    case MAPPER_INVALID:
    default:
        return INVALID_ADDRESS;
    }
}

static mapper_t rom_get_mapper(const buffer_t *buffer)
{
    size_t header_size;
    uint8_t rom_type, extra_hw, rom_size;
    //******************
    if ((buffer == NULL)
        || (buffer->data == NULL))
    {
        return MAPPER_INVALID;
    }
    header_size = buffer->size & 0x7FFF;
    if ((buffer->size - header_size < ROM_MIN_SIZE)
        || (buffer->size - header_size > ROM_MAX_SIZE))
    {
        return MAPPER_INVALID;
    }
    rom_type = buffer->data[header_size + INTERNAL_HEADER_ROM_TYPE_OFFSET] & 0x0F;
    extra_hw = buffer->data[header_size + INTERNAL_HEADER_EXTRA_HW_OFFSET] & 0xF0;
    rom_size = buffer->data[header_size + INTERNAL_HEADER_ROM_SIZE_OFFSET];
    if (extra_hw == 0x10)
    {
        return MAPPER_SFXROM;
    }
    switch (rom_type)
    {
    case 0:
        if (rom_size > 0x0C)
        {
            return MAPPER_EXLOROM;
        }
        else
        {
            return MAPPER_LOROM;
        }
    case 1:
        return MAPPER_HIROM;
    case 3:
        if (extra_hw != 0x30)
        {
            return MAPPER_INVALID;
        }
        else if (rom_size > 0x0C)
        {
            return MAPPER_FULLSA1ROM;
        }
        else
        {
            return MAPPER_SA1ROM;
        }
    case 5:
        return MAPPER_EXHIROM;
    default:
        return MAPPER_INVALID;
    }
}

rom_t *rom_create_from_buffer(buffer_t *buffer)
{
    rom_t *rom;
    //******************
    rom = (rom_t *)malloc(sizeof(rom_t));
    if (rom != NULL)
    {
        rom->buffer = buffer;
        rom->mapper = rom_get_mapper(rom->buffer);
    }
    return rom;
}

rom_t *rom_create_from_file(const char *path)
{
    return rom_create_from_buffer(file_open_and_read_all(path,
                                                         ROM_MAX_SIZE));
}

void rom_destroy(rom_t *rom)
{
    if (rom != NULL)
    {
        buffer_destroy(rom->buffer);
        free(rom);
    }
}

static uint32_t snes_to_pc_header(const rom_t *rom,
                                  uint32_t snes_addr)
{
    if (rom == NULL)
    {
        return INVALID_ADDRESS;
    }
    else
    {
        return (rom->buffer->size & 0x7FFF) + snes_to_pc(snes_addr,
                                                         rom->mapper);
    }
}

uint8_t rom_read_byte(const rom_t *rom,
                      uint32_t snes_addr)
{
    uint32_t pc_addr;
    //******************
    pc_addr = snes_to_pc_header(rom,
                                snes_addr);
    if ((pc_addr == INVALID_ADDRESS)
        || (pc_addr + 1 > rom->buffer->size))
    {
        return 0;
    }
    else
    {
        return rom->buffer->data[pc_addr];
    }
}

uint16_t rom_read_word(const rom_t *rom,
                       uint32_t snes_addr)
{
    uint32_t pc_addr;
    //******************
    pc_addr = snes_to_pc_header(rom,
                                snes_addr);
    if ((pc_addr == INVALID_ADDRESS)
        || (pc_addr + 2 > rom->buffer->size))
    {
        return 0;
    }
    else
    {
        return (uint16_t)rom->buffer->data[pc_addr]
               + ((uint16_t)rom->buffer->data[pc_addr + 1] << 8);
    }
}

uint32_t rom_read_long(const rom_t *rom,
                       uint32_t snes_addr)
{
    uint32_t pc_addr;
    //******************
    pc_addr = snes_to_pc_header(rom,
                                snes_addr);
    if ((pc_addr == INVALID_ADDRESS)
        || (pc_addr + 3 > rom->buffer->size))
    {
        return 0;
    }
    else
    {
        return (uint32_t)rom->buffer->data[pc_addr]
               + ((uint32_t)rom->buffer->data[pc_addr + 1] << 8)
               + ((uint32_t)rom->buffer->data[pc_addr + 2] << 16);
    }
}

buffer_t *rom_read_buffer(const rom_t *rom,
                          uint32_t snes_addr,
                          size_t size)
{
    uint32_t pc_addr;
    buffer_t *buffer;
    //******************
    pc_addr = snes_to_pc_header(rom,
                                snes_addr);
    if ((pc_addr == INVALID_ADDRESS)
        || (pc_addr + size > rom->buffer->size))
    {
        return buffer_create(0);
    }
    buffer = buffer_create(size);
    (void)memcpy(buffer->data,
                 rom->buffer->data + pc_addr,
                 size);
    buffer->size = size;
    return buffer;
}
