// Copyright 2012 Artem Prilutskiy

#ifndef ESCAPEHANDLER_H
#define ESCAPEHANDLER_H

#include <stddef.h>
#include <stdint.h>

#include <Print.h>
#include "Display.h"
#include "Keyboard.h"

typedef Print Link;

// ANSI ECMA-48, VT-100

#define ESCAPE_SEQUENCE_LENGTH  16

class EscapeHandler
{
  public:

    EscapeHandler(Display& display, Keyboard& keyboard, Link& link);

    bool processData(uint32_t data);

  private:

    char sequence[ESCAPE_SEQUENCE_LENGTH];
    size_t length;

    Display& display;
    Keyboard& keyboard;
    Link& link;

    int row;
    int column;

    void processSequence();

};

#endif
