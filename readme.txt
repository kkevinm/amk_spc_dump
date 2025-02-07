AddmusicK SPC dumper v1.0

-----------------------------------

This tool allows you to extract SPC files of all the songs used in a SNES SMW hack.
It only supports hacks that used AddmusicK to insert the music.

To use it, just double click on the "amk_spc_dump.bat" file and drag and drop the hack you want
into the terminal window, then hit Enter. If the file dragged is a SMW hack and AddmusicK is
correctly detected in the ROM, it will generate all the SPC files in the folder "<hack name>_spcs",
which will be created in the same folder as the hack file.

The generated files will use the naming convention "<hack name>_<song number>.spc", where <song
number> will be the song number used in the ROM in hex. The SPC files will have minimal metadata
(just the rom name in the "game" field, and a default song length), since this information cannot
be retrieved just from the ROM file.

-----------------------------------

You can customize some of the tool's behavior by editing the "amk_spc_dump.bat" file:

- YOSHI_DRUMS: this determines if the generated SPCs will have the Yoshi drums channel(s) enabled
  or not (0 = disabled, 1 = enabled). By default it is enabled.
- GLOBAL_SONGS: this determines if SPCs for global songs (i.e. all the various jingles like death
  and goal music) will be generated. Usually hacks just keep the vanilla ones, so by default it is
  disabled. Note that due to the nature of global music, the generated SPCs will use the samples
  and echo settings as how they're defined in the global song's data, so it could sound different
  from in-game, since there it depends on the samples and echo settings of the song playing before
  the global song is played.
- VANILLA_SONGS: this determines if the first 31 local songs will be generated. These are usually
  just the vanilla songs, so it is disabled by default, but some hacks may remove them or replace
  some of them with custom music, so in that case change this to 1.
- SONG_LENGTH_S: the song length (in seconds) set in all the SPC files. It is 4 minutes by default.
- FADE_LENGTH_MS: the song fadeout length (in milliseconds) set in all the SPC files. It is 10
  seconds by default.

TODO:
- check unheadered roms
- check different AMK versions
- add global song number override to account for an edge case: if the amk list doesn't have entries
  for the first songs after the global music list, it will mess up the global song number calculation
  done by the tool, resulting in silent SPCs

-----------------------------------

Changelog:

- v1.0: initial version
