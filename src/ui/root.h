#pragma once

#include "pages/home.h"
#include "pages/welcome.h"
#include <cursed-tea.h>
#include <stddef.h>

#define MIN_UI_X_SIZE 95
#define MIN_UI_Y_SIZE 30

enum ProgramState {
  INITIAL_STATE,
  HOME_STATE,
  PRE_EXIT_STATE,
};

typedef struct Root {
  struct CtModel base;

  enum ProgramState state;

  // Pages
  WelcomePage welcome;
  HomePage home;
} Root;

void root_setup(Root *model);
