#pragma once

#include "../components/title.h"
#include <cursed-tea/core.h>

enum InitialMenuOption {
  // OTHERS
  IM_ARCHIVE,
  IM_PRHAK,
  // BOARDS
  IM_LATEST,
  IM_HIGHWAY,
  IM_MEATSPACE,
  IM_TECHNOLOGY,
  IM_META,
  IM_RANDOM,
};

typedef struct InitialMenu {
  struct CtModel base;
  Title title;
  enum InitialMenuOption selection;
} InitialMenu;

void initial_menu_setup(InitialMenu *model);
