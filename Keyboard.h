// Copyright 2012 Artem Prilutskiy

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define SYMBOL_STRING  (uint16_t)0xffff

#define KANA_LOCK      8
#define CAPS_LOCK      4
#define NUM_LOCK       2
#define SCROLL_LOCK    1

class Keyboard
{
  public:

    virtual void reset() = 0;

    virtual uint16_t getSymbol() = 0;
    virtual const char* getString() = 0;

    virtual void clearIndicators();
    virtual void setIndicator(uint8_t indicator, bool on) = 0;
};

#endif
