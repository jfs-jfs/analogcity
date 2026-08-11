#include "style.h"
#include <assert.h>
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/style/brush.h>

void stylepass_cleanup(void *uncasted_model) {
  StylePass *model = uncasted_model;
  model->child->cleanup(model->child);
}

void stylepass_handler(void *uncasted_model, const struct CtEvent *event) {
  StylePass *model = uncasted_model;
  model->child->handler(model->child, event);
}

void stylepass_render(const void *uncasted_model, struct CtCanvas *canvas) {
  const StylePass *model = uncasted_model;

  ct_brush_fg_hex(DEFAULT_FG_COLOR);
  ct_brush_bg_hex(DEFAULT_BG_COLOR);

  ct_cfill(canvas, L' ');
  model->child->render(model->child, canvas);
}

void stylepass_setup(StylePass *model, struct CtModel *child) {
  assert(NULL != child);
  model->base.render = stylepass_render;
  model->base.handler = stylepass_handler;
  model->base.cleanup = stylepass_cleanup;
  model->child = child;
}
