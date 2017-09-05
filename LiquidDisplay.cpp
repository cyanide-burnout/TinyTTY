// Copyright 2012 Artem Prilutskiy

#include "LiquidDisplay.h"
#include "CharacterMap.h"
#include <avr/pgmspace.h>

LiquidDisplay::LiquidDisplay(LiquidCrystal& display, int width, int height) :
  display(display),
  height(height),
  width(width),
  column(0),
  row(0)
{
  display.begin(width, height);
  buffer = (char*)malloc(width * height);
  memset(buffer, ' ', width * height);
}

LiquidDisplay::~LiquidDisplay()
{
  free(buffer);
}

char LiquidDisplay::getCharacter(uint32_t code)
{
  // Character from ASCII table
  if (code < 128)
    return code;

  // Try to get character from Unicode mapping table
  for (size_t index = 0; index < CHARACTER_COUNT; index ++)
    if (pgm_read_word(&characters[index].code) == code)
      return (char)pgm_read_byte(&characters[index].character);

  // Nothing found, use default character
  return 0x16;
}

void LiquidDisplay::drawFromBuffer()
{
  char* pointer = buffer;
  for (size_t row = 0; row < height; row ++)
  {
    // We should set position of cursor for each new display row
    display.setCursor(0, row);
    for (size_t column = 0; column < width; column ++)
    {
      display.write(*pointer);
      pointer ++;
    }
  }
}

void LiquidDisplay::scrollUp()
{
  char* pointer = buffer;
  for (size_t row = 1; row < height; row ++)
  {
    memmove(pointer, pointer + width, width);
    pointer += width;
  }
  memset(pointer, ' ', width);

  drawFromBuffer();

  row = height - 1;
  column = 0;
  display.setCursor(column, row);
}

void LiquidDisplay::clear()
{
  display.clear();
  memset(buffer, ' ', width * height);

  row = 0;
  column = 0;
}

void LiquidDisplay::newLine()
{
  row ++;
  column = 0;

  if (row < height)
    display.setCursor(column, row);
  else
    scrollUp();
}

void LiquidDisplay::backSpace()
{
  column --;

  if ((column < 0) && (row > 0))
  {
    column = width - 1;
    row --;
  }

  if ((column < 0) && (row == 0))
    column = 0;

  display.setCursor(column, row);
  display.write(' ');
  buffer[width * row + column] = ' ';
  display.setCursor(column, row);
}

void LiquidDisplay::print(uint32_t code)
{
  char character = getCharacter(code);

  // HD44780 has non-linear rows addressing,
  // so we should set position of cursor for each new display row
  if ((row < height) && (column == 0))
    display.setCursor(0, row);

  // 
  if (row >= height)
    scrollUp();

  display.write(character);
  buffer[width * row + column] = character;

  column ++;

  if (column >= width)
  {
    column = 0;
    row ++;
  }
}

void LiquidDisplay::setCursor(int y, int x)
{
  if (y < 0)
    y = 0;
  if (y >= height)
    y = height - 1;

  if (x < 0)
    x = 0;
  if (x >= width)
    x = width - 1;

  row = y;
  column = x;

  display.setCursor(x, y);
}

void LiquidDisplay::cursor()
{
  display.cursor();
  display.blink();
}

void LiquidDisplay::noCursor()
{
  display.noCursor();
}

int LiquidDisplay::getColumn()
{
  return column;
}

int LiquidDisplay::getRow()
{
  return row;
}

int LiquidDisplay::getWidth()
{
  return width;
}

int LiquidDisplay::getHeight()
{
  return height;
}

