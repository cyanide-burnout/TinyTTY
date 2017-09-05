// Copyright 2012 Artem Prilutskiy

#ifndef KEYBOARDMAP_H
#define KEYBOARDMAP_H

#include <stdint.h>
#include <stddef.h>
#include "KeyCodes.h"

#define MODIFIER_SHIFT      (1 << 0)
#define MODIFIER_ALTGR      (1 << 1)
#define MODIFIER_CONTROL    (1 << 2)
#define MODIFIER_ALT        (1 << 3)
#define MODIFIER_SHIFTL     (1 << 4)
#define MODIFIER_SHIFTR     (1 << 5)
#define MODIFIER_CTRLL      (1 << 6)
#define MODIFIER_CTRLR      (1 << 7)
#define MODIFIER_CAPSSHIFT  (1 << 8)
#define MODIFIER_KEYMAP     (1 << 9)

#define MODIFIER_MASK       (MODIFIER_CONTROL | MODIFIER_CTRLL | MODIFIER_CTRLR | MODIFIER_ALT | MODIFIER_ALTGR)

#define FLAG_SYMBOL         0x01
#define FLAG_STRING         0x10

#define FLAG_SYMBOL0        (FLAG_SYMBOL << 0)
#define FLAG_SYMBOL1        (FLAG_SYMBOL << 1)
#define FLAG_SYMBOL2        (FLAG_SYMBOL << 2)
#define FLAG_SYMBOL3        (FLAG_SYMBOL << 3)

#define FLAG_STRING0        (FLAG_STRING << 0)
#define FLAG_STRING1        (FLAG_STRING << 1)
#define FLAG_STRING2        (FLAG_STRING << 2)
#define FLAG_STRING3        (FLAG_STRING << 3)

#define KEYCODE_RELEASE     0x80000000UL

#define ACTION_UPPER_CASE   1
#define ACTION_ALTER_MAP    2

#define SEQUENCE_SIZE       32
#define ACTION_COUNT        4

#define KEY_COUNT           keyCount

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)

union KeyAction
{
  uint16_t symbol;
  char* string;
};

struct KeyMap
{
  uint16_t code;
  uint8_t modifiers;
  uint8_t flags;
  union KeyAction actions[ACTION_COUNT];
};

#pragma pack(pop)

extern struct KeyMap keys[];
extern size_t keyCount;

#ifdef __cplusplus
};
#endif

#endif
