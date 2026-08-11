#include "board.h"
#include "../../file_operations.h"
#include "../components/events.h"
#include "../components/highway-title.h"
#include "../style.h"
#include <bits/pthreadtypes.h>
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/core.h>
#include <cursed-tea/event.h>
#include <cursed-tea/helpers.h>
#include <cursed-tea/layout.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/style/border.h>
#include <cursed-tea/style/brush.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

// To load them in parallel
void *load_board_threads(void *uncasted_model) {
  BoardListing *model = uncasted_model;
  model->_loaded_threads =
      load_threads_descriptions(model->_board_name, model->_threads);
  model->_loading = false;
  ct_event_send_custom(CUSTOM_EV_UPDATE, model);
  return NULL;
}

void board_listing_cleanup(void *uncasted_model) {
  BoardListing *model = uncasted_model;

  ct_subcleanup(&model->highway_title.base);
  ct_subcleanup(&model->meta_title.base);
  ct_subcleanup(&model->meatspace_title.base);
  ct_subcleanup(&model->random_title.base);
  ct_subcleanup(&model->tech_title.base);

  ct_subcleanup(&model->new_thread_form.base);
}

void board_listing_handler(void *uncasted_model, const struct CtEvent *event) {
  BoardListing *model = uncasted_model;

  if (model->_show_creation_form) {
    if (event->type == CUSTOM_EVENT && event->custom_signal == CUSTOM_EV_DONE &&
        event->data == &model->new_thread_form) {

      model->_show_creation_form = false;

      if (model->new_thread_form.created_thread) {
        // Rebuild to show the new thread file
        board_listing_setup(model, model->board);
      }

    } else
      model->new_thread_form.base.handler(&model->new_thread_form, event);
    return;
  }

  if (event->type == KEY_EVENT) {
    switch (event->key) {
    case L'b':
      ct_event_send_custom(CUSTOM_EV_DONE, uncasted_model);
      break;
    case L'j':
      model->_cursor_at = (model->_cursor_at + 1) % model->_loaded_threads;
      break;
    case L'k':
      model->_cursor_at = (model->_cursor_at == 0) ? model->_loaded_threads - 1
                                                   : model->_cursor_at - 1;
      break;
    case L'\n':
    case L'\r':
      model->selection = &model->_threads[model->_cursor_at];
      ct_event_send_custom(CUSTOM_EV_DONE, uncasted_model);
      break;
    case L'n':
      model->_show_creation_form = true;
      ct_subcleanup(&model->new_thread_form.base);
      thread_creation_form_setup(&model->new_thread_form, model->board);
      break;
    case L'\033': // ESC
    case L'Q':
    case L'q':
      ct_event_send_exit();
      return;
    }
  }
}

void board_listing_render(const void *uncasted_model, struct CtCanvas *canvas) {
  log_trace();
  const BoardListing *model = uncasted_model;

  // Set correct title
  switch (model->board) {
  case BOARD_HIGHWAY:
    model->highway_title.base.render(&model->highway_title, canvas);
    break;
  case BOARD_MEATSPACE:
    model->meatspace_title.base.render(&model->meatspace_title, canvas);
    break;
  case BOARD_META:
    model->meta_title.base.render(&model->meta_title, canvas);
    break;
  case BOARD_RANDOM:
    model->random_title.base.render(&model->random_title, canvas);
    break;
  case BOARD_TECHNOLOGY:
    model->tech_title.base.render(&model->tech_title, canvas);
    break;
  }

  struct CtCanvas threads_canvas;
  struct CtBrush backup = ct_brush();
  ct_cmargin_x(canvas, &threads_canvas, 0,
               11 + 2); // 11 lenght of title + 2 spacing
  ct_brush_from(backup);

  // KEYS BAR
  if (!model->_show_creation_form) {
    ct_brush_fg_hex(HINT_COLOR);
    ct_cwrite_ce(&threads_canvas, L"[ k/j :: Up/Down ] [ ENTER :: Select ] [ n "
                                  L":: New ] [ b :: Back ] [ q :: Quit ]");
    ct_brush_from(backup);
  }

  // Not early return for in case of new thread form
  if (model->_loading) {
    ct_cwrite_cc(canvas, L"LOADING");
  } else if (model->_loaded_threads == 0) {
    ct_cwrite_cc(canvas, L"[NO THREADS YET]");
  } else {
    // Display thread list
    size_t start_y_threads = (canvas->max_y - THREADS_TO_DISPLAY_PER_BOARD) / 2;
    for (size_t i = 0; i < model->_loaded_threads; i++) {
      const size_t buffer_length = threads_canvas.max_x - 1 - 2;
      wchar_t buffer[buffer_length + 1];
      buffer[buffer_length] = L'\0';

      if (i == (size_t)model->_cursor_at) {
        ct_brush_fg_hex(PINK_COLOR);
        ct_brush_add_attr(A_BOLD);
        ct_cwrite_char(&threads_canvas, 0, start_y_threads + i, L'>');
        ct_cwrite_char(&threads_canvas, threads_canvas.max_x - 5,
                       start_y_threads + i, L'<');
        ct_brush_attr(A_ITALIC);
      }

      // Format time
      struct tm time_info;
      time_t t = model->_threads[i].last_reply_timestamp;
      localtime_r(&t, &time_info);
      char time_str[21];
      strftime(time_str, sizeof(time_str), "%d/%m/%Y %H:%M:%S", &time_info);
      wchar_t wtime_str[21];
      mbstowcs(wtime_str, time_str, 21);

      // Format string
      swprintf(buffer, buffer_length, L"%03zu :: [%-*.*ls] :: %ls",
               model->_threads[i].number_of_replies,
               (int)THREAD_TILE_DISPLAY_MAX, (int)THREAD_TILE_DISPLAY_MAX,
               model->_threads[i].thread_title, wtime_str);

      // Draw string
      ct_cwrite(&threads_canvas, 2, start_y_threads + i, buffer);
      ct_brush_from(backup);
    }
  }

  if (model->_show_creation_form) {
    model->new_thread_form.base.render(&model->new_thread_form,
                                       &threads_canvas);
  }
}

void board_listing_setup(BoardListing *model, const enum Boards board) {
  model->base.cleanup = board_listing_cleanup;
  model->base.render = board_listing_render;
  model->base.handler = board_listing_handler;
  model->board = board;
  model->_cursor_at = 0;
  model->selection = NULL;

  // Titles
  highway_title_setup(&model->highway_title);
  meatspace_title_setup(&model->meatspace_title);
  meta_title_setup(&model->meta_title);
  random_title_setup(&model->random_title);
  technology_title_setup(&model->tech_title);

  // Thread creation form
  thread_creation_form_setup(&model->new_thread_form, model->board);

  switch (model->board) {
  case BOARD_HIGHWAY:
    strncpy(model->_board_name, "boards/highway", BOARD_MAX_NAME);
    break;
  case BOARD_MEATSPACE:
    strncpy(model->_board_name, "boards/meatspace", BOARD_MAX_NAME);
    break;
  case BOARD_RANDOM:
    strncpy(model->_board_name, "boards/random", BOARD_MAX_NAME);
    break;
  case BOARD_TECHNOLOGY:
    strncpy(model->_board_name, "boards/technology", BOARD_MAX_NAME);
    break;
  case BOARD_META:
    strncpy(model->_board_name, "boards/meta", BOARD_MAX_NAME);
    break;
  }
  model->_board_name[BOARD_MAX_NAME] = '\0';
  model->_loading = true;
  model->_show_creation_form = false;

  pthread_t loading_thread;
  pthread_create(&loading_thread, NULL, load_board_threads, model);
  pthread_detach(loading_thread);
}
