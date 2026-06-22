#include "dp2_sound.h"


ALLEGRO_VOICE *voice = NULL;
ALLEGRO_MIXER *mixer = NULL;

bool init_sound(int samples)
{
   if(!al_install_audio()) {
      printf("al_install_audio() failed.\n");
      return false;
   }

   voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
   if(voice == NULL) {
      printf("Error creating voice\n");
      return false;
   }

   mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
   if(mixer == NULL) {
      printf("Error creating mixer.\n");
      return false;
   }

   if(!al_attach_mixer_to_voice(mixer, voice)){
      printf("Error attaching mixer to voice.\n");
      return false;
   }

   if(!al_set_default_mixer(mixer)) {
      printf("Error setting default mixer.\n");
      return false;
   }

   if(!al_reserve_samples(samples)) {
      printf("Error reserving samples.\n");
      return false;
   }

   return true;
}


bool load_sound(SFX *sfx, char const *file)
{
   sfx->samp = al_load_sample(file);
   if(!sfx->samp) return false;

/*
   sfx->inst = al_create_sample_instance(sfx->samp);
   if(!sfx->inst) return false;

   if(!al_attach_sample_instance_to_mixer(sfx->inst, mixer)) return false;
*/
   return true;
}


void play_sound(SFX *sfx, float volume, float pan, float speed, ALLEGRO_PLAYMODE playmode)
{
   //al_play_sample(sfx_pickup, setting.sound_volume, (pickup.x / PAN_RATIO), 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
   al_set_sample_instance_gain(sfx->inst, volume);
   al_set_sample_instance_pan(sfx->inst, pan);
   al_set_sample_instance_speed(sfx->inst, speed);
   al_set_sample_instance_playmode(sfx->inst, playmode);
   al_play_sample_instance(sfx->inst);
}


void destroy_sound(SFX *sfx)
{
   if(sfx->inst) al_detach_sample_instance(sfx->inst);

   if(sfx->inst) {
      al_destroy_sample_instance(sfx->inst);
      sfx->inst = NULL;
   }

   if(sfx->samp) {
      al_destroy_sample(sfx->samp);
      sfx->samp = NULL;
   }
}

void shutdown_sound(void) {
   if(mixer) al_destroy_mixer(mixer);
   if(voice) al_destroy_voice(voice);
   if(al_is_audio_installed()) al_uninstall_audio();
}
