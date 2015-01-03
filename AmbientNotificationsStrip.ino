
//"services.h/spi.h/boards.h" is needed in every new project
#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <services.h>
#include <SimpleTimer.h>
#include <PololuLedStrip.h>

#define SYNC_BYTE  0xa5
#define LED_COUNT  60

PololuLedStrip<5> ledStrip;
rgb_color colors[LED_COUNT];

uint8_t r = 100;
uint8_t g = 100;
uint8_t b = 100;

SimpleTimer disconnectionTimer;
int timerId;

void checkDisconnect()
{
  if (!ble_connected())
  {
    r = 0;
    g = 0;
    b = 0;
    
    setColors();
    
    disconnectionTimer.disable(timerId);
  }
}

void setColors()
{
  for (uint16_t i = 0; i < LED_COUNT; i += 1)
  {
    colors[i] = (rgb_color){r, g, b};
  }
  
  ledStrip.write(colors, LED_COUNT);
}

void setup()
{
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(LSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.begin();
  
  ble_set_name("Ambient");
  
  ble_begin();
  
  setColors();
  
  timerId = disconnectionTimer.setInterval(3000, checkDisconnect);
}

byte packet[5];
byte counter = 0;

void loop()
{ 
  disconnectionTimer.run();
  
  while(ble_available())
  {
    for (byte i = 0; i < 4; i += 1)
    {
      packet[i] = packet[i + 1];
      disconnectionTimer.enable(timerId);
    }
    
    packet[4] = ble_read();
    counter += 1;
    
    // Packet Format
    // [Sync Byte, Red Value, Green Value, Blue Value, Checksum]
    if ((packet[0] == SYNC_BYTE) && (counter == 5))
    {
      byte checksum = packet[1] ^ packet[2] ^ packet[3];
      
      if (packet[4] == checksum)
      {
        r = packet[1];
        g = packet[2];
        b = packet[3];
        
        setColors();
        
        counter = 0;
      }
    }
  }
  
  ble_do_events();
}

