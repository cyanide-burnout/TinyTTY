// Copyright 2012 Artem Prilutskiy

#include "PS2Wire.h"
#include <Arduino.h>

#include <HardwareSerial.h>

#define CYCLE_DELAY             5
#define CYCLES_PER_MILLISECOND  (1000 / CYCLE_DELAY)
#define DEVICE_TIMEOUT          (200 * CYCLES_PER_MILLISECOND)

PS2Wire::PS2Wire(uint8_t dataPin, uint8_t clockPin) :
  dataPin(dataPin),
  clockPin(clockPin),
  skip(0),
  start(0),
  end(0)
{

}

void PS2Wire::begin(InterruptHandler handler)
{
  releaseLine(clockPin);
  releaseLine(dataPin);

  int number = 0;
  const uint8_t triggers[] = { 2, 3, 21, 20, 19, 18 };
  for (size_t index = 0; index < sizeof(triggers); index ++)
    if (clockPin == triggers[index])
      number = index;

  attachInterrupt(number, handler, FALLING);
}

void PS2Wire::receiveBit()
{
  // Manipulation of the clock line in write function leads to the interrupts request
  // We should skip that calls to avoid readings of outgoing frame tails
  if (skip > 0)
  {
    skip --;
    return;
  }

  uint32_t now = millis();
  if ((now - stamp) > 250)
  {
    store = 0;
    mask = 1;
  }
  stamp = now;

  if (digitalRead(dataPin) == HIGH)
    store |= mask;

  mask <<= 1;

  // Mask will have value of 0x800 after receiving of 11 bits
  // Also we check start/stop bits in the received frame
  if ((mask == 0x800) && ((store & 0x401) == 0x400))
  {
    buffer[end] = (store >> 1) & 0xff;
    end ++;
    if (end >= PS2_BUFFER_SIZE)
      end = 0;
    if (end == start)
      start ++;
    if (start >= PS2_BUFFER_SIZE)
      start = 0;

    store = 0;
    mask = 1;
  }
}

void PS2Wire::interrupt()
{
  noInterrupts();
  receiveBit();
  interrupts();
}

bool PS2Wire::available()
{
  bool value = false;
  noInterrupts();
  value = (start != end);
  interrupts();
  return value;
}

int PS2Wire::read()
{
  int value = EOF;
  noInterrupts();
  if (start != end)
  {
    value = buffer[start];

    start ++;
    if (start >= PS2_BUFFER_SIZE)
      start = 0;
  }
  interrupts();
  return value;
}

void PS2Wire::releaseLine(uint8_t pin)
{
#ifdef INPUT_PULLUP
  pinMode(pin, INPUT_PULLUP);
#else
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
#endif
}

void PS2Wire::bringLow(uint8_t pin)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void PS2Wire::writeBit(uint8_t data)
{
  if (data)
    releaseLine(dataPin);
  else
    bringLow(dataPin);
}

bool PS2Wire::waitFor(uint8_t option)
{
  // Note: with disabled interrupts millis() would not work
  for (uint32_t index = 0; index < DEVICE_TIMEOUT; index ++)
  {
    switch (option)
    {
      case ClockHigh:
        if (digitalRead(clockPin) == HIGH)
          return true;
        break;

      case ClockLow:
        if (digitalRead(clockPin) == LOW)
          return true;
        break;

      case DataHigh:
        if (digitalRead(dataPin) == HIGH)
          return true;
        break;

      case DataLow:
        if (digitalRead(dataPin) == LOW)
          return true;
        break;

      case ClockHigh | DataHigh:
        if ((digitalRead(clockPin) == HIGH) &&
            (digitalRead(dataPin) == HIGH))
          return true;
        break;
    }
    delayMicroseconds(CYCLE_DELAY);
  }
  return false;
}

void PS2Wire::write(uint8_t data)
{
  noInterrupts();

  bringLow(clockPin);
  delayMicroseconds(150);
  bringLow(dataPin);
  delayMicroseconds(10);
  releaseLine(clockPin);
  waitFor(ClockLow);

  uint8_t parity = 1;
  for (size_t index = 0; index < 8; index ++)
  {
    uint8_t bit = data & 1;
    writeBit(bit);
    parity ^= bit;
    data >>= 1;
    waitFor(ClockHigh);
    waitFor(ClockLow);
  }

  writeBit(parity);
  waitFor(ClockHigh);
  waitFor(ClockLow);

  releaseLine(dataPin);
  delayMicroseconds(50);
  waitFor(ClockLow);
  waitFor(ClockHigh | DataHigh);

  // Manipulation of the clock line in write function leads to scheduled interrupts request
  // Next line tells interrupt handler skip 1 bit  
  skip = 1;
 
  interrupts();
}
