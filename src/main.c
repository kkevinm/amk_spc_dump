#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>
#include "rom.h"
#include "amk_dump.h"

int main(int argc,
         char *argv[])
{
    rom_t *rom;
    char *rom_name, *tmp;
    unsigned int yoshi_drums_enable;
    unsigned int global_songs_enable;
    unsigned int vanilla_songs_enable;
    uint16_t global_songs_number_override;
    //******************
    if (argc < 9)
    {
        fprintf(stderr,
                "Usage: amk_spc_dump <rom path> <output path> <song length in s> <fade length in ms> <Yoshi drums enable (0 or 1)> <global songs enable (0 or 1)> <vanilla songs enable (0 or 1)> <global songs number override (0 = don't override)>\n");
        return -1;
    }
    if (sscanf(argv[5],
               "%u",
               &yoshi_drums_enable) != 1)
    {
        fprintf(stderr,
                "Error: Yoshi drums enable flag has an invalid value\n");
        return -1;
    }
    if (sscanf(argv[6],
               "%u",
               &global_songs_enable) != 1)
    {
        fprintf(stderr,
                "Error: global songs enable flag has an invalid value\n");
        return -1;
    }
    if (sscanf(argv[7],
               "%u",
               &vanilla_songs_enable) != 1)
    {
        fprintf(stderr,
                "Error: vanilla songs enable flag has an invalid value\n");
        return -1;
    }
    if (sscanf(argv[8],
               "%hu",
               &global_songs_number_override) != 1)
    {
        fprintf(stderr,
                "Error: global songs number override has an invalid value\n");
        return -1;
    }
    rom = rom_create_from_file(argv[1]);
    if ((rom == NULL)
        || (rom->buffer == NULL)
        || (rom->buffer->data == NULL)
        || (rom->buffer->size == 0))
    {
        rom_destroy(rom);
        return -1;
    }
    if (rom->mapper == MAPPER_INVALID)
    {
        fprintf(stderr,
                "%s is not a valid SNES ROM\n",
                argv[1]);
        rom_destroy(rom);
        return -1;
    }
    rom_name = (char *)malloc(strlen(argv[1]));
    strcpy(rom_name,
           basename(argv[1]));
    tmp = strrchr(rom_name,
                  '.');
    if (tmp != NULL)
    {
        *tmp = '\0';
    }
    amk_dump(rom,
             rom_name,
             argv[2],
             argv[3],
             argv[4],
             yoshi_drums_enable != 0,
             global_songs_enable != 0,
             vanilla_songs_enable != 0,
             (uint8_t)global_songs_number_override);
    rom_destroy(rom);
    free(rom_name);
    return 0;
}
