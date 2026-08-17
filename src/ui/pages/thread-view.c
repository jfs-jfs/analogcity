#include "thread-view.h"
#include "../../file_operations.h"
#include "../components/events.h"
#include "../style.h"
#include <bits/pthreadtypes.h>
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
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
#include <time.h>
#include <unistd.h>
#include <wchar.h>

void *load_thread_file_parallel(void *uncasted_model) {
  ThreadView *model = uncasted_model;
  char board_name[BOARD_MAX_NAME + 1];
  to_board_name(model->board, board_name);
  board_name[BOARD_MAX_NAME] = '\0';

  load_thread_file(board_name, model->thread.thread_title,
                   &model->_dynamic_buffer);

  if (model->_dynamic_buffer == NULL) {
    log_fmt(LOG_ERR, L"load_thread_file_parallel: did not receive a proper "
                     L"initialized buffer");
    return NULL;
  }

  // Calculate the number of lines and buffer length
  model->_total_lines = 0;
  model->_buffer_length = 0;
  wchar_t last = L'\0';
  for (const wchar_t *current_char = model->_dynamic_buffer; *current_char;
       current_char++, model->_buffer_length++) {
    if (*current_char == L'\n')
      model->_total_lines++;
    last = *current_char;
  }
  if (model->_total_lines == 0 || last != L'\n')
    model->_total_lines++;

  model->_loaded_contents = true;
  ct_event_send_custom(CUSTOM_EV_UPDATE, &model);
  return NULL;
}

void thread_view_cleanup(void *uncasted_model) {
  log_trace();
  ThreadView *model = uncasted_model;

  if (NULL != model->_dynamic_buffer)
    free(model->_dynamic_buffer);

  ct_subcleanup(&model->highway_title.base);
  ct_subcleanup(&model->meta_title.base);
  ct_subcleanup(&model->meatspace_title.base);
  ct_subcleanup(&model->random_title.base);
  ct_subcleanup(&model->tech_title.base);

  if (model->_shown_reply_form)
    ct_subcleanup(&model->reply_form.base);
}

void thread_view_handler(void *uncasted_model, const struct CtEvent *event) {
  log_trace();
  ThreadView *model = uncasted_model;

  if (model->_show_reply_form) {
    if (event->type == CUSTOM_EVENT && event->custom_signal == CUSTOM_EV_DONE &&
        event->data == &model->reply_form) {
      model->_show_reply_form = false;

      // Rebuild to see changes
      if (model->reply_form.created_reply) {
        const enum Boards board = model->board;
        ThreadInfo thread_info;
        thread_info_copy(&model->thread, &thread_info);
        thread_view_cleanup(model);
        thread_view_setup(model, board, &thread_info);
      }
    } else
      ct_subhandler(&model->reply_form.base, event);
    return;
  }

  if (event->type == KEY_EVENT) {
    switch (event->key) {
    case L'b':
      ct_event_send_custom(CUSTOM_EV_DONE, model);
      break;
    case L'q':
      ct_event_send_exit();
      return;
    case L'j': {
      size_t max_scroll = model->_total_lines > model->_view_height
                              ? model->_total_lines - model->_view_height
                              : 0;
      if (model->_scroll_offset < max_scroll)
        model->_scroll_offset++;
      break;
    }
    case L'k':
      if (model->_scroll_offset > 0)
        model->_scroll_offset--;
      break;
    case L'g':
      model->_scroll_offset = 0;
      break;
    case L'G': {
      size_t max_scroll = model->_total_lines > model->_view_height
                              ? model->_total_lines - model->_view_height
                              : 0;
      model->_scroll_offset = max_scroll;
      break;
    }
    case L'r':
      if (model->thread.number_of_replies >= THREAD_MAX_REPLIES)
        break;
      model->_show_reply_form = true;
      reply_form_setup(&model->reply_form, model->board, &model->thread);
      model->_shown_reply_form = true;
      break;
    }
  }
}

void thread_view_render(const void *uncasted_model, struct CtCanvas *canvas) {
  log_trace();
  const ThreadView *model = uncasted_model;

  struct CtCanvas thread_canvas, title_canvas;
  struct CtBrush backup = ct_brush();

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

  ct_cmargin_x(canvas, &thread_canvas, 0, 11 + 2);

  size_t content_height = thread_canvas.max_y > 5 ? thread_canvas.max_y - 5 : 0;
  ((ThreadView *)uncasted_model)->_view_height = content_height;
  size_t line_width = thread_canvas.max_x;
  ((ThreadView *)uncasted_model)->_total_lines =
      count_visual_rows(model->_dynamic_buffer, line_width);
  size_t scroll_offset = model->_scroll_offset;
  size_t max_scroll = (model->_total_lines > content_height)
                          ? model->_total_lines - content_height
                          : 0;
  if (scroll_offset > max_scroll)
    scroll_offset = max_scroll;

  if (!model->_show_reply_form) {
    ct_brush_fg_hex(HINT_COLOR);
    if (model->thread.number_of_replies < THREAD_MAX_REPLIES)
      ct_cwrite_ce(
          &thread_canvas,
          L"[ k/j :: Up/Down ] [ r :: Reply ] [ b :: Back ] [ q :: Quit ]");
    else
      ct_cwrite_ce(
          &thread_canvas,
          L"[ k/j :: Up/Down ] [ MAX REPLIES ] [ b :: Back ] [ q :: Quit ]");
    ct_brush_from(backup);
  }
  ct_cmargin_y(&thread_canvas, &thread_canvas, 1, 1);

  ct_brush_bg_hex(BLUE_COLOR);
  ct_brush_fg_hex(DEFAULT_BG_COLOR);
  ct_brush_add_attr(A_BOLD);

  // TOP BAR
  ct_cwrite_char_line(&thread_canvas, 0, 0, thread_canvas.max_x - 1, 0, L' ');
  ct_cmargin_x(&thread_canvas, &title_canvas, 1, 1);

  // Title
  wchar_t title_buffer[THREAD_TILE_DISPLAY_MAX + 10];
  swprintf(title_buffer, THREAD_TILE_DISPLAY_MAX + 10, L"[%.*ls]",
           (int)THREAD_TILE_DISPLAY_MAX, model->thread.thread_title);
  ct_cwrite_cs(&title_canvas, title_buffer);

  // Author
  wchar_t author_buffer[AUTHOR_MAX_NAME + 10];
  swprintf(author_buffer, AUTHOR_MAX_NAME + 10, L"[%ls]",
           model->thread.thread_author);
  ct_cwrite_ss(&title_canvas, author_buffer);

  // Last reply time
  struct tm time_info;
  time_t last_timestamp = model->thread.last_reply_timestamp;
  localtime_r(&last_timestamp, &time_info);
  wchar_t time_string[64];
  wcsftime(time_string, 64, L"[%d-%m-%Y %H:%M:%S]", &time_info);
  ct_cwrite_es(&title_canvas, time_string);

  // BOTTOM BAR
  ct_cmargin_y(&thread_canvas, &thread_canvas, 0, 1);
  size_t scroll_pct = max_scroll > 0 ? (scroll_offset * 100) / max_scroll : 0;
  wchar_t scroll_text[32];
  swprintf(scroll_text, 32, L"[ %03zu%% ] ", scroll_pct);
  ct_cwrite_char_line(&thread_canvas, 0, thread_canvas.max_y - 1,
                      thread_canvas.max_x - 1, thread_canvas.max_y - 1, L' ');
  ct_cwrite_ce(&thread_canvas, scroll_text);

  ct_brush_from(backup);

  if (!model->_loaded_contents) {
    ct_cwrite_cc(canvas, L"[LOADING]");
    return;
  }

  ct_cmargin_y(&thread_canvas, &thread_canvas, 1, 1);

  // Lateral Lines
  // ct_brush_fg_hex(BLUE_COLOR);
  // ct_cwrite_char_line(&thread_canvas, 0, 0, 0, thread_canvas.max_y - 1,
  // L'┃'); ct_cwrite_char_line(&thread_canvas, thread_canvas.max_x - 1, 0,
  //                     thread_canvas.max_x - 1, thread_canvas.max_y - 1,
  //                     L'┃');
  // ct_brush_from(backup);

  ct_cmargin_x(&thread_canvas, &thread_canvas, 1, 1);
  ct_cfill(&thread_canvas, L' ');
  // ct_cborder(&thread_canvas, NULL);

  size_t display_line = 0;
  size_t pos = 0;

  while (pos < model->_buffer_length &&
         display_line < scroll_offset + thread_canvas.max_y) {
    size_t line_start = pos;
    size_t line_len = 0;
    while (pos < model->_buffer_length &&
           model->_dynamic_buffer[pos] != L'\n' && line_len < line_width) {
      pos++;
      line_len++;
    }

    if (display_line >= scroll_offset) {
      wchar_t line_buf[line_width + 1];
      wcsncpy(line_buf, &model->_dynamic_buffer[line_start], line_len);
      line_buf[line_len] = L'\0';
      cwrite_parsed_dialog_colors(&thread_canvas, 0,
                                  display_line - scroll_offset, line_buf);
      // ct_cwrite(&thread_canvas, 0, display_line - scroll_offset, line_buf);
    }

    display_line++;
    if (pos < model->_buffer_length && model->_dynamic_buffer[pos] == L'\n')
      pos++;
  }

  if (model->_show_reply_form) {
    ct_subrender(&model->reply_form.base, canvas);
  }
}

void thread_view_setup(ThreadView *model, const enum Boards board,
                       ThreadInfo *info) {
  log_trace();
  model->base.cleanup = thread_view_cleanup;
  model->base.handler = thread_view_handler;
  model->base.render = thread_view_render;

  model->board = board;
  model->_dynamic_buffer = NULL;
  model->_loaded_contents = false;
  model->_scroll_offset = 0;
  model->_total_lines = 0;
  model->_buffer_length = 0;
  model->_view_height = 0;
  model->_show_reply_form = false;
  model->_shown_reply_form = false;

  highway_title_setup(&model->highway_title);
  meatspace_title_setup(&model->meatspace_title);
  random_title_setup(&model->random_title);
  meta_title_setup(&model->meta_title);
  technology_title_setup(&model->tech_title);

  // reply_form_setup(&model->reply_form, board, NULL);

  if (info == NULL)
    return;

  thread_info_copy(info, &model->thread);

  pthread_t loading_thread;
  pthread_create(&loading_thread, NULL, load_thread_file_parallel, model);
  pthread_detach(loading_thread);
}
