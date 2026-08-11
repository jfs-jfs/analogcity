#pragma once

#include <cursed-tea/core.h>
#include <stddef.h>
typedef struct StarsWall {
  struct CtModel base;
  size_t spacing_x, spacing_y;
} StarsWall;

void stars_wall_setup(StarsWall *model, const size_t spacing_x,
                      const size_t spacing_y);
