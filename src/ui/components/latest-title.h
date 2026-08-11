#pragma once

#include <cursed-tea/core.h>
#include <stddef.h>
typedef struct LatestTitle {
  struct CtModel base;
} LatestTitle;

void latest_title_setup(LatestTitle *model);
