// Copyright 2012 Artem Prilutskiy

#include "EscapeHandler.h"
#include <avr/pgmspace.h>

#define BUFFER_SIZE  16

EscapeHandler::EscapeHandler(Display& display, Keyboard& keyboard, Link& link) :
  display(display),
  keyboard(keyboard),
  link(link),
  length(0),
  column(0),
  row(0)
{

}

void EscapeHandler::processSequence()
{
  // Set zero-terminaltor to escape sequence
  sequence[length] = '\0';
  // Reset counter to set state machine to initial state
  length = 0;

  // http://ascii-table.com/ansi-escape-sequences.php
  // http://ascii-table.com/ansi-escape-sequences-vt-100.php
  // http://en.wikipedia.org/wiki/ANSI_escape_code
  // http://www.braun-home.net/michael/info/misc/VT100_commands.htm
  // http://graphcomp.com/info/specs/ansi_col.html
  // http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
  // http://nixdoc.net/man-pages/HP-UX/man5/dtterm.5.html

  // Request to identify terminal type
  if ((strcmp_P(sequence, PSTR("\e[c")) == 0) ||
      (strcmp_P(sequence, PSTR("\e[0c")) == 0) ||
      (strcmp_P(sequence, PSTR("\eZ")) == 0))
  {
    link.print("\e[?1;0c");
    return;
  }

  // Request for terminal status
  if (strcmp_P(sequence, PSTR("\e[5n")) == 0)
  {
    link.print("\e[0n");
    return;
  }

  // Report cursor position
  if (strcmp_P(sequence, PSTR("\e[6n")) == 0)
  {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "\e[%d;%dR",
      display.getRow(),
      display.getColumn());
    link.print(buffer);
    return;
  }

  // Report the size of the area in characters
  if (strcmp_P(sequence, PSTR("\e[18t")) == 0)
  {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "\e[8;%d;%dt",
      display.getHeight(),
      display.getWidth());
    link.print(buffer);
    return;
  }

  // Clear screen
  if (strcmp_P(sequence, PSTR("\e[2J")) == 0)
  {
    display.clear();
    return;
  }

  // Clear to the end of line
  if (strcmp_P(sequence, PSTR("\e[K")) == 0)
  {
    int column = display.getColumn();
    for (int index = column; index < display.getWidth(); index ++)
      display.print(' ');
    display.setCursor(display.getRow(), column);
  }

  // Show cursor
  if (strcmp_P(sequence, PSTR("\e?25h")) == 0)
  {
    display.cursor();
    return;
  }

  // Hide cursor
  if (strcmp_P(sequence, PSTR("\e[?25l")) == 0)
  {
    display.noCursor();
    return;
  }

  // Save cursor
  if ((strcmp_P(sequence, PSTR("\e[s")) == 0) ||
      (strcmp_P(sequence, PSTR("\e7")) == 0))
  {
    row = display.getRow();
    column = display.getColumn();
    return;
  }

  // Restore cursor
  if ((strcmp_P(sequence, PSTR("\e[u")) == 0) ||
      (strcmp_P(sequence, PSTR("\e8")) == 0))
  {
    display.setCursor(row, column);
    return;
  }

  // Cursor manipulations

  int row = 0;
  int column = 0;

  // Set the cursor position  
  if (sscanf(sequence, "\e[%d;%dH", row, column) >= 0)
  {
    display.setCursor(row, column);
    return;
  }

  int number = 1;

  // Move cursor up
  if ((strcmp_P(sequence, PSTR("\eA")) == 0) ||
      (sscanf(sequence, "\e[%dA", number) >= 0))
  {
    display.setCursor(display.getRow() - number, display.getColumn());
    return;
  }

  // Move cursor down
  if ((strcmp_P(sequence, PSTR("\eB")) == 0) ||
      (sscanf(sequence, "\e[%dB", number) >= 0))
  {
    display.setCursor(display.getRow() + number, display.getColumn());
    return;
  }

  // Move cursor forward
  if ((strcmp_P(sequence, PSTR("\eC")) == 0) ||
      (sscanf(sequence, "\e[%dC", number) >= 0))
  {
    display.setCursor(display.getRow(), display.getColumn() + number);
    return;
  }

  // Move cursor backward
  if ((strcmp_P(sequence, PSTR("\eD")) == 0) ||
      (sscanf(sequence, "\e[%dD", number) >= 0))
  {
    display.setCursor(display.getRow(), display.getColumn() - number);
    return;
  }

  // Clear indicators
  if (strcmp_P(sequence, PSTR("\e[0q")) == 0)
  {
    keyboard.clearIndicators();
    return;
  }

  // Turn on Num Lock
  if (strcmp_P(sequence, PSTR("\e[1q")) == 0)
  {
    keyboard.setIndicator(NUM_LOCK, true);
    return;
  }

  // Turn on Caps Lock
  if (strcmp_P(sequence, PSTR("\e[2q")) == 0)
  {
    keyboard.setIndicator(CAPS_LOCK, true);
    return;
  }

  // Turn on Scroll Lock
  if (strcmp_P(sequence, PSTR("\e[3q")) == 0)
  {
    keyboard.setIndicator(SCROLL_LOCK, true);
    return;
  }

  // Turn off Num Lock
  if (strcmp_P(sequence, PSTR("\e[21q")) == 0)
  {
    keyboard.setIndicator(NUM_LOCK, false);
    return;
  }

  // Turn off Caps Lock
  if (strcmp_P(sequence, PSTR("\e[22q")) == 0)
  {
    keyboard.setIndicator(CAPS_LOCK, false);
    return;
  }

  // Turn off Scroll Lock
  if (strcmp_P(sequence, PSTR("\e[23q")) == 0)
  {
    keyboard.setIndicator(SCROLL_LOCK, false);
    return;
  }
}

bool EscapeHandler::processData(uint32_t data)
{
  // Check for start of sequence
  if ((length == 0) && (data != '\e'))
    return false;

  sequence[length] = data;
  length ++;

  // Stop process if the sequence unknown
  if ((length == sizeof(sequence)) || (data > 127))
    length = 0;

  // Process ANSI sequence
  if ((length > 2) && (sequence[1] == '[') && (data >= '@') && (data <= '~'))
    processSequence();

  // Process VT-100 2-byte sequences
  // Note:
  // ECMA-48 says that 2-byte sequences should be terminated with character starting from '@',
  // but in fact VT-100 uses characters starting from '<'.
  if ((length == 2) && (data >= '<') && (data <= 'Z'))
    processSequence();

  // Process VT-100 3-byte sequences
  if ((length == 3) && (sequence[1] >= '(') && (sequence[1] <= ')'))
    processSequence();

  return true;
}
