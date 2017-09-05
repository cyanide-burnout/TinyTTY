// Copyright 2012 Artem Prilutskiy

#ifndef UNICODEENCODER_H
#define UNICODEENCODER_H

#include <stdint.h>
#include <stddef.h>

class UnicodeEncoder
{
  public:
    UnicodeEncoder(uint32_t data);
    bool available();
    uint8_t read();

  private:
    uint32_t data;
    size_t count;
    size_t index;
};

#endif
