#include "spc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPC_HEADER_STRING "SNES-SPC700 Sound File Data v0.30"
#define SPC_HEADER_BYTE   (26)
#define SPC_VERSION_MINOR (31)

spc_t *spc_create(void)
{
    return (spc_t *)malloc(sizeof(spc_t));
}

void spc_destroy(spc_t *spc)
{
    free(spc);
}

void spc_init(spc_t *spc)
{
    if (spc != NULL)
    {
        memset(spc,
               0,
               sizeof(*spc));
        memcpy(spc->header_string,
               SPC_HEADER_STRING,
               sizeof(spc->header_string));
        spc->header_bytes[0] = SPC_HEADER_BYTE;
        spc->header_bytes[1] = SPC_HEADER_BYTE;
        spc->has_id666 = SPC_ID666_ENABLED;
        spc->version_minor = SPC_VERSION_MINOR;
        spc->default_channel_disables = SPC_DEFAULT_CHANNEL_ENABLED;
        spc->dump_emulator = SPC_DUMP_EMULATOR_UNKNOWN;
    }
}

void spc_set_song_title(spc_t *spc,
                        const char *song_title)
{
    if ((spc != NULL)
        && (song_title != NULL))
    {
        memcpy(spc->song_title,
               song_title,
               sizeof(spc->song_title));
    }
}

void spc_set_game_title(spc_t *spc,
                        const char *game_title)
{
    if ((spc != NULL)
        && (game_title != NULL))
    {
        memcpy(spc->game_title,
               game_title,
               sizeof(spc->game_title));
    }
}

void spc_set_dumper(spc_t *spc,
                    const char *dumper)
{
    if ((spc != NULL)
        && (dumper != NULL))
    {
        memcpy(spc->dumper,
               dumper,
               sizeof(spc->dumper));
    }
}

void spc_set_comments(spc_t *spc,
                      const char *comments)
{
    if ((spc != NULL)
        && (comments != NULL))
    {
        memcpy(spc->comments,
               comments,
               sizeof(spc->comments));
    }
}

void spc_set_song_length_s(spc_t *spc,
                           const char *song_length_s)
{
    if ((spc != NULL)
        && (song_length_s != NULL))
    {
        memcpy(spc->song_length_s,
               song_length_s,
               sizeof(spc->song_length_s));
    }
}

void spc_set_fade_length_ms(spc_t *spc,
                            const char *fade_length_ms)
{
    if ((spc != NULL)
        && (fade_length_ms != NULL))
    {
        memcpy(spc->fade_length_ms,
               fade_length_ms,
               sizeof(spc->fade_length_ms));
    }
}

void spc_set_artist(spc_t *spc,
                    const char *artist)
{
    if ((spc != NULL)
        && (artist != NULL))
    {
        memcpy(spc->artist,
               artist,
               sizeof(spc->artist));
    }
}

void spc_set_dump_date(spc_t *spc,
                       int day,
                       int month,
                       int year)
{
    if (spc != NULL)
    {
        char dump_date_tmp[sizeof(spc->dump_date) + 1];
        //******************
        printf("day %d, month %d, year %d", day, month, year);
        snprintf(dump_date_tmp,
                 sizeof(dump_date_tmp),
                 "%02d/%02d/%04d",
                 month,
                 day,
                 year);
        memcpy(spc->dump_date,
               dump_date_tmp,
               sizeof(spc->dump_date));
    }
}
