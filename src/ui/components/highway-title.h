#pragma once

#include <cursed-tea/core.h>
#include <stddef.h>
typedef struct HighwayTitle {
  struct CtModel base;
} HighwayTitle;

void highway_title_setup(HighwayTitle *model);
