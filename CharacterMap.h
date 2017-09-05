// Copyright 2012 Artem Prilutskiy

#ifndef CHARACTERMAP_H
#define CHARACTERMAP_H

#include <stdint.h>
#include <stddef.h>

#define CHARACTER_COUNT  characterCount

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)

struct CharacterMap
{
  uint16_t code;
  char character;
};

#pragma pack(pop)

extern struct CharacterMap characters[];
extern size_t characterCount;

#ifdef __cplusplus
};
#endif

#endif