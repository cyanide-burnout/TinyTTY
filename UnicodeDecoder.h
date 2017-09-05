// Copyright 2012 Artem Prilutskiy

#ifndef UNICODEDECODER_H
#define UNICODEDECODER_H

#include <stddef.h>
#include <stdint.h>

class UnicodeDecoder
{
  public:

    UnicodeDecoder();

    bool processData(char data);
    uint32_t getCode();

  private:

    uint32_t code;
    size_t number;

};

#endif
