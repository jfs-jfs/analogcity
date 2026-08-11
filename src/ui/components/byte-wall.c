#include "byte-wall.h"
#include "events.h"
#include <cursed-tea/canvas.h>
#include <cursed-tea/event.h>
#include <cursed-tea/layout.h>
#include <cursed-tea/ui/common.h>
#include <stddef.h>

#define BYTES_HEIGHT 5
static const wchar_t *BYTES[] = {
    L"00000000", L"01101001", L"00110110", L"01011000", L"00001101",
};

void byte_wall_handler(void *uncasted_model, const struct CtEvent *event) {
  ByteWall *model = uncasted_model;
  if (event->type == CUSTOM_EVENT && event->data == uncasted_model) {
    switch (event->custom_signal) {
    case CUSTOM_EV_START:
      model->_running = true;
      model->_counter = 0;
      ct_event_send_custom_delayed(CUSTOM_EV_TICK, model->speed_ms,
                                   uncasted_model);
      break;
    case CUSTOM_EV_TICK:
      if (!model->_running)
        return;

      model->_counter++;
      ct_event_send_custom_delayed(CUSTOM_EV_TICK, model->speed_ms,
                                   uncasted_model);
      break;
    }
  }
}

void byte_wall_render(const void *uncasted_model, struct CtCanvas *canvas) {

  const ByteWall *model = uncasted_model;

  size_t times_x = (canvas->max_x - 1) / 9;
  size_t times_y = (canvas->max_y) / 2 + 1;

  for (size_t i = 0; i < times_y; i++)
    for (size_t j = 0; j < times_x; j++) {
      if (i * 2 > canvas->max_y - 1)
        break;
      size_t index = (i + j + model->_counter) % BYTES_HEIGHT;
      ct_cwrite(canvas, j * 9, i * 2, BYTES[index]);
    }
}

void byte_wall_setup(ByteWall *model, const size_t speed_ms) {

  model->base.cleanup = _empty_cleanup;
  model->base.handler = byte_wall_handler;
  model->base.render = byte_wall_render;

  model->speed_ms = speed_ms;
  model->_running = false;
}
