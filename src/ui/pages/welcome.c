#include "welcome.h"
#include "../components/events.h"
#include "../style.h"
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/layout.h>
#include <cursed-tea/logger.h>
#include <ncurses.h>

void welcome_page_cleanup(void *uncasted_model) {
  WelcomePage *model = uncasted_model;

  model->line1.base.cleanup(&model->line1);
  model->line2.base.cleanup(&model->line2);
  model->title.base.cleanup(&model->title);

  // Cleans model->background too no need to call ourselves
  model->background_wrapper.base.cleanup(&model->background_wrapper);
}

void welcome_page_handler(void *uncasted_model, const struct CtEvent *event) {

  // Done trigger
  if (event->type == KEY_EVENT && event->key == L'\n') {
    ct_event_send_custom(CUSTOM_EV_DONE, uncasted_model);
    return;
  }
  if (event->type == KEY_EVENT &&
      (event->key == L'\033' || event->key == L'q')) {
    ct_event_send_exit();
    return;
  }

  WelcomePage *model = uncasted_model;
  if (event->type == RESIZE_EVENT) {
    model->line1.current_y = 0;
    model->line2.current_y = event->new_size.y - 1;
    ct_event_send_custom(CUSTOM_EV_START, &model->line1);
    ct_event_send_custom(CUSTOM_EV_START, &model->line2);
  }
  // Keep track of end of animations
  else if (event->type == CUSTOM_EVENT &&
           event->custom_signal == CUSTOM_EV_DONE) {
    if (event->data == &model->line1) {
      log_debug(L"Line1 ended");
      model->_line1_done = true;
    } else if (event->data == &model->line2) {
      log_debug(L"Line2 ended");
      model->_line2_done = true;
    } else if (event->data == &model->background_wrapper) {
      log_debug(L"Background fade ended");
      model->_fade_done = true;
    }

    if (!model->_fade_started && model->_line1_done && model->_line2_done) {
      model->_fade_started = true;
      ct_event_send_custom(CUSTOM_EV_START, &model->background);
      ct_event_send_custom(CUSTOM_EV_START, &model->background_wrapper);
    }
  }

  model->line1.base.handler(&model->line1, event);
  model->line2.base.handler(&model->line2, event);
  model->background_wrapper.base.handler(&model->background_wrapper, event);
}

void welcome_page_render(const void *uncasted_model, struct CtCanvas *canvas) {
  log_trace();

  const WelcomePage *model = uncasted_model;

  struct CtBrush backup = ct_brush();

  struct CtCanvas background, end_line;
  ct_ccutout_c(canvas, &background, 93, canvas->max_y);
  ct_cmargin_x(&background, &end_line, 0, 92);

  // Lines
  ct_brush_fg_hex(BLUE_COLOR);
  model->line1.base.render(&model->line1, &background);
  model->line2.base.render(&model->line2, &end_line);

  // Background
  ct_brush_add_attr(A_BOLD);
  ct_cmargin_x(&background, &background, 0, 2);
  model->background_wrapper.base.render(&model->background_wrapper,
                                        &background);
  ct_brush_from(backup);

  // Title
  if (model->_line1_done) {
    struct CtCanvas title_canvas;
    ct_cmargin_y(canvas, &title_canvas, 11, 0);
    ct_brush_fg_hex(PINK_COLOR);
    model->title.base.render(&model->title, &title_canvas);
  }

  // Hint
  if (model->_fade_done) {
    struct CtCanvas hint;
    const size_t hint_length = 14 + 1;
    const size_t start = (background.max_x - hint_length) / 2;
    const size_t end = start + hint_length;
    wchar_t *hint_txt = L"PRESS <ENTER>";
    ct_ccutout(&background, &hint, start, background.max_y - 4, end,
               background.max_y - 1);
    ct_brush_border(B_THICK);
    ct_cborder(&hint, &hint);
    ct_brush_from(backup);
    ct_cwrite_ss(&hint, hint_txt);
  }
  ct_brush_from(backup);
}

void welcome_page_setup(WelcomePage *model) {
  model->base.cleanup = welcome_page_cleanup;
  model->base.render = welcome_page_render;
  model->base.handler = welcome_page_handler;
  model->_line1_done = false;
  model->_line2_done = false;
  model->_fade_done = false;
  model->_fade_started = false;

  title_setup(&model->title);
  growing_vline_setup(&model->line1, 20, GVL_DOWN);
  growing_vline_setup(&model->line2, 20, GVL_UP);
  byte_wall_setup(&model->background, 100);
  fader_setup(&model->background_wrapper, &model->background.base,
              DEFAULT_BG_COLOR, GREY_COLOR, 100);
}
