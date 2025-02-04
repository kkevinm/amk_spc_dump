#ifndef AMK_DUMP_H_
#define AMK_DUMP_H_

#include "rom.h"

void amk_dump(const rom_t *rom,
              const char *rom_name,
              const char *out_path,
              const char *default_song_length,
              const char *default_fade_length);

#endif
