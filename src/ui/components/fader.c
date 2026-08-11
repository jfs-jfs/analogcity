#include "fader.h"
#include "events.h"
#include <cursed-tea/canvas.h>
#include <cursed-tea/core.h>
#include <cursed-tea/event.h>
#include <cursed-tea/logger.h>
#include <cursed-tea/style/brush.h>
#include <cursed-tea/ui/common.h>

#define FADER_STEPS 10

void fader_cleanup(void *uncasted_model) {
  Fader *model = uncasted_model;
  if (NULL != model->child)
    return;
  model->child->cleanup(model->child);
}

void fader_handler(void *uncasted_model, const struct CtEvent *event) {
  Fader *model = uncasted_model;
  if (model->child == NULL)
    return;

  if (event->type == CUSTOM_EVENT && event->data == model) {
    switch (event->custom_signal) {
    case CUSTOM_EV_START:
      log_info(L"START");
      model->_running = true;
      ct_event_send_custom_delayed(CUSTOM_EV_TICK, model->speed_ms, model);
      break;
    case CUSTOM_EV_TICK:
      log_info(L"TICK");
      if (model->_step >= FADER_STEPS) {
        model->_done = true;
        ct_event_send_custom(CUSTOM_EV_DONE, model);
        break;
      }
      if (!model->_running || model->_done)
        break;
      model->_step++;
      ct_event_send_custom_delayed(CUSTOM_EV_TICK, model->speed_ms, model);
    }
  }

  model->child->handler(model->child, event);
}

void fader_render(const void *uncasted_model, struct CtCanvas *canvas) {
  const Fader *model = uncasted_model;

  if (model->child == NULL)
    return;

  const struct CtBrush backup = ct_brush();

  if (!model->_running)
    return;

  if (model->_done) {
    ct_brush_fg(model->to.r, model->to.g, model->to.b);
    model->child->render(model->child, canvas);
    ct_brush_from(backup);
    return;
  }

  struct CtRGB step;
  step.r = model->from.r +
           (model->to.r - model->from.r) * model->_step / FADER_STEPS;
  step.g = model->from.g +
           (model->to.g - model->from.g) * model->_step / FADER_STEPS;
  step.b = model->from.b +
           (model->to.b - model->from.b) * model->_step / FADER_STEPS;
  ct_brush_fg(step.r, step.g, step.b);
  model->child->render(model->child, canvas);
  ct_brush_from(backup);
}

void fader_setup(Fader *model, struct CtModel *child, const char *from,
                 const char *to, const size_t speed_ms) {
  model->base.cleanup = fader_cleanup;
  model->base.handler = fader_handler;
  model->base.render = fader_render;

  if (!parse_hex_color(from, &model->from))
    log_crit(L"fader: unable to parse from color");

  if (!parse_hex_color(to, &model->to))
    log_crit(L"fader: unable to parse to color");

  model->speed_ms = speed_ms;
  model->child = child;
  model->_step = 0;
  model->_running = false;
  model->_done = false;
}
