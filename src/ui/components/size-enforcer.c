#include "size-enforcer.h"
#include <cursed-tea/canvas-write.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/helpers.h>
#include <wchar.h>

void size_enforcer_cleanup(void *uncasted_model) {
  SizeEnforcer *model = uncasted_model;
  ct_subcleanup(model->child);
}

void size_enforcer_handler(void *uncasted_model, const struct CtEvent *event) {
  SizeEnforcer *model = uncasted_model;

  if (event->type == RESIZE_EVENT)
    model->_can_render =
        model->min_x <= event->new_size.x && model->min_y <= event->new_size.y;

  if (model->_can_render)
    return ct_subhandler(model->child, event);

  if (event->type == KEY_EVENT && (event->key == L'q' || event->key == L'\033'))
    ct_event_send_exit();
}

void size_enforcer_render(const void *uncasted_model, struct CtCanvas *canvas) {
  const SizeEnforcer *model = uncasted_model;

  if (model->_can_render)
    return ct_subrender(model->child, canvas);

  wchar_t buffer[120];
  if (canvas->max_x < model->min_x)
    swprintf(buffer, 120, L"[ TERMINAL TO SLIM :: EXPAND IT %zu COLUMNS ]",
             model->min_x - canvas->max_x);
  else
    swprintf(buffer, 120, L"[ TERMINAL TO SHORT :: EXPAND IT %zu ROWS ]",
             model->min_y - canvas->max_y);

  ct_cwrite_cc(canvas, buffer);
}

void size_enforcer_setup(SizeEnforcer *model, const size_t min_x,
                         const size_t min_y, struct CtModel *child) {
  model->child = child;
  model->min_x = min_x;
  model->min_y = min_y;
  model->_can_render = true;

  model->base.cleanup = size_enforcer_cleanup;
  model->base.render = size_enforcer_render;
  model->base.handler = size_enforcer_handler;
}
