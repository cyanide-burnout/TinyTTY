// Copyright 2012 Artem Prilutskiy

#ifndef LIQUIDDISPLAY_H
#define LIQUIDDISPLAY_H

#include <LiquidCrystal.h>
#include <stdint.h>
#include <stddef.h>
#include "Display.h"

class LiquidDisplay : public Display
{
  public:

    LiquidDisplay(LiquidCrystal& display, int width, int height);
    ~LiquidDisplay();

    void clear();
    void newLine();
    void backSpace();
    void print(uint32_t code);

    void setCursor(int y, int x);

    void cursor();
    void noCursor();

    int getColumn();
    int getRow();

    int getWidth();
    int getHeight();

  private:

    LiquidCrystal& display;
    int width;
    int height;

    char* buffer;
    int column;
    int row;

    char getCharacter(uint32_t code);

    void drawFromBuffer();
    void scrollUp();

};

#endif
