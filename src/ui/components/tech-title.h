#pragma once

#include <cursed-tea.h>
#include <stddef.h>
typedef struct TechnologyTitle {
  struct CtModel base;
} TechnologyTitle;

void technology_title_setup(TechnologyTitle *model);
