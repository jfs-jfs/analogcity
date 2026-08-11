#include "root.h"
#include "components/events.h"
#include "pages/welcome.h"
#include <cursed-tea/event.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

void root_cleanup(void *uncasted_model) {
  Root *model = uncasted_model;

  // Clean pages
  model->welcome.base.cleanup(&model->welcome);
}

void root_handler(void *uncasted_model, const struct CtEvent *event) {
  // SHOULD BE DONE BY EACH COMPONENT
  // if (event->type == KEY_EVENT && event->key == L'\033') {
  //   ct_event_send_exit();
  //   return;
  // }
  // if (event->type == KEY_EVENT && event->key == L'q') {
  //   ct_event_send_exit();
  //   return;
  // }

  Root *model = uncasted_model;

  // Handle change of state
  // - WelcomePage
  if (event->type == CUSTOM_EVENT && event->custom_signal == CUSTOM_EV_DONE &&
      event->data == &model->welcome) {
    model->state = HOME_STATE;
    return;
  }

  switch (model->state) {
  case INITIAL_STATE:
    model->welcome.base.handler(&model->welcome, event);
    break;

  case PRE_EXIT_STATE:
    ct_event_send_exit();
    break;

  case HOME_STATE:
    model->home.base.handler(&model->home, event);
    break;
  }
}

void root_render(const void *uncasted_model, struct CtCanvas *canvas) {
  const Root *model = uncasted_model;
  switch (model->state) {

  case INITIAL_STATE:
    model->welcome.base.render(&model->welcome, canvas);
    break;
  case PRE_EXIT_STATE:
    break;
  case HOME_STATE:
    model->home.base.render(&model->home, canvas);
    break;
  }
}

void root_setup(Root *model) {
  model->base.cleanup = root_cleanup;
  model->base.render = root_render;
  model->base.handler = root_handler;
  model->state = INITIAL_STATE;

  // Setup Pages
  welcome_page_setup(&model->welcome);
  home_page_setup(&model->home);
}
