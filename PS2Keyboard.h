// Copyright 2012 Artem Prilutskiy

#ifndef PS2KEYBOARD_H
#define PS2KEYBOARD_H

#include <stddef.h>
#include <stdint.h>
#include "PS2Wire.h"
#include "Keyboard.h"
#include "KeyboardMap.h"

class PS2Keyboard : public Keyboard
{
  public:

    PS2Keyboard(PS2Wire& wire);

    void reset();

    bool processData(uint8_t data);

    uint16_t getSymbol();
    const char* getString();

    void clearIndicators();
    void setIndicator(uint8_t indicator, bool on);

  private:

    PS2Wire& wire;

    uint8_t indicators;
    uint32_t code;

    uint16_t modifiers;
    KeyMap key;

    char buffer[SEQUENCE_SIZE];

    void updateIndicators();

    bool searchKey();
    bool processKey();

    size_t getActionIndex();
};

#endif
