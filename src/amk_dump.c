#include "amk_dump.h"
#include <string.h>
#include <stdbool.h>
#include "rom.h"

#define AMK_NAME_ADDR              (0x0E8000)
#define AMK_NAME_STRING            "@AMK"
#define AMK_SAMPLE_GROUPS_PTR_ADDR (AMK_NAME_ADDR + 4 + 1)
#define AMK_MUSIC_PTR_ADDR         (AMK_SAMPLE_GROUPS_PTR_ADDR + 3)

void amk_dump(rom_t *rom,
              const char *rom_name)
{
    buffer_t *buffer;
    uint32_t sample_groups_ptr, music_ptr, samples_ptr, loops_ptr, tmp;
    int music_num, samples_num;
    //******************
    if (rom == NULL)
    {
        return;
    }
    buffer = rom_read_buffer(rom,
                             AMK_NAME_ADDR,
                             sizeof(AMK_NAME_STRING) - 1);
    if (memcmp(buffer->data,
               AMK_NAME_STRING,
               buffer->size) != 0)
    {
        fprintf(stderr,
                "Could not find AMK in the ROM\n");
        return;
    }
    /* Get sample groups pointer */
    sample_groups_ptr = rom_read_long(rom,
                                      AMK_SAMPLE_GROUPS_PTR_ADDR);
    if (sample_groups_ptr == 0)
    {
        fprintf(stderr,
                "Invalid sample groups pointer at $%06X\n",
                AMK_SAMPLE_GROUPS_PTR_ADDR);
        return;
    }
    /* Get music pointer */
    music_ptr = rom_read_long(rom,
                              AMK_MUSIC_PTR_ADDR);
    if (music_ptr == 0)
    {
        fprintf(stderr,
                "Invalid songs pointer at $%06X\n",
                AMK_MUSIC_PTR_ADDR);
        return;
    }
    /* Find songs number and samples pointer */
    music_num = -1;
    samples_ptr = music_ptr;
    do
    {
        tmp = rom_read_long(rom,
                            samples_ptr);
        music_num++;
        samples_ptr += 3;
    } while (tmp != 0xFFFFFF);
    samples_ptr += 3;
    /* Find samples number and sample loops pointer */
    samples_num = -1;
    loops_ptr = samples_ptr;
    do
    {
        tmp = rom_read_long(rom,
                            loops_ptr);
        samples_num++;
        loops_ptr += 3;
    } while (tmp != 0xFFFFFF);
    loops_ptr += 3;

    printf("Song pointers table located at $%06X with %d songs\n",
           music_ptr,
           music_num - 1);
    printf("Sample pointers table located at $%06X with %d samples\n",
           samples_ptr,
           samples_num);
    printf("Sample loops table located at $%06X\n",
           loops_ptr);
    printf("Sample groups table located at $%06X\n",
           sample_groups_ptr);
}
