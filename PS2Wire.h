// Copyright 2012 Artem Prilutskiy

#ifndef PS2WIRE_H
#define PS2WIRE_H

#include <stddef.h>
#include <stdint.h>

#define PS2_BUFFER_SIZE  48

class PS2Wire
{
  public:

    typedef void (*InterruptHandler)();

    PS2Wire(uint8_t dataPin, uint8_t clockPin);
    void begin(InterruptHandler handler);

    void interrupt();

    bool available();
    int read();

    void write(uint8_t data);

  private:

    enum
    {
      ClockHigh = 1,
      ClockLow = 2,
      DataHigh = 4,
      DataLow = 8
    };

    uint8_t dataPin;
    uint8_t clockPin;

    volatile uint8_t buffer[PS2_BUFFER_SIZE];
    volatile size_t start;
    volatile size_t end;

    volatile uint16_t skip;

    uint32_t stamp;
    uint16_t store;
    uint16_t mask;

    void receiveBit();

    void bringLow(uint8_t pin);
    void releaseLine(uint8_t pin);
    void writeBit(uint8_t data);
    bool waitFor(uint8_t option);
};

#endif
