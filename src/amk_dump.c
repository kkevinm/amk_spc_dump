#include "amk_dump.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "rom.h"
#include "spc.h"

#define DUMPER_NAME                         "AMK Dumper v1.0"

#define AMK_NAME_ADDR                       (0x0E8000)
#define AMK_NAME_STRING                     "@AMK"
#define AMK_SAMPLE_GROUPS_PTR_ADDR          (AMK_NAME_ADDR + 4 + 1)
#define AMK_MUSIC_PTR_ADDR                  (AMK_SAMPLE_GROUPS_PTR_ADDR + 3)
#define AMK_SPC_ENGINE_ADDR                 (0x0F8000)

#define AMK_SPC_MAIN_LOOP_RAM_POS           (0x434)

#define AMK_SPC_DEFAULT_SP_LOW              (0xCF)

#define AMK_SPC_RAM_MISC_FLAGS_MIRROR_ADDR   (0x5F)
#define AMK_SPC_DEFAULT_MISC_FLAGS_MIRROR    (0x20)
#define AMK_SPC_RAM_YOSHI_DRUMS_COMMAND_ADDR (0xF5)
#define AMK_SPC_ENABLE_YOSHI_DRUMS           (0x02)
#define AMK_SPC_RAM_SONG_NUMBER_ADDR         (0xF6)
#define AMK_SPC_RAM_TEMPO_ADDR               (0x51)
#define AMK_SPC_DEFAULT_TEMPO                (0x36)
#define AMK_SPC_RAM_REGISTERS_ADDR           (0xF0)

#define AMK_SPC_DEFAULT_LEFT_MASTER_VOLUME  (0x7F)
#define AMK_SPC_DEFAULT_RIGHT_MASTER_VOLUME (0x7F)
#define AMK_SPC_DEFAULT_ECHO_FEEDBACK       (0x60)
#define AMK_SPC_DEFAULT_MISC_FLAGS          (0x2F)
#define AMK_SPC_DEFAULT_ECHO_BUFFER_ADDRESS (0x60)

const uint8_t amk_spc_default_ram_registers[] =
{
    0xFF, 0x31, 0x4C, 0x0F, 0x00, 0x00, 0x0A, 0x00, 0xFF, 0xFF, 0x10, 0xFF, 0xFF, 0x00, 0xFF, 0xFF
};

const uint8_t amk_spc_main_loop_pos_bytes[] =
{
    0xEB, 0xFD, 0xF0, 0xFC
};

typedef struct
{
    buffer_t *data;
    uint16_t start_address;
} amk_spc_engine_t;

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
    uint16_t start_address;
} amk_music_t;

static uint16_t amk_get_spc_main_loop_pos(const spc_t *spc,
                                          uint16_t start_pos)
{
    int i;
    uint16_t bytes_found_pos;
    //******************
    bytes_found_pos = 0;
    /*
     * Look for the main loop starting bytes... if not found after a while,
     * just use the default position and hope for the best
     */
    for (i = 0; i < 100; i++)
    {
        if (spc->ram[start_pos + i] == amk_spc_main_loop_pos_bytes[bytes_found_pos])
        {
            bytes_found_pos++;
            if (bytes_found_pos >= sizeof(amk_spc_main_loop_pos_bytes))
            {
                return start_pos + i - sizeof(amk_spc_main_loop_pos_bytes) + 1;
            }
        }
        else
        {
            bytes_found_pos = 0;
        }
    }
    return AMK_SPC_MAIN_LOOP_RAM_POS;
}

static void amk_dump_spc(const amk_music_t *song,
                         const amk_spc_engine_t *spc_engine,
                         const char *spc_file_path,
                         const char *game_name,
                         const char *song_length,
                         const char *fade_length,
                         bool yoshi_drums_enable)
{
    spc_t spc;
    buffer_t buffer;
    time_t t;
    struct tm time_handle;
    uint16_t spc_ram_index, spc_sample_index, spc_main_loop_index;
    uint8_t i;
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
    /* Copy the SPC engine */
    spc_ram_index = spc_engine->start_address;
    memcpy(&spc.ram[spc_ram_index],
           spc_engine->data->data,
           spc_engine->data->size);
    /* Copy the music data */
    if (song->music_data != NULL)
    {
        spc_ram_index = song->start_address;
        memcpy(&spc.ram[spc_ram_index],
               song->music_data->data,
               song->music_data->size);
        spc_ram_index += (uint16_t)song->music_data->size;
    }
    else
    {
        spc_ram_index += (uint16_t)spc_engine->data->size;
    }
    /* Make the index aligned to 0x100 for the samples table */
    if ((spc_ram_index & 0xFF) != 0)
    {
        spc_ram_index = (spc_ram_index & 0xFF00) + 0x100;
    }
    /* Make space for the sample table before the samples data */
    spc_sample_index = spc_ram_index + (4 * (uint16_t)song->samples_num);
    /* Set the sample table address in the DSP */
    spc.dsp.sample_table_address = spc_ram_index >> 8;
    /* Copy the sample data and setup the samples table */
    for (i = 0; i < song->samples_num; i++)
    {
        bool dup_sample;
        uint8_t j;
        //******************
        dup_sample = false;
        for (j = 0; j < i; j++)
        {
            /* If the sample is duplicated, copy the table data but not the sample data */
            if (song->samples_ptr[j] == song->samples_ptr[i])
            {
                uint16_t dup_spc_sample_index;
                //******************
                dup_spc_sample_index = spc_ram_index + ((j - i) * 4);
                spc.ram[spc_ram_index + 0] = spc.ram[dup_spc_sample_index + 0];
                spc.ram[spc_ram_index + 1] = spc.ram[dup_spc_sample_index + 1];
                spc.ram[spc_ram_index + 2] = spc.ram[dup_spc_sample_index + 2];
                spc.ram[spc_ram_index + 3] = spc.ram[dup_spc_sample_index + 3];
                dup_sample = true;
                break;
            }
        }
        if (!dup_sample)
        {
            uint16_t loop_point;
            //******************
            loop_point = spc_sample_index + song->samples_ptr[i]->loop_point;
            spc.ram[spc_ram_index + 0] = (uint8_t)spc_sample_index;
            spc.ram[spc_ram_index + 1] = (uint8_t)(spc_sample_index >> 8);
            spc.ram[spc_ram_index + 2] = (uint8_t)loop_point;
            spc.ram[spc_ram_index + 3] = (uint8_t)(loop_point >> 8);
            memcpy(&spc.ram[spc_sample_index],
                   song->samples_ptr[i]->data->data,
                   song->samples_ptr[i]->data->size);
            spc_sample_index += song->samples_ptr[i]->data->size;
        }
        spc_ram_index += 4;
    }
    /* Set some DSP registers */
    spc.dsp.left_master_volume = AMK_SPC_DEFAULT_LEFT_MASTER_VOLUME;
    spc.dsp.echo_feedback = AMK_SPC_DEFAULT_ECHO_FEEDBACK;
    spc.dsp.right_master_volume = AMK_SPC_DEFAULT_RIGHT_MASTER_VOLUME;
    spc.dsp.misc_flags = AMK_SPC_DEFAULT_MISC_FLAGS;
    spc.dsp.echo_buffer_address = AMK_SPC_DEFAULT_ECHO_BUFFER_ADDRESS;
    /* Set the default tempo */
    spc.ram[AMK_SPC_RAM_TEMPO_ADDR] = AMK_SPC_DEFAULT_TEMPO;
    /*
     * This should be the DSP flags mirror in ram but AMK sets it to a
     * different value than spc.dsp.misc_flags... I won't question it
     */
    spc.ram[AMK_SPC_RAM_MISC_FLAGS_MIRROR_ADDR] = AMK_SPC_DEFAULT_MISC_FLAGS_MIRROR;
    /* Set the default ram registers */
    memcpy(&spc.ram[AMK_SPC_RAM_REGISTERS_ADDR],
           amk_spc_default_ram_registers,
           sizeof(amk_spc_default_ram_registers));
    /* Tell the engine to play this song (TODO: don't use hardcoded value) */
    spc.ram[AMK_SPC_RAM_SONG_NUMBER_ADDR] = 0xA;
    /* Enable Yoshi drums if enabled */
    if (yoshi_drums_enable)
    {
        spc.ram[AMK_SPC_RAM_YOSHI_DRUMS_COMMAND_ADDR] = AMK_SPC_ENABLE_YOSHI_DRUMS;
    }
    /* Set the initial SPC PC */
    spc_main_loop_index = amk_get_spc_main_loop_pos(&spc,
                                                    spc_engine->start_address);
    ((uint8_t *)&spc.regs.pc)[0] = (uint8_t)spc_main_loop_index;
    ((uint8_t *)&spc.regs.pc)[1] = (uint8_t)(spc_main_loop_index >> 8);
    /* Set the initial SPC SP */
    spc.regs.sp_low = AMK_SPC_DEFAULT_SP_LOW;
    /* Write the SPC data to the file */
    buffer.data = (uint8_t *)&spc;
    buffer.size = sizeof(spc);
    file_open_and_write_all(spc_file_path,
                            &buffer);
}

void amk_dump(const rom_t *rom,
              const char *rom_name,
              const char *out_path,
              const char *default_song_length,
              const char *default_fade_length,
              bool yoshi_drums_enable)
{
    buffer_t *buffer;
    uint32_t sample_groups_ptr, music_ptr, samples_ptr, loops_ptr, tmp;
    int music_num, samples_num, i;
    amk_spc_engine_t spc_engine;
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
            songs[i].start_address = rom_read_word(rom,
                                                   music_data_ptr + 2);
            songs[i].music_data = rom_read_buffer(rom,
                                                  music_data_ptr + 4,
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
                    songs[i].samples_ptr[j] = &samples[curr_sample];
                }
                else
                {
                    songs[i].samples_ptr[j] = NULL;
                }
            }
        }
    }
    /* Get SPC engine data */
    spc_engine.start_address = rom_read_word(rom,
                                             AMK_SPC_ENGINE_ADDR + 2);
    spc_engine.data = rom_read_buffer(rom,
                                      AMK_SPC_ENGINE_ADDR + 4,
                                      rom_read_word(rom,
                                                    AMK_SPC_ENGINE_ADDR));
    // TODO restore
    for (i = 10; i < 11; i++)
    //for (i = 1; i < music_num; i++)
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
                     &spc_engine,
                     spc_file_path,
                     rom_name,
                     default_song_length,
                     default_fade_length,
                     yoshi_drums_enable);
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
    free(spc_engine.data);
}
