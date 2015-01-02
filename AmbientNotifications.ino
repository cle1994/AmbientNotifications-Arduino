
//"services.h/spi.h/boards.h" is needed in every new project
#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <services.h>
#include <SimpleTimer.h>

#define LED_RED_PIN     6
#define LED_GREEN_PIN   5
#define LED_BLUE_PIN    3

#define SYNC_BYTE       0xa5

byte g_RedVal = 210;
byte g_GreenVal = 210;
byte g_BlueVal = 210;

SimpleTimer disconnectionTimer;
int timerId;

void checkDisconnect()
{
  if (!ble_connected())
  {
    g_RedVal = 255;
    g_GreenVal = 255;
    g_BlueVal = 255;
    
    setColors();
    
    disconnectionTimer.disable(timerId);
  }
}

void setColors()
{
  analogWrite(LED_RED_PIN, g_RedVal);
  analogWrite(LED_BLUE_PIN, g_BlueVal);
  analogWrite(LED_GREEN_PIN, g_GreenVal);
}

void setup()
{
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(LSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.begin();
  
  ble_set_name("Ambient");
  
  ble_begin();
  
  pinMode(LED_RED_PIN,OUTPUT);
  pinMode(LED_GREEN_PIN,OUTPUT);
  pinMode(LED_BLUE_PIN,OUTPUT);
  
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
        g_RedVal = packet[1];
        g_GreenVal = packet[2];
        g_BlueVal = packet[3];
        
        setColors();
        
        counter = 0;
      }
    }
  }
  
  ble_do_events();
}

