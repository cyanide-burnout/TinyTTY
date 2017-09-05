// Copyright 2012 Artem Prilutskiy

#include <LiquidCrystal.h>
#include <HardwareSerial.h>
#include <stdint.h>
#include <string.h>

#include "UnicodeDecoder.h"
#include "UnicodeEncoder.h"
#include "LiquidDisplay.h"
#include "EscapeHandler.h"
#include "PS2Keyboard.h"
#include "PS2Wire.h"

#define TRAINER_BOARD
// #define ARDUINO_BOARD

// Please select "LilyBad Arduino w/ Atmega328" to compile sketch for Trainer Board

// http://hackaday.com/2008/05/29/how-to-super-simple-serial-terminal/

#define TIMEOUT  1000
#define LED_PIN  13

PS2Wire wire(2 /* DATA */, 3 /* CLK */);
PS2Keyboard keyboard(wire);

#ifdef TRAINER_BOARD
LiquidCrystal display(10 /* RS */, 9 /* RW */, 8 /* EN */, 7 /* D4 */, 6 /* D5 */, 5 /* D6 */, 4 /* D7 */);
LiquidDisplay console(display, 20, 4);
#endif

#ifdef ARDUINO_BOARD
LiquidCrystal display(8 /* RS */, 9 /* EN */, 4 /* D4 */, 5 /* D5 */, 6 /* D6 */, 7 /* D7 */);
LiquidDisplay console(display, 16, 2);
#endif

EscapeHandler handler(console, keyboard, Serial);
UnicodeDecoder decoder;

uint32_t alarm;
uint8_t blink;

void keyboardInterrupt()
{
  wire.interrupt();
}

void setup()
{
  Serial.begin(19200);

  wire.begin(keyboardInterrupt);
  keyboard.reset();

  pinMode(LED_PIN, OUTPUT);
  alarm = millis() + TIMEOUT;
  blink = LOW;

  console.cursor();
}

void loop()
{
  if (Serial.available() && decoder.processData(Serial.read()))
  {
    uint32_t code = decoder.getCode();
    if (!handler.processData(code))
    {
      switch (code)
      {
        case 0x05:  // ENQ
          break;

        case '\r':  // CR
          console.setCursor(console.getRow(), 0);
          break;

        case '\n':  // LF
          console.newLine();
          break;

        case '\b':  // BS
        case 0x7f:  // DEL
          console.backSpace();
          break;

        default:
          console.print(code);
          break;
      }
    }
  }

  if (wire.available() && keyboard.processData(wire.read()))
  {
    uint16_t symbol = keyboard.getSymbol();
    if (symbol != SYMBOL_STRING)
    {
      // Send Unicode character
      UnicodeEncoder encoder(symbol);
      while (encoder.available())
        Serial.print((char)encoder.read());
    }
    else
    {
      // Send escape sequence
      Serial.print(keyboard.getString());
    }
  }

  if (alarm < millis())
  {
    blink ^= HIGH;
    digitalWrite(LED_PIN, blink);
    alarm = millis() + TIMEOUT;
  }
}
