#include "growing-vline.h"
#include "events.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/ui/common.h>
#include <stddef.h>

#define LINE_CHAR L'┃'

void growing_vline_handler(void *uncasted_model, const struct CtEvent *event) {

  GrowingVLine *model = uncasted_model;

  if (event->type == CUSTOM_EVENT && event->custom_signal == CUSTOM_EV_START) {
    model->_started = true;

    ct_event_send_custom_delayed(CUSTOM_EV_TICK, model->tick_speed_ms, model);
  } else if (event->type == CUSTOM_EVENT &&
             event->custom_signal == CUSTOM_EV_TICK && event->data == model) {
    if (model->current_y == model->_end_y) {
      ct_event_send_custom(CUSTOM_EV_DONE, model);
      return;
    }

    model->current_y =
        model->dir == GVL_UP ? model->current_y - 1 : model->current_y + 1;
    ct_event_send_custom_delayed(CUSTOM_EV_TICK, model->tick_speed_ms, model);
  }
}

void growing_vline_render(const void *uncasted_model, struct CtCanvas *canvas) {
  const GrowingVLine *model = uncasted_model;
  GrowingVLine *mutable_model = (void *)uncasted_model;
  mutable_model->_end_y = model->dir == GVL_UP ? 0 : canvas->max_y - 1;

  if (!model->_started)
    return;

  if (model->dir == GVL_UP) {
    for (size_t i = model->current_y; i < canvas->max_y; i++) {
      ct_cwrite_char(canvas, 0, i, LINE_CHAR);
    }
  } else {
    for (size_t i = 0; i <= model->current_y; i++) {
      ct_cwrite_char(canvas, 0, i, LINE_CHAR);
    }
  }
}

void growing_vline_setup(GrowingVLine *model, const size_t tick_speed_ms,
                         const enum GrowVLineDirection dir) {
  model->base.cleanup = _empty_cleanup;
  model->base.render = growing_vline_render;
  model->base.handler = growing_vline_handler;

  model->tick_speed_ms = tick_speed_ms;
  model->dir = dir;
  model->current_y = 0;
  model->_started = false;
}
