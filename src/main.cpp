#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

Adafruit_NeoPixel strip(100, 6, NEO_RGB + NEO_KHZ800);

/*
  Serial Event example

  When new serial data arrives, this sketch adds it to a String.
  When a newline is received, the loop prints the string and clears it.

  A good test for this is to try it with a GPS receiver that sends out
  NMEA 0183 sentences.

  NOTE: The serialEvent() feature is not available on the Leonardo, Micro, or
  other ATmega32U4 based boards.

  created 9 May 2011
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/SerialEvent
*/

/**
 * BYTE 0 - 1 = Header = Ox259
 * Bytes 2 - 3 = Offset
 * Bytes 4 - Footer = RGB (Each color = 1 Byte Every 3 Bytes = new LED) Possible 6 bytes
 * 2 Bytes Footer = 0x132
 * Last Byte = CHECKSUM
**/
typedef enum
{
  UNAVAILABE,
  WAITING,
  STARTING,
  OFFSET1,
  OFFSET2,
  R,
  G,
  B,
  CLOSING,
  CLOSED
} states;

byte lastByte = 0;
states state = UNAVAILABE;
uint32_t rgb = 0;
uint16_t offset = 0;
uint32_t checksum = 0;

void setup()
{
  // initialize serial:
  Serial.begin(9600);
  // flush the serial
  while (Serial.available())
  {
    Serial.read();
  }
  strip.setBrightness(50);
  strip.begin();
}

boolean checksumValid(byte sumByte)
{
  Serial.print("given sum : ");
  Serial.println(sumByte, HEX);
  Serial.print("Checked Sum : ");
  Serial.println(checksum & 0xFF, HEX);  
  return (byte)(checksum & 0xFF) == sumByte;
}

void processColor()
{

  strip.setPixelColor(offset, rgb);
}

void handleState(byte lastByte)
{
  if(state != CLOSED) {
    checksum += lastByte;
  }
  switch (state)
  {
  case OFFSET1:
    offset = lastByte;
    break;
  case OFFSET2:
    offset = (offset << 8) | lastByte;
    break;
  case R:
    if (rgb > 0)
    {
      Serial.println(offset,DEC);
      processColor();
      offset += 1;
      rgb = 0;
    }
    rgb = (uint32_t)lastByte << 16;
    break;
  case G:
    rgb = rgb | ((uint32_t)lastByte << 8);
    break;
  case B:
    rgb = rgb | (uint32_t)lastByte;
    break;
  case CLOSED:
    if (checksumValid(lastByte))
    {
      strip.show();
      rgb = 0;
    }
    break;
  }
} // end of handlePreviousState

void processIncomingByte(const byte b)
{
  handleState(b);

  // set the new state, if we recognize it
  switch (state)
  {
  case WAITING:
    if (b == 0x59)
    {
      state = STARTING;
    }
    else
    {
      lastByte = 0;
      checksum = 0;
    }
    break;
  case STARTING:
    if (b == (0x02))
    {
      state = OFFSET1;
    }
    else
    {
      lastByte = 0;
      checksum = 0;
      state = WAITING;
    }
    break;
  case OFFSET1:
    state = OFFSET2;
    break;
  case OFFSET2:
    state = R;
    break;
  case R:
    if (b == 0x32)
    {
      state = CLOSING;
    }
    else
    {
      state = G;
    }
    break;
  case G:
    state = B;
    break;
  case B:
    state = R;
    break;
  case CLOSING:
    if (b == 0x01)
    {
      state = CLOSED;
    }
    else
    {
      state = G;
    }
    break;
  case CLOSED:
    state = WAITING;
    break;
  default:
    lastByte = 0;
    state = UNAVAILABE;
    break;
  }
}

void loop()
{
  // print the string when a newline arrives:
  while (Serial.available())
  {
    if (state == UNAVAILABE)
    {
      state = WAITING;
      Serial.println("ready");
    }
    processIncomingByte(Serial.read());
  }
}