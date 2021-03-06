/*
 * Simple interface to the Fly Sky IBus RC system.
 */

#include <Arduino.h>
#include "FlySkyIBus.h"

FlySkyIBus IBus;

void FlySkyIBus::begin(HardwareSerial& serial, unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert)
{
  serial.begin(baud, config, rxPin, txPin, invert);
  begin((Stream&)serial);
}

void FlySkyIBus::begin(Stream& stream)
{
  this->stream = &stream;
  this->state = DISCARD;
  this->last = millis();
  this->ptr = 0;
  this->len = 0;
  this->chksum = 0;
  this->lchksum = 0;
  this->lastUpdate = millis();
}

void FlySkyIBus::loop(void)
{
  uint8_t i;
  uint32_t now;
  uint8_t v;

  while (stream->available() > 0)
  {
    now = millis();
    if (now - last >= PROTOCOL_TIMEGAP)
    {
      state = GET_LENGTH;
    }
    last = now;
    
    v = stream->read();
    switch (state)
    {
      case GET_LENGTH:
        if (v <= PROTOCOL_LENGTH)
        {
          ptr = 0;
          len = v - PROTOCOL_OVERHEAD;
          chksum = 0xFFFF - v;
          state = GET_DATA;
        }
        else
        {
          state = DISCARD;
        }
        break;

      case GET_DATA:
        buffer[ptr++] = v;
        chksum -= v;
        if (ptr == len)
        {
          state = GET_CHKSUML;
        }
        break;
        
      case GET_CHKSUML:
        lchksum = v;
        state = GET_CHKSUMH;
        break;

      case GET_CHKSUMH:
        // Validate checksum
        if (chksum == (v << 8) + lchksum)
        {
          // Execute command - we only know command 0x40
          switch (buffer[0])
          {
            case PROTOCOL_COMMAND40:
              // Valid - extract channel data
              for (i = 1; i < PROTOCOL_CHANNELS * 2 + 1; i += 2)
              {
                channel[i / 2] = buffer[i] | (buffer[i + 1] << 8);
              }
              lastUpdate = millis();
              break;

            default:
              break;
          }
        }
        state = DISCARD;
        break;

      case DISCARD:
      default:
        break;
    }
  }
}

void FlySkyIBus::getChannels(uint16_t *channels, size_t length)
{
  if(length / sizeof(uint16_t) > PROTOCOL_CHANNELS)
  {
    return;
  }
  memcpy(channels, channel, length);
}

long FlySkyIBus::millisSinceUpdate()
{
	return millis() - lastUpdate;
}