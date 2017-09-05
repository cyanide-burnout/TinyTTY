// Copyright 2012 Artem Prilutskiy

#include "PS2Keyboard.h"
#include <avr/pgmspace.h>
#include <string.h>

// http://marsohod.org/index.php/ourblog/11-blog/57-ps2proto
// http://www.computer-engineering.org/ps2keyboard/

#define PS2_ACKNOWLEDGE            0xfa
#define PS2_RESEND                 0xfe
#define PS2_FAILURE                0xfc

#define KEYBOARD_RESET             0xff
#define SET_KEYBOARD_INDICATORS    0xed
#define SET_KEYBOARD_TYPEMATIC     0xf3
#define READ_KEYBOARD_ID           0xf2
#define SELECT_SCAN_CODE_SET       0xf0
#define KEYBOARD_COMPLETE_SUCCESS  0xaa

#define KANA_LOCK_ON               8
#define CAPS_LOCK_ON               4
#define NUM_LOCK_ON                2
#define SCROLL_LOCK_ON             1

PS2Keyboard::PS2Keyboard(PS2Wire& wire) :
  wire(wire),
  indicators(0),
  modifiers(0),
  code(0)
{
  memset(&key, 0, sizeof(key));
  memset(buffer, 0, sizeof(buffer));
}

void PS2Keyboard::reset()
{
  modifiers = 0;
  indicators = 0;
  wire.write(KEYBOARD_RESET);
}

void PS2Keyboard::updateIndicators()
{
  wire.write(SET_KEYBOARD_INDICATORS);
  wire.write(indicators);
}

bool PS2Keyboard::processData(uint8_t data)
{
  // Skip release prefix byte and set release flag
  if (data == 0xf0)
  {
    code |= KEYCODE_RELEASE;
    return false;
  }

  code = (code & KEYCODE_RELEASE) | (code << 8) | data;

  // Keep read if uncomplited data was received
  if ((data == 0xe0) ||             // 2-byte key code prefix
      (data == 0xe1) ||             // 3-byte key code prefix
      ((code & 0xff00) == 0xe100))  // second byte after 3-byte key code prefix
    return false;

  // Process completed key code and clear input buffer
  bool result = processKey();
  code = 0UL;

  return result;
}

bool PS2Keyboard::searchKey()
{
  for (size_t index = 0; index < KEY_COUNT; index ++)
    if (pgm_read_word(&keys[index].code) == code)
    {
      memcpy_P(&key, &keys[index], sizeof(key));

      if ((key.modifiers == 0) && ((modifiers & MODIFIER_MASK) == 0))
        return true;

      if ((key.modifiers != 0) && ((modifiers & MODIFIER_MASK) != 0))
      {
        uint16_t mask = 0xff;
        if (key.modifiers & MODIFIER_SHIFT)
          mask &= ~(MODIFIER_SHIFTL | MODIFIER_SHIFTR);
        if (key.modifiers & MODIFIER_CONTROL)
          mask &= ~(MODIFIER_CTRLL | MODIFIER_CTRLR);
        if (key.modifiers & (MODIFIER_SHIFTL | MODIFIER_SHIFTR))
          mask &= ~MODIFIER_SHIFT;
        if (key.modifiers & (MODIFIER_CTRLL | MODIFIER_CTRLR))
          mask &= ~MODIFIER_CONTROL;
        if ((modifiers & mask) == key.modifiers)
          return true;
      }
    }
  return false;
}

bool PS2Keyboard::processKey()
{
  switch (code)
  {
    case KEYCODE_L_SHFT:
      modifiers |= MODIFIER_SHIFTL;
      modifiers |= MODIFIER_SHIFT;
      return false;

    case KEYCODE_R_SHFT:
      modifiers |= MODIFIER_SHIFTR;
      modifiers |= MODIFIER_SHIFT;
      return false;

    case KEYCODE_L_SHFT | KEYCODE_RELEASE:
      modifiers &= ~MODIFIER_SHIFTL;
      if ((modifiers & MODIFIER_SHIFTR) == 0)
        modifiers &= ~MODIFIER_SHIFT;
      return false;

    case KEYCODE_R_SHFT | KEYCODE_RELEASE:
      modifiers &= ~MODIFIER_SHIFTR;
      if ((modifiers & MODIFIER_SHIFTL) == 0)
        modifiers &= ~MODIFIER_SHIFT;
      return false;

    case KEYCODE_L_CTRL:
      modifiers |= MODIFIER_CTRLL;
      modifiers |= MODIFIER_CONTROL;
      return false;

    case KEYCODE_R_CTRL:
      modifiers |= MODIFIER_CTRLR;
      modifiers |= MODIFIER_CONTROL;
      return false;

    case KEYCODE_L_CTRL | KEYCODE_RELEASE:
      modifiers &= ~MODIFIER_CTRLL;
      if ((modifiers & MODIFIER_CTRLR) == 0)
        modifiers &= ~MODIFIER_CONTROL;
      return false;

    case KEYCODE_R_CTRL | KEYCODE_RELEASE:
      modifiers &= ~MODIFIER_CTRLR;
      if ((modifiers & MODIFIER_CTRLL) == 0)
        modifiers &= ~MODIFIER_CONTROL;
      return false;

    case KEYCODE_L_ALT:
      modifiers |= MODIFIER_ALTGR;
      return false;

    case KEYCODE_R_ALT:
      modifiers |= MODIFIER_ALTGR;
      return false;

    case KEYCODE_L_ALT | KEYCODE_RELEASE:
      modifiers &= ~MODIFIER_ALTGR;
      return false;

    case KEYCODE_R_ALT | KEYCODE_RELEASE:
      modifiers &= ~MODIFIER_ALTGR;
      return false;

    case KEYCODE_CAPS:
      modifiers ^= MODIFIER_CAPSSHIFT;
      indicators ^= CAPS_LOCK_ON;
      updateIndicators();
      return false;

    // case KEYCODE_NUM: // Should we do something ???

    case KEYCODE_SPACE:
      if (modifiers & MODIFIER_CONTROL)
      {
        modifiers ^= MODIFIER_KEYMAP;
        return false;
      }

    default:
      return searchKey();
  }
}

size_t PS2Keyboard::getActionIndex()
{
  size_t index = 0;

  if ((key.flags & (FLAG_SYMBOL2 | FLAG_STRING2)) &&
      (modifiers & MODIFIER_KEYMAP))
    index |= ACTION_ALTER_MAP;

  if (key.flags & ((FLAG_SYMBOL | FLAG_STRING) << index))
  {
    if (modifiers & MODIFIER_CAPSSHIFT)
      index ^= ACTION_UPPER_CASE;
    if (modifiers & MODIFIER_SHIFT)
      index ^= ACTION_UPPER_CASE;
  }

  return index;
}

uint16_t PS2Keyboard::getSymbol()
{
  size_t index = getActionIndex();

  if (key.flags & (FLAG_STRING << index))
    return SYMBOL_STRING;

  return key.actions[index].symbol;
}

const char* PS2Keyboard::getString()
{
  size_t index = getActionIndex();

  if (key.flags & (FLAG_SYMBOL << index))
    return NULL;

  return strcpy_P(buffer, (const prog_char*)key.actions[index].string);
}

void PS2Keyboard::clearIndicators()
{
  indicators = 0;
  modifiers &= ~MODIFIER_CAPSSHIFT;
  updateIndicators();
}

void PS2Keyboard::setIndicator(uint8_t indicator, bool on)
{
  if (on)
    indicators |= indicator;

  if (!on)
    indicators &= ~indicator;

  if ((on) && (indicator == CAPS_LOCK_ON))
    modifiers |= MODIFIER_CAPSSHIFT;

  if ((!on) && (indicator == CAPS_LOCK_ON))
    modifiers &= ~MODIFIER_CAPSSHIFT;

  updateIndicators();
}
