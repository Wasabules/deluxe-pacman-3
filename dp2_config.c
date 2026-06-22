#include "dp2_config.h"



//    Loads values from a config file.
int option(ALLEGRO_CONFIG *config, char const *name, int v)
{
   char const *value = NULL;
   char str[TEXT_BUFFER] = "";

   value = al_get_config_value(config, "settings", name);
   if(value) v = strtol(value, NULL, 0);

   snprintf(str, TEXT_BUFFER, "%d", v);

   al_set_config_value(config, "settings", name, str);

   return v;
}



// Load Config Settings.
// If no config file exists, create a new one
// with default settings.
void load_config(SETTING *setting)
{
   int write = false;
   ALLEGRO_CONFIG *config = NULL;
   config = al_load_config_file("Deluxe Pacman 2.ini");
   if(!config) {
      config = al_create_config();
      write = true;
   }

   // vsync: 0 -> Driver chooses, 1 on, 2 off.
   setting->vsync = option(config, "vsync", 2);
   // fullscreen: 0 = windowed, 1 = fullscreen, 2 = fullscreen window
   //(desktop resolution with a scaled 800x600 screen to fit it)
   setting->fullscreen = option(config, "fullscreen", 1);
   setting->shadows = option(config, "shadows", 1);
   setting->frequency = option(config, "frequency", 0);
   setting->sound_on = option(config, "sound_on", 1);
   setting->music_on = option(config, "music_on", 1);
   setting->sound_volume = (float)option(config, "sound_volume", 7) / 10; // convert to float 0.0 - 1.0
   setting->music_volume = (float)option(config, "music_volume", 5) / 10; // convert to float 0.0 - 1.0
   setting->opengl = option(config, "opengl", 0);
   setting->cheat_mode = option(config, "testbed", 0);
   setting->difficulty = option(config, "difficulty", 1);
   setting->user_levels = option(config, "user_levels", 1);
   setting->suicide = option(config, "suicide", 0);
   setting->pakman = option(config, "pakman", 0);

   // Write the file back (so a template is generated on first run).
   if(write) al_save_config_file("Deluxe Pacman 2.ini", config);

   al_destroy_config(config);
}



// Save Config Settings.
// Saves the current settings struct
// Mainly used in the options menu
void save_config(SETTING *setting)
{
   ALLEGRO_CONFIG *config = NULL;

   config = al_create_config();

   option(config, "vsync", setting->vsync);
   option(config, "fullscreen", setting->fullscreen);
   option(config, "shadows", (int)setting->shadows);
   option(config, "frequency", setting->frequency);
   option(config, "sound_on", (int)setting->sound_on);
   option(config, "music_on", (int)setting->music_on);
   option(config, "sound_volume", (int)(setting->sound_volume * 10));
   option(config, "music_volume", (int)(setting->music_volume * 10));
   option(config, "opengl", (int)setting->opengl);
   option(config, "testbed", (int)setting->cheat_mode);
   option(config, "difficulty", setting->difficulty);
   option(config, "user_levels", (int)setting->user_levels);
   option(config, "suicide", setting->suicide);

   al_set_standard_file_interface(); // need to switch from PAK file to disk access
   if(!al_save_config_file("Deluxe Pacman 2.ini", config)) {
      a5_error(AT, setting->screen, "Error saving settings.\n");
   }
   al_set_physfs_file_interface(); // switch back to PAK file access

   al_destroy_config(config);
}
