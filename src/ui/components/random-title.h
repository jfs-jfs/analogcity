#pragma once

#include <cursed-tea/core.h>
#include <stddef.h>
typedef struct RandomTitle {
  struct CtModel base;
} RandomTitle;

void random_title_setup(RandomTitle *model);
