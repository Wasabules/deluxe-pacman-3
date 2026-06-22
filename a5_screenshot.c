// Note: if you use physfs, than you need to call al_set_standard_file_interface();
// before this function and al_set_physfs_file_interface(); afterwards.

#include "a5_screenshot.h"

bool a5_screenshot(const char *gamename)
{
   time_t rawtime;
   struct tm *timeinfo;
   char filename[80], timestr[80];
   bool saved;
   ALLEGRO_STATE state;

   al_store_state(&state, ALLEGRO_STATE_NEW_FILE_INTERFACE);

   al_set_standard_file_interface();

   time(&rawtime);
   timeinfo = localtime(&rawtime);

   strftime(timestr, 80, "%Y%m%d_%H%M%S", timeinfo);
   snprintf(filename, 80, "%s_%s.png", gamename, timestr);

   saved = al_save_bitmap(filename, al_get_target_bitmap());

   al_restore_state(&state);

   if(!saved) return false;

   return true;
}
