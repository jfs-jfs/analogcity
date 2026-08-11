#include "latest-listing.h"
#include "../../file_operations.h"
#include "../components/events.h"
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
void *load_latest_threads(void *uncasted_model) {
  LatestListing *model = uncasted_model;
  model->_loaded_threads = load_thread_descriptions_all_boards(model->_threads);
  model->_loading = false;
  ct_event_send_custom(CUSTOM_EV_UPDATE, model);
  return NULL;
}

void latest_listing_cleanup(void *uncasted_model) {
  LatestListing *model = uncasted_model;
  ct_subcleanup(&model->title.base);
}

void latest_listing_handler(void *uncasted_model, const struct CtEvent *event) {
  LatestListing *model = uncasted_model;

  if (event->type == KEY_EVENT) {
    switch (event->key) {
    case L'b':
      ct_event_send_custom(CUSTOM_EV_DONE, uncasted_model);
      break;
    case L'j':
      if (model->_loading || model->_loaded_threads == 0)
        break;
      model->_cursor_at = (model->_cursor_at + 1) % model->_loaded_threads;
      break;
    case L'k':
      if (model->_loading || model->_loaded_threads == 0)
        break;
      model->_cursor_at = (model->_cursor_at == 0) ? model->_loaded_threads - 1
                                                   : model->_cursor_at - 1;
      break;
    case L'\n':
    case L'\r':
      if (model->_loading || model->_loaded_threads == 0)
        break;
      model->selection = &model->_threads[model->_cursor_at];
      ct_event_send_custom(CUSTOM_EV_DONE, uncasted_model);
      break;
    case L'Q':
    case L'q':
      ct_event_send_exit();
      return;
    }
  }
}

void latest_listing_render(const void *uncasted_model,
                           struct CtCanvas *canvas) {
  log_trace();
  const LatestListing *model = uncasted_model;

  struct CtBrush backup = ct_brush();
  struct CtCanvas threads_canvas, title_canvas;

  // Set title
  ct_cmargin_x(canvas, &title_canvas, 0, canvas->max_x - 11); // 11 title width
  ct_subrender(&model->title.base, &title_canvas);

  ct_cmargin_x(canvas, &threads_canvas, 11 + 2,
               0); // 11 lenght of title + 2 spacing
  ct_brush_from(backup);

  // KEYS BAR
  ct_brush_fg_hex(HINT_COLOR);
  ct_cwrite_ce(
      &threads_canvas,
      L"[ k/j :: Up/Down ] [ ENTER :: Select ] [ b :: Back ] [ q :: Quit ]");
  ct_brush_from(backup);
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
}

void latest_listing_setup(LatestListing *model) {
  model->base.cleanup = latest_listing_cleanup;
  model->base.render = latest_listing_render;
  model->base.handler = latest_listing_handler;
  model->_cursor_at = 0;
  model->_loaded_threads = 0;
  model->selection = NULL;

  // Titles
  latest_title_setup(&model->title);

  model->_loading = true;

  pthread_t loading_thread;
  pthread_create(&loading_thread, NULL, load_latest_threads, model);
  pthread_detach(loading_thread);
}
