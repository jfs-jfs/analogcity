#pragma once

#include "../components/byte-wall.h"
#include "../components/fader.h"
#include "../components/growing-vline.h"
#include "../components/title.h"

#include <cursed-tea/core.h>
typedef struct WelcomePage {
  struct CtModel base;
  GrowingVLine line1, line2;
  Fader background_wrapper;
  ByteWall background;
  Title title;
  bool _line1_done, _line2_done, _fade_done, _fade_started;
} WelcomePage;

void welcome_page_setup(WelcomePage *model);
