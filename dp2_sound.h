#ifndef _dp2_sound_h_
#define _dp2_sound_h_

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

typedef struct SFX {
   ALLEGRO_SAMPLE *samp;
   ALLEGRO_SAMPLE_INSTANCE *inst;
} SFX;

bool init_sound(int samples);
bool load_sound(SFX *sfx, char const *file);
void play_sound(SFX *sfx, float volume, float pan, float speed, ALLEGRO_PLAYMODE playmode);
void destroy_sound(SFX *sfx);
void shutdown_sound(void);

#endif
