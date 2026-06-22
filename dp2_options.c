#include "dp2_options.h"


int options_menu(void)
{
   ALLEGRO_BITMAP *options_back = NULL, *buffer = NULL;
   SETTING default_settings = setting;
   SETTING old_settings = setting;
   BUTTON s_windowed, s_fswindow, s_fullscreen, s_sound, s_music, s_shadows, s_opengl;
   BUTTON s_diffeasy, s_diffmed, s_diffhard, s_userlevels, s_suicide;
   BUTTON b_lsound, b_rsound, b_lmusic, b_rmusic;
   BUTTON b_defaults, b_save, b_exit;
   bool done = false, options_done = false;
   int options_shutdown = 0;
   ALLEGRO_SAMPLE *options_select = NULL, *options_enter = NULL;
   ALLEGRO_KEYBOARD_STATE keys;
   ALLEGRO_MOUSE_STATE mouse;
   const int num_buttons = 19;
   BUTTON *p_select[] = {&s_windowed, &s_fswindow, &s_fullscreen, &s_sound, &b_lsound, &b_rsound,
                         &s_music, &b_lmusic, &b_rmusic, &s_shadows, &s_opengl,
                         &s_diffeasy, &s_diffmed, &s_diffhard, &s_userlevels, &s_suicide,
                         &b_defaults, &b_save, &b_exit
                        };

   int selection = 0; // DEFAULTS is the first, maximum value is num_buttons-1

   default_settings.vsync = 0;
   default_settings.fullscreen = 2;
   default_settings.shadows = 1;
   default_settings.frequency = 0;
   default_settings.sound_on = 1;
   default_settings.music_on = 1;
   default_settings.sound_volume = 0.7f;
   default_settings.music_volume = 0.5f;
   default_settings.opengl = 0;
   default_settings.cheat_mode = 0;
   default_settings.difficulty = 1;
   default_settings.user_levels = 1;
   default_settings.suicide = 0;

   if(!new_button(&s_windowed, true, "Graphics/Buttons/Button.png", 120, 84)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }
   if(setting.fullscreen == 0) s_windowed.state = s_windowed.old_state = 2;

   if(!new_button(&s_fswindow, true, "Graphics/Buttons/Button.png", 320, 84)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }
   if(setting.fullscreen == 1) s_fswindow.state = s_fswindow.old_state = 2;

   if(!new_button(&s_fullscreen, true, "Graphics/Buttons/Button.png", 520, 84)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }
   if(setting.fullscreen == 2) s_fullscreen.state = s_fullscreen.old_state = 2;


   if(!new_button(&s_sound, true, "Graphics/Buttons/Button.png", 120, 134)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }
   if(setting.sound_on) s_sound.state = s_sound.old_state = 2;

   if(!new_button(&b_lsound, false, "Graphics/Buttons/Left_Arrow.png", 368, 134)) {
      printf("%s(%d): Error loading Left_Arrow.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }

   if(!new_button(&b_rsound, false, "Graphics/Buttons/Right_Arrow.png", 432, 134)) {
      printf("%s(%d): Error loading Right_Arrow.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }

   if(!new_button(&s_music, true, "Graphics/Buttons/Button.png", 120, 184)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }
   if(setting.music_on) s_music.state = s_music.old_state = 2;

   if(!new_button(&b_lmusic, false, "Graphics/Buttons/Left_Arrow.png", 368, 184)) {
      printf("%s(%d): Error loading Left_Arrow.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }

   if(!new_button(&b_rmusic, false, "Graphics/Buttons/Right_Arrow.png", 432, 184)) {
      printf("%s(%d): Error loading Right_Arrow.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }

   if(!new_button(&s_shadows, true, "Graphics/Buttons/Button.png", 120, 234)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }
   if(setting.shadows) s_shadows.state = s_shadows.old_state = 2;

   if(!new_button(&s_opengl, true, "Graphics/Buttons/Button.png", 120, 284)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }
   if(setting.opengl) s_opengl.state = s_opengl.old_state = 2;

   if(!new_button(&s_diffeasy, true, "Graphics/Buttons/Button.png", 120, 334)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }
   if(setting.difficulty == 0) s_diffeasy.state = s_diffeasy.old_state = 2;

   if(!new_button(&s_diffmed, true, "Graphics/Buttons/Button.png", 320, 334)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }
   if(setting.difficulty == 1) s_diffmed.state = s_diffmed.old_state = 2;

   if(!new_button(&s_diffhard, true, "Graphics/Buttons/Button.png", 520, 334)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }
   if(setting.difficulty == 2) s_diffhard.state = s_diffhard.old_state = 2;

   if(!new_button(&s_userlevels, true, "Graphics/Buttons/Button.png", 120, 384)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }
   if(setting.user_levels) s_userlevels.state = s_userlevels.old_state = 2;

   if(!new_button(&s_suicide, true, "Graphics/Buttons/Button.png", 120, 434)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }
   if(setting.suicide) s_suicide.state = s_suicide.old_state = 2;

   if(!new_button(&b_defaults, false, "Graphics/Buttons/Defaults.png", 86, 540)) {
      printf("%s(%d): Error loading Defaults.png.\n", __FILE__, __LINE__);
      return 2;   // 2 = shutdown game completely
   }

   if(!new_button(&b_save, false, "Graphics/Buttons/Save_Small.png", 324, 540)) {
      printf("%s(%d): Error loading Save_Small.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }

   if(!new_button(&b_exit, false, "Graphics/Buttons/Exit.png", 563, 540)) {
      printf("%s(%d): Error loading Exit.png.\n", __FILE__, __LINE__);
      options_shutdown = 2;
      goto SHUTDOWN;
   }

   // Load background image
   options_back = al_load_bitmap("Graphics/Options_Background.png");
   if(!options_back) {
      a5_error(AT, setting.screen, "Failed to load Options_Background.png");
      options_shutdown = 2;
      goto SHUTDOWN;
   }

   options_select = al_load_sample("Sound/Menu_Select.ogg");
   if(!options_select) {
      a5_error(AT, setting.screen, "Failed to load Menu_Select.ogg");
      options_shutdown = 2;
      goto SHUTDOWN;
   }

   options_enter = al_load_sample("Sound/Menu_Enter.ogg");
   if(!options_enter) {
      a5_error(AT, setting.screen, "Failed to load Menu_Enter.ogg");
      options_shutdown = 2;
      goto SHUTDOWN;
   }

   // We'll set the redraw state to true so the buttons get drawn the first time through
   setting.redraw = true;

   if(!p_select[selection]->is_switch) p_select[selection]->state = p_select[selection]->old_state = 1;

   done = false;

   do {
      al_wait_for_event(event_queue, &event);

      // MOUSE
      p_select[selection]->state = 0;
      p_select[selection]->old_state = 0;

      switch(event.type) {
         case ALLEGRO_EVENT_DISPLAY_CLOSE:
            printf("Close button detected.\n");
            options_shutdown = 2;
            goto SHUTDOWN;
            break;

         case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            al_get_mouse_state(&mouse);
            for(int i = 0; i < num_buttons; i++) {
               if(check_button(p_select[i], &mouse)) {
                  p_select[selection]->state = p_select[selection]->old_state;
                  selection = i;
                  p_select[selection]->old_state = p_select[selection]->state;
                  p_select[selection]->state = 2;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(options_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               }
            }
            break;

         case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            al_get_mouse_state(&mouse);

            if(check_button(p_select[selection], &mouse)) { // make certain mouse is over a button
               if(!p_select[selection]->is_switch) p_select[selection]->state = 1;
               setting.redraw = true;

               switch(selection) {
                  case 0: // Windowed mode (s_windowed)
                     s_windowed.state = s_windowed.old_state = 2;
                     s_fswindow.state = s_fswindow.old_state = 0;
                     s_fullscreen.state = s_fullscreen.old_state = 0;
                     setting.fullscreen = 0;
                     setting.redraw = true;
                     break;
                  case 1: // Fullscreen-Window mode (s_fswindow)
                     s_windowed.state = s_windowed.old_state = 0;
                     s_fswindow.state = s_fswindow.old_state = 2;
                     s_fullscreen.state = s_fullscreen.old_state = 0;
                     setting.fullscreen = 1;
                     setting.redraw = true;
                     break;
                  case 2: // Fullscreen mode (s_fullscreen)
                     s_windowed.state = s_windowed.old_state = 0;
                     s_fswindow.state = s_fswindow.old_state = 0;
                     s_fullscreen.state = s_fullscreen.old_state = 2;
                     setting.fullscreen = 2;
                     setting.redraw = true;
                     break;
                  case 3: // Sound On/Off (s_sound)
                     s_sound.state = s_sound.old_state = setting.sound_on ? 1 : 2;
                     setting.sound_on = !setting.sound_on;
                     setting.redraw = true;
                     break;
                  case 4: // Left Sound voume arrow
                     setting.sound_volume -= 0.1f;
                     if(setting.sound_volume < 0) setting.sound_volume = 0;
                     setting.redraw = true;
                     break;
                  case 5: // Right Sound voume arrow
                     setting.sound_volume += 0.1f;
                     if(setting.sound_volume > 1.0f) setting.sound_volume = 1.0f;
                     setting.redraw = true;
                     break;
                  case 6: // Music On/Off (s_music)
                     s_music.state = s_music.old_state = setting.music_on ? 1 : 2;
                     setting.music_on = !setting.music_on;
                     if(!setting.music_on) al_stop_sample(&music_id);
                     else al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                     setting.redraw = true;
                     break;
                  case 7: // Left Music voume arrow
                     setting.music_volume -= 0.1f;
                     if(setting.music_volume < 0) setting.music_volume = 0;
                     if(setting.music_on) {
                        al_stop_sample(&music_id);
                        al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                     }
                     setting.redraw = true;
                     break;
                  case 8: // Right Music voume arrow
                     setting.music_volume += 0.1f;
                     if(setting.music_volume > 1.0f) setting.music_volume = 1.0f;
                     if(setting.music_on) {
                        al_stop_sample(&music_id);
                        al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                     }
                     setting.redraw = true;
                     break;
                  case 9: // Shadows On/Off (s_shadows)
                     s_shadows.state = s_shadows.old_state = setting.shadows ? 1 : 2;
                     setting.shadows = !setting.shadows;
                     setting.redraw = true;
                     break;
                  case 10: // OpenGL On/Off (s_opengl)
                     s_opengl.state = s_opengl.old_state = setting.opengl ? 1 : 2;
                     setting.opengl = !setting.opengl;
                     setting.redraw = true;
                     break;
                  case 11: // Difficulty Easy (s_diffeasy)
                     s_diffeasy.state = s_diffeasy.old_state = 2;
                     s_diffmed.state = s_diffmed.old_state = 0;
                     s_diffhard.state = s_diffhard.old_state = 0;
                     setting.difficulty = 0;
                     setting.redraw = true;
                     break;
                  case 12: // Difficulty Medium (s_diffmed)
                     s_diffeasy.state = s_diffeasy.old_state = 0;
                     s_diffmed.state = s_diffmed.old_state = 2;
                     s_diffhard.state = s_diffhard.old_state = 0;
                     setting.difficulty = 1;
                     setting.redraw = true;
                     break;
                  case 13: // Difficulty Hard (s_diffhard)
                     s_diffeasy.state = s_diffeasy.old_state = 0;
                     s_diffmed.state = s_diffmed.old_state = 0;
                     s_diffhard.state = s_diffhard.old_state = 2;
                     setting.difficulty = 2;
                     setting.redraw = true;
                     break;
                  case 14: // User Levels On/Off (s_userlevels)
                     s_userlevels.state = s_userlevels.old_state = setting.user_levels ? 1 : 2;
                     setting.user_levels = !setting.user_levels;
                     setting.redraw = true;
                     break;
                  case 15: // Suicide Speed On/Off (s_suicide)
                     s_suicide.state = s_suicide.old_state = setting.suicide ? 1 : 2;
                     setting.suicide = !setting.suicide;
                     setting.redraw = true;
                     break;
                  case 16: // DEFAULTS - Reset options to defaults
                     setting = default_settings;
                     setting.redraw = true;
                     break;
                  case 17: // SAVE - Save options
                     if(setting.fullscreen != old_settings.fullscreen) options_shutdown = 1;
                     if(setting.opengl != old_settings.opengl) options_shutdown = 1;
                     save_config(&setting);
                     setting.redraw = true;
                     options_done = true;
                     break;
                  case 18: // EXIT - Exit options screen
                     // change the music setting back to what it used to be.
                     if(old_settings.music_on && !setting.music_on)
                        al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                     else if(!old_settings.music_on && setting.music_on)
                        al_stop_sample(&music_id);

                     if(old_settings.music_volume != setting.music_volume && old_settings.music_on) {
                        al_stop_sample(&music_id);
                        al_play_sample(music_main, old_settings.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                     }
                     setting = old_settings;
                     setting.redraw = true;
                     options_done = true;
                     break;
                  default:
                     printf("OPTIONS: (%i)Invalid menu selection: %i\n", __LINE__, selection);
               }
            }
            for(int i = 1; i < num_buttons - 1; i++) setting.redraw = check_button(p_select[i], &mouse) | setting.redraw;
            break;

         // This changes the buttons based on whether the mouse is over them, but we
         // probably shouldn't use this because we want the buttons to remain lit up
         // for keyboard reference.
         case ALLEGRO_EVENT_MOUSE_AXES:
            al_get_mouse_state(&mouse);
            for(int i = 0; i < num_buttons; i++) {
               bool cb = check_button(p_select[i], &mouse);
               if(cb && (selection != i)) {
                  p_select[selection]->state = p_select[selection]->old_state;
                  selection = i;
                  p_select[selection]->old_state = p_select[selection]->state;
                  p_select[selection]->state = 1;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(options_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               }
            }
            break;
         /// JOYSTICK
         case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
            if(event.joystick.button == 0) {   // Button 0 = A (green)
               if(p_select[selection]->is_switch) p_select[selection]->state = p_select[selection]->old_state = 2;
               else p_select[selection]->state = 2;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(options_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            else if(event.joystick.button == 1) { // Button 1 = B (red)
               p_select[selection]->state = p_select[selection]->old_state;
               selection = num_buttons-1;
               p_select[selection]->state = 2;
               if(setting.sound_on)
                  al_play_sample(options_enter , setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            break;

         case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
            if(event.joystick.button == 0) {   // Button 0 = A (green)
               if(!p_select[selection]->is_switch) p_select[selection]->state = 1;
               setting.redraw = true;
               switch(selection) {
                  case 0: // Windowed mode (s_windowed)
                     s_windowed.state = s_windowed.old_state = 2;
                     s_fswindow.state = s_fswindow.old_state = 0;
                     s_fullscreen.state = s_fullscreen.old_state = 0;
                     setting.fullscreen = 0;
                     setting.redraw = true;
                     break;
                  case 1: // Fullscreen-Window mode (s_fswindow)
                     s_windowed.state = s_windowed.old_state = 0;
                     s_fswindow.state = s_fswindow.old_state = 2;
                     s_fullscreen.state = s_fullscreen.old_state = 0;
                     setting.fullscreen = 1;
                     setting.redraw = true;
                     break;
                  case 2: // Fullscreen mode (s_fullscreen)
                     s_windowed.state = s_windowed.old_state = 0;
                     s_fswindow.state = s_fswindow.old_state = 0;
                     s_fullscreen.state = s_fullscreen.old_state = 2;
                     setting.fullscreen = 2;
                     setting.redraw = true;
                     break;
                  case 3: // Sound On/Off (s_sound)
                     s_sound.state = s_sound.old_state = setting.sound_on ? 1: 2;
                     setting.sound_on = !setting.sound_on;
                     setting.redraw = true;
                     break;
                  case 4: // Left Sound voume arrow
                     setting.sound_volume -= 0.1f;
                     if(setting.sound_volume < 0) setting.sound_volume = 0;
                     setting.redraw = true;
                     break;
                  case 5: // Right Sound voume arrow
                     setting.sound_volume += 0.1f;
                     if(setting.sound_volume > 1.0f) setting.sound_volume = 1.0f;
                     setting.redraw = true;
                     break;
                  case 6: // Music On/Off (s_music)
                     s_music.state = s_music.old_state = setting.music_on ? 1 : 2;
                     setting.music_on = !setting.music_on;
                     if(!setting.music_on) al_stop_sample(&music_id);
                     else al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                     setting.redraw = true;
                     break;
                  case 7: // Left Music voume arrow
                     setting.music_volume -= 0.1f;
                     if(setting.music_volume < 0) setting.music_volume = 0;
                     if(setting.music_on) {
                        al_stop_sample(&music_id);
                        al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                     }
                     setting.redraw = true;
                     break;
                  case 8: // Right Music voume arrow
                     setting.music_volume += 0.1f;
                     if(setting.music_volume > 1.0f) setting.music_volume = 1.0f;
                     if(setting.music_on) {
                        al_stop_sample(&music_id);
                        al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                     }
                     setting.redraw = true;
                     break;
                  case 9: // Shadows On/Off (s_shadows)
                     s_shadows.state = s_shadows.old_state = setting.shadows ? 1 : 2;
                     setting.shadows = !setting.shadows;
                     setting.redraw = true;
                     break;
                  case 10: // OpenGL On/Off (s_opengl)
                     s_opengl.state = s_opengl.old_state = setting.opengl ? 1 : 2;
                     setting.opengl = !setting.opengl;
                     setting.redraw = true;
                     break;
                  case 11: // Difficulty Easy (s_diffeasy)
                     s_diffeasy.state = s_diffeasy.old_state = 2;
                     s_diffmed.state = s_diffmed.old_state = 0;
                     s_diffhard.state = s_diffhard.old_state = 0;
                     setting.difficulty = 0;
                     setting.redraw = true;
                     break;
                  case 12: // Difficulty Medium (s_diffmed)
                     s_diffeasy.state = s_diffeasy.old_state = 0;
                     s_diffmed.state = s_diffmed.old_state = 2;
                     s_diffhard.state = s_diffhard.old_state = 0;
                     setting.difficulty = 1;
                     setting.redraw = true;
                     break;
                  case 13: // Difficulty Hard (s_diffhard)
                     s_diffeasy.state = s_diffeasy.old_state = 0;
                     s_diffmed.state = s_diffmed.old_state = 0;
                     s_diffhard.state = s_diffhard.old_state = 2;
                     setting.difficulty = 2;
                     setting.redraw = true;
                     break;
                  case 14: // User Levels On/Off (s_userlevels)
                     s_userlevels.state = s_userlevels.old_state = setting.user_levels ? 1 : 2;
                     setting.user_levels = !setting.user_levels;
                     setting.redraw = true;
                     break;
                  case 15: // Suicide Speed On/Off (s_suicide)
                     s_suicide.state = s_suicide.old_state = setting.suicide ? 1 : 2;
                     setting.suicide = !setting.suicide;
                     setting.redraw = true;
                     break;
                  case 16: // DEFAULTS - Reset options to defaults
                     setting = default_settings;
                     setting.redraw = true;
                     break;
                  case 17: // SAVE - Save options
                     if(setting.fullscreen != old_settings.fullscreen) options_shutdown = 1;
                     if(setting.opengl != old_settings.opengl) options_shutdown = 1;
                     save_config(&setting);
                     setting.redraw = true;
                     options_done = true;
                     break;
                  case 18: // EXIT - Exit options screen
                     // change the music setting back to what it used to be.
                     if(old_settings.music_on && !setting.music_on)
                        al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                     else if(!old_settings.music_on && setting.music_on)
                        al_stop_sample(&music_id);

                     if(old_settings.music_volume!=setting.music_volume && old_settings.music_on) {
                        al_stop_sample(&music_id);
                        al_play_sample(music_main, old_settings.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                     }
                     setting = old_settings;
                     setting.redraw = true;
                     options_done = true;
                     break;
                  default:
                     printf("OPTIONS: (%i)Invalid menu selection: %i\n", __LINE__, selection);
               }
            }
            else if(event.joystick.button == 1) { // Button 1 = B (red)
               // don't exit until ESC button is released
               // change the music setting back to what it used to be.
               if(old_settings.music_on && !setting.music_on)
                  al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
               else if(!old_settings.music_on && setting.music_on)
                  al_stop_sample(&music_id);
               if(old_settings.music_volume!=setting.music_volume && old_settings.music_on) {
                  al_stop_sample(&music_id);
                  al_play_sample(music_main, old_settings.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
               }
               setting = old_settings;
               setting.redraw = true;
               options_done = true;
               break;
            }
            break;

         case ALLEGRO_EVENT_JOYSTICK_AXIS:
            if(event.joystick.axis == 1) {           // Axis 1 = Y
               if(event.joystick.pos <= -0.5f) {     // Up
                  p_select[selection]->state = p_select[selection]->old_state;
                  selection--;
                  if(selection < 0) selection = num_buttons-1;
                  p_select[selection]->old_state = p_select[selection]->state;
                  p_select[selection]->state = 1;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(options_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               }
               else if(event.joystick.pos >= 0.5f) { // Down
                  p_select[selection]->state = p_select[selection]->old_state;
                  selection++;
                  if(selection > num_buttons-1) selection = 0;
                  p_select[selection]->old_state = p_select[selection]->state;
                  p_select[selection]->state = 1;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(options_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               }
            }
            else if(event.joystick.axis == 0) {      // Axis 0 = X
               if(event.joystick.pos <= -0.5f) {     // Left
                  p_select[selection]->state = p_select[selection]->old_state;
                  selection--;
                  if(selection < 0) selection = num_buttons-1;
                  p_select[selection]->old_state = p_select[selection]->state;
                  p_select[selection]->state = 1;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(options_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               }
               else if(event.joystick.pos >= 0.5f) { // Right
                  p_select[selection]->state = p_select[selection]->old_state;
                  selection++;
                  if(selection > num_buttons-1) selection = 0;
                  p_select[selection]->old_state = p_select[selection]->state;
                  p_select[selection]->state = 1;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(options_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               }
            }
            break;

         case ALLEGRO_EVENT_JOYSTICK_CONFIGURATION:
            al_reconfigure_joysticks();
            break;

         /// TODO: When using the keyboard I may add in a selection box later on
         // Keyboard
         case ALLEGRO_EVENT_KEY_DOWN:
            switch(event.keyboard.keycode) {
               case ALLEGRO_KEY_DOWN:
               case ALLEGRO_KEY_PAD_2:
               case ALLEGRO_KEY_S:
               case ALLEGRO_KEY_RIGHT:
               case ALLEGRO_KEY_PAD_6:
               case ALLEGRO_KEY_D:
                  p_select[selection]->state = p_select[selection]->old_state;
                  selection++;
                  if(selection > num_buttons - 1) selection = 0;
                  p_select[selection]->old_state = p_select[selection]->state;
                  p_select[selection]->state = 1;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(options_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  break;
               case ALLEGRO_KEY_UP:
               case ALLEGRO_KEY_PAD_8:
               case ALLEGRO_KEY_W:
               case ALLEGRO_KEY_LEFT:
               case ALLEGRO_KEY_PAD_4:
               case ALLEGRO_KEY_A:
                  p_select[selection]->state = p_select[selection]->old_state;
                  selection--;
                  if(selection < 0) selection = num_buttons - 1;
                  p_select[selection]->old_state = p_select[selection]->state;
                  p_select[selection]->state = 1;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(options_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  break;
               case ALLEGRO_KEY_ESCAPE:
                  p_select[selection]->state = p_select[selection]->old_state;
                  selection = num_buttons - 1;
                  p_select[selection]->state = 2;
                  if(setting.sound_on)
                     al_play_sample(options_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  break;
               case ALLEGRO_KEY_ENTER:
                  if(p_select[selection]->is_switch) p_select[selection]->state = p_select[selection]->old_state = 2;
                  else p_select[selection]->state = 2;

                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(options_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  break;
            }
            break;

         case ALLEGRO_EVENT_KEY_UP:
            // When ENTER is released, the buttons should pop back up.
            switch(event.keyboard.keycode) {
               case ALLEGRO_KEY_ENTER:
                  if(!p_select[selection]->is_switch) p_select[selection]->state = 1;
                  setting.redraw = true;
                  switch(selection) {
                     case 0: // Windowed mode (s_windowed)
                        s_windowed.state = s_windowed.old_state = 2;
                        s_fswindow.state = s_fswindow.old_state = 0;
                        s_fullscreen.state = s_fullscreen.old_state = 0;
                        setting.fullscreen = 0;
                        setting.redraw = true;
                        break;
                     case 1: // Fullscreen-Window mode (s_fswindow)
                        s_windowed.state = s_windowed.old_state = 0;
                        s_fswindow.state = s_fswindow.old_state = 2;
                        s_fullscreen.state = s_fullscreen.old_state = 0;
                        setting.fullscreen = 1;
                        setting.redraw = true;
                        break;
                     case 2: // Fullscreen mode (s_fullscreen)
                        s_windowed.state = s_windowed.old_state = 0;
                        s_fswindow.state = s_fswindow.old_state = 0;
                        s_fullscreen.state = s_fullscreen.old_state = 2;
                        setting.fullscreen = 2;
                        setting.redraw = true;
                        break;
                     case 3: // Sound On/Off (s_sound)
                        s_sound.state = s_sound.old_state = setting.sound_on ? 1 : 2;
                        setting.sound_on = !setting.sound_on;
                        setting.redraw = true;
                        break;
                     case 4: // Left Sound voume arrow
                        setting.sound_volume -= 0.1f;
                        if(setting.sound_volume < 0) setting.sound_volume = 0;
                        setting.redraw = true;
                        break;
                     case 5: // Right Sound voume arrow
                        setting.sound_volume += 0.1f;
                        if(setting.sound_volume > 1.0f) setting.sound_volume = 1.0f;
                        setting.redraw = true;
                        break;
                     case 6: // Music On/Off (s_music)
                        s_music.state = s_music.old_state = setting.music_on ? 1 : 2;
                        setting.music_on = !setting.music_on;
                        if(!setting.music_on) al_stop_sample(&music_id);
                        else al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                        setting.redraw = true;
                        break;
                     case 7: // Left Music voume arrow
                        setting.music_volume -= 0.1f;
                        if(setting.music_volume < 0) setting.music_volume = 0;
                        if(setting.music_on) {
                           al_stop_sample(&music_id);
                           al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                        }
                        setting.redraw = true;
                        break;
                     case 8: // Right Music voume arrow
                        setting.music_volume += 0.1f;
                        if(setting.music_volume > 1.0f) setting.music_volume = 1.0f;
                        if(setting.music_on) {
                           al_stop_sample(&music_id);
                           al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                        }
                        setting.redraw = true;
                        break;
                     case 9: // Shadows On/Off (s_shadows)
                        s_shadows.state = s_shadows.old_state = setting.shadows ? 1 : 2;
                        setting.shadows = !setting.shadows;
                        setting.redraw = true;
                        break;
                     case 10: // OpenGL On/Off (s_opengl)
                        s_opengl.state = s_opengl.old_state = setting.opengl ? 1 : 2;
                        setting.opengl = !setting.opengl;
                        setting.redraw = true;
                        break;
                     case 11: // Difficulty Easy (s_diffeasy)
                        s_diffeasy.state = s_diffeasy.old_state = 2;
                        s_diffmed.state = s_diffmed.old_state = 0;
                        s_diffhard.state = s_diffhard.old_state = 0;
                        setting.difficulty = 0;
                        setting.redraw = true;
                        break;
                     case 12: // Difficulty Medium (s_diffmed)
                        s_diffeasy.state = s_diffeasy.old_state = 0;
                        s_diffmed.state = s_diffmed.old_state = 2;
                        s_diffhard.state = s_diffhard.old_state = 0;
                        setting.difficulty = 1;
                        setting.redraw = true;
                        break;
                     case 13: // Difficulty Hard (s_diffhard)
                        s_diffeasy.state = s_diffeasy.old_state = 0;
                        s_diffmed.state = s_diffmed.old_state = 0;
                        s_diffhard.state = s_diffhard.old_state = 2;
                        setting.difficulty = 2;
                        setting.redraw = true;
                        break;
                     case 14: // User Levels On/Off (s_userlevels)
                        s_userlevels.state = s_userlevels.old_state = setting.user_levels ? 1 : 2;
                        setting.user_levels = !setting.user_levels;
                        setting.redraw = true;
                        break;
                     case 15: // Suicide Speed On/Off (s_suicide)
                        s_suicide.state = s_suicide.old_state = setting.suicide ? 1 : 2;
                        setting.suicide = !setting.suicide;
                        setting.redraw = true;
                        break;
                     case 16: // DEFAULTS - Reset options to defaults
                        setting = default_settings;
                        setting.redraw = true;
                        break;
                     case 17: // SAVE - Save options
                        if(setting.fullscreen != old_settings.fullscreen) options_shutdown = 1;
                        if(setting.opengl != old_settings.opengl) options_shutdown = 1;
                        save_config(&setting);
                        setting.redraw = true;
                        options_done = true;
                        break;
                     case 18: // EXIT - Exit options screen
                        // change the music setting back to what it used to be.
                        if(old_settings.music_on && !setting.music_on)
                           al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                        else if(!old_settings.music_on && setting.music_on)
                           al_stop_sample(&music_id);

                        if(old_settings.music_volume != setting.music_volume && old_settings.music_on) {
                           al_stop_sample(&music_id);
                           al_play_sample(music_main, old_settings.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                        }
                        setting = old_settings;
                        setting.redraw = true;
                        options_done = true;
                        break;
                     default:
                        printf("OPTIONS: (%i)Invalid menu selection: %i\n", __LINE__, selection);
                  }
                  break;
               case ALLEGRO_KEY_F12:
                  if(setting.sound_on)
                     al_play_sample(sfx_camera, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  a5_screenshot("Screenshots/DeluxePacman2");
                  break;
               case ALLEGRO_KEY_ESCAPE:
                  // don't exit until ESC is released
                  // change the music setting back to what it used to be.
                  if(old_settings.music_on && !setting.music_on)
                     al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                  else if(!old_settings.music_on && setting.music_on)
                     al_stop_sample(&music_id);

                  if(old_settings.music_volume != setting.music_volume && old_settings.music_on) {
                     al_stop_sample(&music_id);
                     al_play_sample(music_main, old_settings.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
                  }
                  setting = old_settings;
                  setting.redraw = true;
                  options_done = true;
                  break;
            }
            break;

         case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
            printf("Display Switched In\n");
            setting.redraw = true;
            break;

         case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
            printf("Display Switched Out\n");
            break;

         case ALLEGRO_EVENT_DISPLAY_FOUND:
            printf("Display Switched Found\n");
            setting.redraw = true;
            break;

         case ALLEGRO_EVENT_DISPLAY_LOST:
            printf("Display Switched Lost\n");
            break;
      }

      if(setting.redraw) {
         setting.redraw = false;
         switch(setting.fullscreen) {
            case 0:
               s_windowed.state = s_windowed.old_state = selection == 0 ? 1 : 2;
               s_fswindow.state = selection == 1 ? 1 : 0;
               s_fullscreen.state = selection == 2 ? 1 : 0;
               break;
            case 1:
               s_windowed.state = selection == 0 ? 1 : 0;
               s_fswindow.state = s_fswindow.old_state = selection == 1 ? 1 : 2;
               s_fullscreen.state = selection == 2 ? 1 : 0;
               break;
            case 2:
               s_windowed.state = selection == 0 ? 1 : 0;
               s_fswindow.state = selection == 1 ? 1 : 0;
               s_fullscreen.state = s_fullscreen.old_state = selection == 2 ? 1 : 2;
               break;
         }
         if(setting.sound_on) s_sound.state = s_sound.old_state = selection == 3 ? 1 : 2;
         if(setting.music_on) s_music.state = s_music.old_state = selection == 6 ? 1 : 2;
         if(setting.shadows) s_shadows.state = s_shadows.old_state = selection == 9 ? 1 : 2;
         if(setting.opengl) s_opengl.state = s_opengl.old_state = selection == 10 ? 1 : 2;
         switch(setting.difficulty) {
            case 0:
               s_diffeasy.state = s_diffeasy.old_state = selection == 11 ? 1 : 2;
               s_diffmed.state = selection == 12 ? 1 : 0;
               s_diffhard.state = selection == 13 ? 1 : 0;
               break;
            case 1:
               s_diffeasy.state = selection == 11 ? 1 : 0;
               s_diffmed.state = s_diffmed.old_state = selection == 12 ? 1 : 2;
               s_diffhard.state = selection == 13 ? 1 : 0;
               break;
            case 2:
               s_diffeasy.state = selection == 11 ? 1 : 0;
               s_diffmed.state = selection == 12 ? 1 : 0;
               s_diffhard.state = s_diffhard.old_state = selection == 13 ? 1 : 2;
               break;
         }
         if(setting.user_levels) s_userlevels.state = s_userlevels.old_state = selection == 14 ? 1 : 2;
         if(setting.suicide) s_suicide.state = s_suicide.old_state = selection == 15 ? 1 : 2;


         al_set_target_bitmap(al_get_backbuffer(setting.screen));

         al_draw_bitmap(options_back, 0, 0, 0);

         for(int i = 0; i < num_buttons; i++) draw_button(p_select[i]);

         // X+34, Y+6
         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 154, 90, ALLEGRO_ALIGN_LEFT, "Windowed");
         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 354, 80, ALLEGRO_ALIGN_LEFT, "Fullscreen");
         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 354, 100, ALLEGRO_ALIGN_LEFT, "Window");
         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 554, 90, ALLEGRO_ALIGN_LEFT, "Fullscreen");

         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 154, 140, ALLEGRO_ALIGN_LEFT, "Sound Volume:");
         al_draw_textf(font_digital, al_map_rgb_f(1, 0, 0), 400, 140, ALLEGRO_ALIGN_LEFT,
                       "%2d", (int)(setting.sound_volume * 10));

         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 154, 190, ALLEGRO_ALIGN_LEFT, "Music Volume:");
         al_draw_textf(font_digital, al_map_rgb_f(1, 0, 0), 400, 190, ALLEGRO_ALIGN_LEFT,
                       "%2d", (int)(setting.music_volume * 10));

         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 154, 240, ALLEGRO_ALIGN_LEFT, "Shadows");
         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 154, 290, ALLEGRO_ALIGN_LEFT, "OpenGL");

         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 154, 340, ALLEGRO_ALIGN_LEFT, "Easy");
         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 354, 340, ALLEGRO_ALIGN_LEFT, "Medium");
         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 554, 340, ALLEGRO_ALIGN_LEFT, "Hard");

         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 154, 390, ALLEGRO_ALIGN_LEFT, "User Levels");

         al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 154, 440, ALLEGRO_ALIGN_LEFT, "Suicide Speed");

         al_flip_display();
         done = options_done;
      }
   }
   while(!done);

   // rest long enough for the player to see the button pressed animation and hear the sound effect
   al_rest(.75);


SHUTDOWN:
   al_destroy_sample(options_select);
   al_destroy_sample(options_enter);

   al_destroy_bitmap(options_back);

   for(int i = 0; i < num_buttons; i++) destroy_button(p_select[i]);

   return options_shutdown;
}
