#pragma once

#include <cursed-tea.h>
#include <stddef.h>
typedef struct HighwayTitle {
  struct CtModel base;
} HighwayTitle;

void highway_title_setup(HighwayTitle *model);
