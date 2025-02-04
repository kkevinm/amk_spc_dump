#ifndef SPC_H_
#define SPC_H_

#include <stdint.h>

typedef enum __attribute__((packed))
{
    SPC_ID666_ENABLED  = 0x26,
    SPC_ID666_DISABLED = 0x27
} spc_id666_flag_t;

typedef enum __attribute__((packed))
{
    SPC_DEFAULT_CHANNEL_ENABLED  = 0,
    SPC_DEFAULT_CHANNEL_DISABLED = 1
} spc_default_channel_flag_t;

typedef enum __attribute__((packed))
{
    SPC_DUMP_EMULATOR_UNKNOWN = 0,
    SPC_DUMP_EMULATOR_ZSNES   = 1,
    SPC_DUMP_EMULATOR_SNES9X  = 2
} spc_dump_emulator_t;

typedef struct __attribute__((packed))
{
    char header_string[33];
    uint8_t header_bytes[2];
    spc_id666_flag_t has_id666;
    uint8_t version_minor;
    uint16_t reg_pc;
    uint8_t reg_a;
    uint8_t reg_x;
    uint8_t reg_y;
    uint8_t reg_psw;
    uint8_t reg_sp_low;
    uint8_t reserved1[2];
    char song_title[32];
    char game_title[32];
    char dumper[16];
    char comments[32];
    char dump_date[11];
    char song_length_s[3];
    char fade_length_ms[5];
    char artist[32];
    spc_default_channel_flag_t default_channel_disables;
    spc_dump_emulator_t dump_emulator;
    uint8_t reserved2[45];
    uint8_t ram[65536];
    uint8_t dsp_regs[128];
    uint8_t unused[64];
    uint8_t extra_ram[64];
} spc_t;

spc_t *spc_create(void);

void spc_destroy(spc_t *spc);

void spc_init(spc_t *spc);

void spc_set_song_title(spc_t *spc,
                        const char *song_title);

void spc_set_game_title(spc_t *spc,
                        const char *game_title);

void spc_set_dumper(spc_t *spc,
                    const char *dumper);

void spc_set_comments(spc_t *spc,
                      const char *comments);

void spc_set_song_length_s(spc_t *spc,
                           const char *song_length_s);

void spc_set_fade_length_ms(spc_t *spc,
                            const char *fade_length_ms);

void spc_set_artist(spc_t *spc,
                    const char *artist);

void spc_set_dump_date(spc_t *spc,
                       int day,
                       int month,
                       int year);

#endif
