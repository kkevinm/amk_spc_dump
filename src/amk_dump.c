#include "amk_dump.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "rom.h"

#define AMK_NAME_ADDR              (0x0E8000)
#define AMK_NAME_STRING            "@AMK"
#define AMK_SAMPLE_GROUPS_PTR_ADDR (AMK_NAME_ADDR + 4 + 1)
#define AMK_MUSIC_PTR_ADDR         (AMK_SAMPLE_GROUPS_PTR_ADDR + 3)

typedef struct
{
    buffer_t *data;
    uint16_t loop_point;
} amk_sample_t;

typedef struct
{
    amk_sample_t **samples_ptr;
    uint8_t samples_num;
    buffer_t *music_data;
} amk_music_t;

void amk_dump(rom_t *rom,
              const char *rom_name)
{
    buffer_t *buffer;
    uint32_t sample_groups_ptr, music_ptr, samples_ptr, loops_ptr, tmp;
    int music_num, samples_num, i;
    amk_sample_t *samples;
    amk_music_t *songs;
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
    /* Sanity check */
    if ((music_num == 0)
        || (samples_num == 0))
    {
        return;
    }
    /* Build samples array */
    samples = (amk_sample_t *)malloc(sizeof(amk_sample_t) * samples_num);
    if (samples == NULL)
    {
        fprintf(stderr,
                "Memory allocation error\n");
        return;
    }
    for (i = 0; i < samples_num; i++)
    {
        uint32_t sample_table_ptr;
        //******************
        sample_table_ptr = rom_read_long(rom,
                                         samples_ptr + (i * 3));
        /* Check for invalid sample (although it never seems to happen...) */
        if ((sample_table_ptr & 0xFFFF) == 0)
        {
            samples[i].data = NULL;
            samples[i].loop_point = 0;
        }
        else
        {
            uint16_t sample_len;
            //******************
            sample_len = rom_read_word(rom,
                                       sample_table_ptr);
            samples[i].data = rom_read_buffer(rom,
                                              sample_table_ptr + 2,
                                                (uint32_t)sample_len);
            samples[i].loop_point = rom_read_word(rom,
                                                  loops_ptr + (i * 2));
        }
    }
    /* Build songs array */
    songs = (amk_music_t *)malloc(sizeof(amk_music_t) * music_num);
    if (songs == NULL)
    {
        fprintf(stderr,
                "Memory allocation error\n");
        return;
    }
    for (i = 0; i < music_num; i++)
    {
        uint32_t sample_group_ptr;
        //******************
        sample_group_ptr = (uint32_t)rom_read_word(rom,
                                                   sample_groups_ptr + (i * 2));
        /* Check song with no samples (should just be song 0) */
        if (sample_group_ptr == 0)
        {
            
        }
        else
        {
            int j;
            //******************
            /* Add bank byte to sample group pointer */
            sample_group_ptr |= (sample_groups_ptr & 0xFF0000);
            songs[i].samples_num = rom_read_byte(rom,
                                                 sample_group_ptr);
            songs[i].samples_ptr = (amk_sample_t **)malloc(sizeof(amk_sample_t *) * songs[i].samples_num);
            for (j = 0; j < songs[i].samples_num; j++)
            {
                
            }
        }
    }
    /* Destroy everything */
    for (i = 0; i < samples_num; i++)
    {
        free(samples[i].data);
    }
    free(samples);
    for (i = 0; i < music_num; i++)
    {
        free(songs[i].samples_ptr);
        free(songs[i].music_data);
    }
    free(songs);
}
