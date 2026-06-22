#pragma once
bool init_hiscore(void);
void clear_hiscores(int dif);
void shutdown_hiscore(void);
bool score_table(unsigned long score, int player, int level);
unsigned long get_hiscore(int dif, int num);
