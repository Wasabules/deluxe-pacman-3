#pragma once
#include <stdio.h>
#include <math.h> // sqrt()
#include <stdlib.h> // abs()
#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include "dp2_pacman.h"
#include "dp2_ghost.h"
#include "dp2_config.h"

bool dp2_collision(PACMAN *p, GHOST *g, bool h);
