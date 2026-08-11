#pragma once

#include "../components/archive-title.h"
#include "../components/column.h"
#include "../components/phrack-title.h"
#include "../components/stars-wall.h"
#include "archive.h"
#include "board.h"
#include "initial-menu.h"
#include "latest-listing.h"
#include "thread-view.h"
#include <cursed-tea/core.h>

enum HomePageState {
  HP_MENU,
  HP_MACRO_BOARD,
  HP_BOARD,
  HP_ARCHIVE,
  HP_PHRAK,
  HP_THREAD,
};

typedef struct HomePage {
  struct CtModel base;
  StarsWall background;

  enum HomePageState state;

  // SubPages
  InitialMenu menu;

  bool _from_macro_board;
  ThreadView thread;

  bool _board_initialized;
  BoardListing board;

  bool _latest_initialized;
  LatestListing latest;

  bool _archive_initialized;
  Archive archive;

  // Elements
  ArchiveTitle archive_title;
  PhrackTitle phrack_title;

} HomePage;

void home_page_setup(HomePage *model);
