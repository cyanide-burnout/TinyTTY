// Copyright 2012 Artem Prilutskiy

#include "UnicodeDecoder.h"

UnicodeDecoder::UnicodeDecoder() :
  number(0),
  code(0)
{

}

bool UnicodeDecoder::processData(char data)
{
  if ((data & 0x80) == 0x00)
  {
    code = data;
    return true;
  }

  if ((data & 0xc0) == 0x80)
  {
    code = (code << 6) | (data & 0x3f);
    number --;
    return (number == 0);
  }

  for (size_t index = 0; index < 5; index ++)
  {
    uint16_t mask = (0x0fe0 >> index) & 0xff;
    uint16_t mark = (0x0fc0 >> index) & 0xff;
    if ((data & mask) == mark)
    {
      mask = (0x001f >> index);
      code = (data & mask);
      number = index + 1;
      return false;
    }
  }

  return false;
}

uint32_t UnicodeDecoder::getCode()
{
  return code;
}
