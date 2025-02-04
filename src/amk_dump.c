#include "amk_dump.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "rom.h"
#include "spc.h"

#define DUMPER_NAME                "AMK Dumper v1.0"

#define AMK_NAME_ADDR              (0x0E8000)
#define AMK_NAME_STRING            "@AMK"
#define AMK_SAMPLE_GROUPS_PTR_ADDR (AMK_NAME_ADDR + 4 + 1)
#define AMK_MUSIC_PTR_ADDR         (AMK_SAMPLE_GROUPS_PTR_ADDR + 3)
#define AMK_SPC_ENGINE_ADDR        (0x0F8000)

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

static void amk_dump_spc(const amk_music_t *song,
                         const buffer_t *spc_engine,
                         const char *spc_file_path,
                         const char *game_name,
                         const char *song_length,
                         const char *fade_length)
{
    spc_t spc;
    buffer_t buffer;
    time_t t;
    struct tm time_handle;
    //******************
    spc_init(&spc);
    spc_set_game_title(&spc,
                       game_name);
    spc_set_dumper(&spc,
                   DUMPER_NAME);
    spc_set_song_length_s(&spc,
                          song_length);
    spc_set_fade_length_ms(&spc,
                           fade_length);
    t = time(NULL);
    time_handle = *localtime(&t);
    spc_set_dump_date(&spc,
                      time_handle.tm_mday,
                      time_handle.tm_mon + 1,
                      time_handle.tm_year + 1900);
    // TODO regs and ram
    buffer.data = (uint8_t *)&spc;
    buffer.size = sizeof(spc);
    file_open_and_write_all(spc_file_path,
                            &buffer);
}

void amk_dump(const rom_t *rom,
              const char *rom_name,
              const char *out_path,
              const char *default_song_length,
              const char *default_fade_length)
{
    buffer_t *buffer, *spc_engine;
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
        uint32_t music_data_ptr;
        uint32_t sample_group_ptr;
        //******************
        music_data_ptr = rom_read_long(rom,
                                       music_ptr + (i * 3));
        if (music_data_ptr == 0)
        {
            songs[i].music_data = NULL;
        }
        else
        {
            uint16_t music_len;
            //******************
            music_len = rom_read_word(rom,
                                      music_data_ptr);
            songs[i].music_data = rom_read_buffer(rom,
                                                  music_data_ptr + 2,
                                                  (uint32_t)music_len);
        }
        sample_group_ptr = (uint32_t)rom_read_word(rom,
                                                   sample_groups_ptr + (i * 2));
        if (sample_group_ptr == 0)
        {
            songs[i].samples_num = 0;
            songs[i].samples_ptr = NULL;
        }
        else
        {
            int j;
            //******************
            sample_group_ptr |= (sample_groups_ptr & 0xFF0000);
            songs[i].samples_num = rom_read_byte(rom,
                                                 sample_group_ptr);
            songs[i].samples_ptr = (amk_sample_t **)malloc(sizeof(amk_sample_t *) * songs[i].samples_num);
            for (j = 0; j < songs[i].samples_num; j++)
            {
                uint16_t curr_sample;
                //******************
                curr_sample = rom_read_word(rom,
                                            sample_group_ptr + 1 + (j * 2));
                if (curr_sample < samples_num)
                {
                    songs[i].samples_ptr[j] = &samples[i];
                }
                else
                {
                    songs[i].samples_ptr[j] = NULL;
                }
            }
        }
    }
    /* Get SPC engine data */
    spc_engine = rom_read_buffer(rom,
                                 AMK_SPC_ENGINE_ADDR + 2,
                                 rom_read_word(rom,
                                               AMK_SPC_ENGINE_ADDR));
    // TODO restore
    for (i = 10; i < 11; i++)
    //for (i = 0; i < music_num; i++)
    {
        char spc_file_path[500];
        //******************
        snprintf(spc_file_path,
                 sizeof(spc_file_path),
                 "%s/%s_%02X.spc",
                 out_path,
                 rom_name,
                 i);
        amk_dump_spc(&songs[i],
                     spc_engine,
                     spc_file_path,
                     rom_name,
                     default_song_length,
                     default_fade_length);
    }
    /* Destroy everything */
    free(buffer);
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
    free(spc_engine);
}
