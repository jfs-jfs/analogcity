#include "archive-entry-view.h"
#include "../../file_operations.h"
#include "../components/events.h"
#include "../style.h"
#include <bits/pthreadtypes.h>
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/layout.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/style/border.h>
#include <cursed-tea/style/brush.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>

void *load_archive_file_parallel(void *uncasted_model) {
  ArchiveEntryView *model = uncasted_model;
  load_archive_file(model->archive_directory, model->entry_file,
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

void archive_entry_view_cleanup(void *uncasted_model) {
  ArchiveEntryView *model = uncasted_model;

  if (NULL != model->_dynamic_buffer)
    free(model->_dynamic_buffer);
}

void archive_entry_view_handler(void *uncasted_model,
                                const struct CtEvent *event) {
  if (event->type != KEY_EVENT)
    return;

  ArchiveEntryView *model = uncasted_model;

  switch (event->key) {
  case L'b':
    ct_event_send_custom(CUSTOM_EV_DONE, model);
    break;
  case L'q':
    ct_event_send_exit();
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
  case L'k':
    if (model->_scroll_offset > 0)
      model->_scroll_offset--;
    break;
  case L'j': {
    size_t max_scroll = model->_total_lines > model->_view_height
                            ? model->_total_lines - model->_view_height
                            : 0;
    if (model->_scroll_offset < max_scroll)
      model->_scroll_offset++;
    break;
  }
  }
}

void _draw_entry_file(const ArchiveEntryView *model, struct CtCanvas *canvas) {
  ((ArchiveEntryView *)model)->_view_height = canvas->max_y;

  // Oracle made
  size_t max_scroll = model->_total_lines > canvas->max_y
                          ? model->_total_lines - canvas->max_y
                          : 0;
  size_t scroll_offset =
      model->_scroll_offset > max_scroll ? max_scroll : model->_scroll_offset;

  ct_cfill(canvas, L' ');

  size_t line_width = canvas->max_x;
  ((ArchiveEntryView *)model)->_total_lines =
      count_visual_rows(model->_dynamic_buffer, line_width);
  size_t display_line = 0;
  size_t pos = 0;

  while (pos < model->_buffer_length &&
         display_line < scroll_offset + canvas->max_y) {
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
      cwrite_parsed_dialog_colors(canvas, 0, display_line - scroll_offset,
                                  line_buf);
    }

    display_line++;
    if (pos < model->_buffer_length && model->_dynamic_buffer[pos] == L'\n')
      pos++;
  }
}

void archive_entry_view_render(const void *uncasted_model,
                               struct CtCanvas *canvas) {
  const ArchiveEntryView *model = uncasted_model;
  const struct CtBrush backup = ct_brush();

  // Shortcut hint
  ct_cwrite_ce(canvas, L"[ k/j :: Up/Down ] [ b :: Back ] [ q :: Quit ]");

  // Calc realstate
  struct CtCanvas panel_canvas, text_canvas, panel_text_canvas;
  ct_cmargin_y(canvas, &panel_canvas, 4, 2);
  ct_cmargin_y(&panel_canvas, &text_canvas, 1, 1);

  ct_cborder(&panel_canvas, NULL);

  ct_brush_bg_hex(PINK_COLOR);
  ct_brush_fg_hex(DEFAULT_BG_COLOR);
  ct_brush_add_attr(A_BOLD);

  // Panel top bar
  ct_cwrite_char_line(&panel_canvas, 0, 0, panel_canvas.max_x - 1, 0, L' ');
  ct_cmargin_x(&panel_canvas, &panel_text_canvas, 1, 1);

  wchar_t title_buffer[MAX_PATH_LENGTH + 5];
  swprintf(title_buffer, MAX_PATH_LENGTH + 4, L"[ ARCHIVE ] :: [ %.*ls ]",
           THREAD_TILE_DISPLAY_MAX, model->entry_file);
  ct_cwrite_ss(&panel_text_canvas, title_buffer);

  // Panel bottom bar
  ((ArchiveEntryView *)uncasted_model)->_total_lines =
      count_visual_rows(model->_dynamic_buffer, text_canvas.max_x);
  size_t max_scroll = model->_total_lines > text_canvas.max_y
                          ? model->_total_lines - text_canvas.max_y
                          : 0;
  size_t scroll_offset =
      model->_scroll_offset > max_scroll ? max_scroll : model->_scroll_offset;
  size_t scroll_pct = max_scroll > 0 ? (scroll_offset * 100) / max_scroll : 0;
  wchar_t scroll_text[32];
  swprintf(scroll_text, 32, L"[ %03zu%% ] ", scroll_pct);
  ct_cwrite_char_line(&panel_canvas, 0, panel_canvas.max_y - 1,
                      panel_canvas.max_x - 1, panel_canvas.max_y - 1, L' ');
  ct_cwrite_ce(&panel_canvas, scroll_text);

  ct_brush_from(backup);

  // Display file text
  if (!model->_loaded_contents)
    return;
  _draw_entry_file(model, &text_canvas);
}

void archive_entry_view_setup(ArchiveEntryView *model, const wchar_t *directory,
                              const wchar_t *file) {
  model->base.cleanup = archive_entry_view_cleanup;
  model->base.render = archive_entry_view_render;
  model->base.handler = archive_entry_view_handler;

  model->_dynamic_buffer = NULL;
  model->_loaded_contents = false;
  model->_scroll_offset = 0;
  model->_total_lines = 0;
  model->_buffer_length = 0;
  model->_view_height = 0;

  if (directory == NULL || file == NULL || wcslen(directory) == 0 ||
      wcslen(file) == 0) {
    log_error(L"archive_entry_view_setup: directory or file are NULL or have "
              L"length 0");
    model->archive_directory[0] = L'\0';
    model->entry_file[0] = L'\0';
    return;
  }

  wcsncpy(model->archive_directory, directory, MAX_PATH_LENGTH);
  wcsncpy(model->entry_file, file, MAX_PATH_LENGTH);

  pthread_t loading_thread;
  pthread_create(&loading_thread, NULL, load_archive_file_parallel, model);
  pthread_detach(loading_thread);
}
