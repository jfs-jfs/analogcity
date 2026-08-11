#pragma once

#include <cursed-tea/core.h>

#define DEFAULT_BG_COLOR "#141414"
#define DEFAULT_FG_COLOR "#B0B0B0"

#define BLUE_COLOR "#278BF5"
#define PINK_COLOR "#AA1DB1"
#define GREY_COLOR "#878787"
#define HINT_COLOR "#666"
#define BLACK_COLOR "#242424"
#define RED_COLOR "#CF1313"

typedef struct StylePass {
  struct CtModel base;
  struct CtModel *child;
} StylePass;

void stylepass_setup(StylePass *model, struct CtModel *child);
