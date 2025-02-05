#ifndef SPC_H_
#define SPC_H_

#include <stdint.h>

typedef enum __attribute__((packed))
{
    SPC_ID666_ENABLED  = 0x26,
    SPC_ID666_DISABLED = 0x27
} spc_id666_flag_t;

typedef struct __attribute__((packed))
{
    uint16_t pc;
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t psw;
    uint8_t sp_low;
} spc_regs_t;

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
    uint8_t right_volume;
    uint8_t left_volume;
    uint16_t pitch;
    uint8_t sample;
    uint16_t adsr;
    uint8_t gain;
    uint8_t current_envelope;
    uint8_t current_sample;
    uint8_t unused[2];
} spc_dsp_channel_t;

typedef struct __attribute__((packed))
{
    spc_dsp_channel_t channel0;
    uint8_t left_master_volume;
    uint8_t echo_feedback;
    uint8_t reserved0;
    uint8_t echo_fir0;
    spc_dsp_channel_t channel1;
    uint8_t right_master_volume;
    uint8_t reserved1[2];
    uint8_t echo_fir1;
    spc_dsp_channel_t channel2;
    uint8_t left_echo_volume;
    uint8_t pitch_modulation_enable;
    uint8_t reserved2;
    uint8_t echo_fir2;
    spc_dsp_channel_t channel3;
    uint8_t right_echo_volume;
    uint8_t noise_enable;
    uint8_t reserved3;
    uint8_t echo_fir3;
    spc_dsp_channel_t channel4;
    uint8_t key_on_voices;
    uint8_t echo_enable;
    uint8_t reserved4;
    uint8_t echo_fir4;
    spc_dsp_channel_t channel5;
    uint8_t key_off_voices;
    uint8_t sample_table_address;
    uint8_t reserved5;
    uint8_t echo_fir5;
    spc_dsp_channel_t channel6;
    uint8_t misc_flags;
    uint8_t echo_buffer_address;
    uint8_t reserved6;
    uint8_t echo_fir6;
    spc_dsp_channel_t channel7;
    uint8_t voice_end_flags;
    uint8_t echo_delay;
    uint8_t reserved7;
    uint8_t echo_fir7;
} spc_dsp_t;

typedef struct __attribute__((packed))
{
    char header_string[33];
    uint8_t header_bytes[2];
    spc_id666_flag_t has_id666;
    uint8_t version_minor;
    spc_regs_t regs;
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
    spc_dsp_t dsp;
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
