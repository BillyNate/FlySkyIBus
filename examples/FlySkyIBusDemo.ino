/*
 * Test FlySky IBus interface on an ESP32
 *  Connect FS-iA6B receiver i-bus wire to GPIO16
 */
 
#include "FlySkyIBus.h"

void setup() 
{
  Serial.begin(115200);
  IBus.begin(Serial2, 115200, SERIAL_8N1, GPIO_NUM_16);
}

void loop() 
{
  IBus.loop();
  Serial.println(IBus.readChannel(0), HEX);
}
