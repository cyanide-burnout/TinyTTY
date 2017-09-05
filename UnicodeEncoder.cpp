// Copyright 2012 Artem Prilutskiy

#include "UnicodeEncoder.h"

// http://en.wikipedia.org/wiki/UTF-8

UnicodeEncoder::UnicodeEncoder(uint32_t data) :
  data(data),
  count(0),
  index(0)
{
  if (data < 0x80)
  {
    count = 1;
    return;
  }

  while (data != 0)
  {
    data >>= 6;
    count ++;
  }
}

uint8_t UnicodeEncoder::read()
{
  index ++;

  if (count == 1)
    return data;

  uint16_t shift = (count - index) * 6;
  uint32_t buffer = (data >> shift) & 0x3f;

  if (index == 1)
    buffer |= 0xff00 >> count;
  else
    buffer |= 0x80;

  return buffer;
}

bool UnicodeEncoder::available()
{
  return index < count;
}