#pragma once

#include <cursed-tea/core.h>
#include <stddef.h>
typedef struct MeatSpaceTitle {
  struct CtModel base;
} MeatSpaceTitle;

void meatspace_title_setup(MeatSpaceTitle *model);
