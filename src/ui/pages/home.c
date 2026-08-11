#include "home.h"
#include "../components/events.h"
#include "archive.h"
#include "board.h"
#include "initial-menu.h"
#include "latest-listing.h"
#include "thread-view.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/core.h>
#include <cursed-tea/event.h>
#include <cursed-tea/helpers.h>
#include <cursed-tea/layout.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/style/border.h>
#include <cursed-tea/style/brush.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

#define BANNER_HEIGT 26
#define MENU_WIDTH 33

void home_page_cleanup(void *uncasted_model) {
  HomePage *model = uncasted_model;
  model->menu.base.cleanup(&model->menu);
  model->board.base.cleanup(&model->board);
  model->background.base.cleanup(&model->background);

  ct_subcleanup(&model->background.base);

  ct_subcleanup(&model->menu.base);
  ct_subcleanup(&model->thread.base);

  ct_subcleanup(&model->archive_title.base);
  ct_subcleanup(&model->phrack_title.base);

  if (model->_board_initialized)
    ct_subcleanup(&model->board.base);

  if (model->_latest_initialized)
    ct_subcleanup(&model->latest.base);

  if (model->_archive_initialized)
    ct_subcleanup(&model->archive.base);
}

void home_page_handler(void *uncasted_model, const struct CtEvent *event) {
  HomePage *model = uncasted_model;

  // Changes of subpage
  if (event->type == CUSTOM_EVENT && event->custom_signal == CUSTOM_EV_DONE) {
    if (event->data == &model->menu) {
      switch (model->menu.selection) {
      case IM_ARCHIVE:
        model->state = HP_ARCHIVE;
        if (model->_archive_initialized) {
          ct_subcleanup(&model->archive.base);
        }
        archive_setup(&model->archive, ARCHIVE_ANALOG_DIRECTORY,
                      &model->archive_title.base);
        model->_archive_initialized = true;
        break;
      case IM_PRHAK:
        model->state = HP_PHRAK;
        if (model->_archive_initialized) {
          ct_subcleanup(&model->archive.base);
        }
        archive_setup(&model->archive, ARCHIVE_PHRACK_DIRECTORY,
                      &model->phrack_title.base);
        model->_archive_initialized = true;
        break;
      case IM_LATEST:
        model->state = HP_MACRO_BOARD;
        if (model->_latest_initialized)
          ct_subcleanup(&model->latest.base);
        model->_latest_initialized = true;
        latest_listing_setup(&model->latest);
        break;
      case IM_HIGHWAY:
        model->state = HP_BOARD;
        if (model->_board_initialized)
          ct_subcleanup(&model->board.base);
        model->_board_initialized = true;
        board_listing_setup(&model->board, BOARD_HIGHWAY);
        break;
      case IM_MEATSPACE:
        model->state = HP_BOARD;
        if (model->_board_initialized)
          ct_subcleanup(&model->board.base);
        model->_board_initialized = true;
        board_listing_setup(&model->board, BOARD_MEATSPACE);
        break;
      case IM_TECHNOLOGY:
        model->state = HP_BOARD;
        if (model->_board_initialized)
          ct_subcleanup(&model->board.base);
        model->_board_initialized = true;
        board_listing_setup(&model->board, BOARD_TECHNOLOGY);
        break;
      case IM_META:
        model->state = HP_BOARD;
        if (model->_board_initialized)
          ct_subcleanup(&model->board.base);
        model->_board_initialized = true;
        board_listing_setup(&model->board, BOARD_META);
        break;
      case IM_RANDOM:
        model->state = HP_BOARD;
        if (model->_board_initialized)
          ct_subcleanup(&model->board.base);
        model->_board_initialized = true;
        board_listing_setup(&model->board, BOARD_RANDOM);
        break;
      }
    } else if (event->data == &model->board) {
      if (model->board.selection == NULL)
        model->state = HP_MENU;
      else {
        model->state = HP_THREAD;
        model->_from_macro_board = false;
        // Clean previous
        ct_subcleanup(&model->thread.base);
        // Build new
        thread_view_setup(&model->thread, model->board.board,
                          model->board.selection);
      }
    } else if (event->data == &model->latest) {
      if (model->latest.selection == NULL)
        model->state = HP_MENU;
      else {
        model->state = HP_THREAD;
        model->_from_macro_board = true;
        // Clean previous
        ct_subcleanup(&model->thread.base);
        // Build new
        thread_view_setup(&model->thread, model->latest.selection->thread_board,
                          model->latest.selection);
      }

    } else if (event->data == &model->thread) {

      if (model->_from_macro_board) {
        model->state = HP_MACRO_BOARD;
        ct_subcleanup(&model->latest.base);
        latest_listing_setup(&model->latest);
      } else {
        model->state = HP_BOARD;
        ct_subcleanup(&model->board.base);
        board_listing_setup(&model->board, model->thread.board);
      }
    } else if (event->data == &model->archive) {
      model->state = HP_MENU;
    }
  }

  // Defer to subpage
  switch (model->state) {

  case HP_MENU:
    model->menu.base.handler(&model->menu, event);
    break;
  case HP_BOARD:
    model->board.base.handler(&model->board, event);
    break;
  case HP_ARCHIVE:
  case HP_PHRAK:
    ct_subhandler(&model->archive.base, event);
    break;
  case HP_MACRO_BOARD:
    ct_subhandler(&model->latest.base, event);
    break;
  case HP_THREAD:
    ct_subhandler(&model->thread.base, event);
    break;
  }
}

void home_page_render(const void *uncasted_model, struct CtCanvas *canvas) {
  log_trace();

  const HomePage *model = uncasted_model;
  struct CtBrush backup = ct_brush();

  struct CtCanvas min_canvas;
  ct_ccutout_c(canvas, &min_canvas, 93, canvas->max_y);

  log_fmt(LOG_TRACE, L"before background");
  ct_brush_fg_hex("#053970");
  ct_brush_add_attr(A_DIM | A_ITALIC);
  model->background.base.render(&model->background, canvas);

  ct_brush_from(backup);

  switch (model->state) {
  case HP_MENU:
    log_fmt(LOG_TRACE, L"before menu");
    model->menu.base.render(&model->menu, &min_canvas);
    break;
  case HP_BOARD:
    log_fmt(LOG_TRACE, L"before board");
    model->board.base.render(&model->board, &min_canvas);
    break;
  case HP_ARCHIVE:
  case HP_PHRAK:
    ct_subrender(&model->archive.base, &min_canvas);
    break;
  case HP_MACRO_BOARD:
    ct_subrender(&model->latest.base, &min_canvas);
    break;
  case HP_THREAD:
    ct_subrender(&model->thread.base, &min_canvas);
    break;
  }
}

void home_page_setup(HomePage *model) {
  model->base.cleanup = home_page_cleanup;
  model->base.handler = home_page_handler;
  model->base.render = home_page_render;

  model->state = HP_MENU;

  stars_wall_setup(&model->background, 4, 2);
  initial_menu_setup(&model->menu);

  model->_latest_initialized = false;
  model->_board_initialized = false;
  model->_from_macro_board = false;
  model->_archive_initialized = false;

  // Build without loading anything
  thread_view_setup(&model->thread, BOARD_RANDOM, NULL);

  // Componetnst
  phrack_title_setup(&model->phrack_title);
  archive_title_setup(&model->archive_title);
}
