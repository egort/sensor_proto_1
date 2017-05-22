/*
          ##
         ###
        # ##
          ##      RECEIVER NODE
          ##
         ####
*/

#include "dht11.h"
#include <SPI.h>
#include <plainRFM69.h>

#define DHTPIN 3 // pin DHT11
#define SLAVE_SELECT_PIN 10 // SS/NSS line on SPI bus
#define SENDER_DETECT_PIN A0 // pull down for receiver mode, or up if it is sender
#define RESET_PIN 8 // reset pin RFM69
#define DIO2_PIN 2 // data pin DIO2 RFM69 should be attached to digital pin with interrupt capability (2 or 3 for Pro Mini)
#define INTERRUPT_NUMBER 0 // on Pro Mini INT0 at pin 2, INT1 at 3

/*
    This is a minimal example with the interrupt to call poll().
    It transmits a 4 byte integer every 500 ms.
*/

dht11 DHT11;
plainRFM69 rfm = plainRFM69(SLAVE_SELECT_PIN);

// calculate "heat index" based on humidity and temperature
double heatIndex(double tempC, double humidity)
{
  double c1 = -42.38, c2 = 2.049, c3 = 10.14, c4 = -0.2248, c5= -6.838e-3, c6=-5.482e-2, c7=1.228e-3, c8=8.528e-4, c9=-1.99e-6  ;
  double Tf = ((tempC*9)/5)+32;
  double R = humidity;
  double A = (( c5 * Tf) + c2) * Tf + c1;
  double B = ((c7 * Tf) + c4) * Tf + c3;
  double C = ((c9 * Tf) + c8) * Tf + c6;
  double rv = (C * R + B) * R + A;
  return rv;
}

void interrupt_RFM(){
    rfm.poll(); // in the interrupt, call the poll function
}

void setup() {
  SPI.begin();
  Serial.begin(19200);
  Serial.println("--DHT11 @ RFM69 here!");
  // <!-- RFM69 setup
  bareRFM69::reset(RESET_PIN); // sent the RFM69 a hard-reset.
  rfm.setRecommended(); // set recommended paramters in RFM69.
  rfm.setPacketType(false, false); // set the used packet type.
  rfm.setBufferSize(2);   // set the internal buffer size.
  rfm.setPacketLength(4); // set the packet length.
  rfm.setFrequency((uint32_t) 915*1000*1000); // set the frequency to 915MHz
  // baudrate is default, 4800 bps now.
  rfm.receive(); // set it to receiving mode.
  /*
      setup up interrupts such that we don't have to call poll() in a loop.
  */
  rfm.setDioMapping1(RFM69_PACKET_DIO_2_AUTOMODE); // tell the RFM to represent whether we are in automode on DIO 2.
  pinMode(DIO2_PIN, INPUT); // set pinmode to input
  SPI.usingInterrupt(INTERRUPT_NUMBER); // Tell the SPI library we're going to use the SPI bus from an interrupt.
  attachInterrupt(INTERRUPT_NUMBER, interrupt_RFM, CHANGE); // hook our interrupt function to any edge.
  rfm.receive();       // start receiving
  pinMode(SENDER_DETECT_PIN, INPUT_PULLUP); // sernder|receiver switch pin
  // RFM69 setup -->
  delay(5);
}

void loop() {

  int dht = DHT11.read(DHTPIN);
  float humi = DHT11.humidity;
  float temp = DHT11.temperature;

  switch (dht)
  {
    case DHTLIB_OK:
    {
		  Serial.println("OK");
      Serial.print("Humidity (%): ");
      Serial.println(humi, 2);
      Serial.print("Temperature (°C): ");
      Serial.println(temp, 2);
      Serial.print("Heat index: ");
      Serial.print(heatIndex(temp, humi));
      Serial.println(" *C");
    }
		break;
    case DHTLIB_ERROR_CHECKSUM:
		  Serial.println("Checksum error");
		break;
    case DHTLIB_ERROR_TIMEOUT:
		  Serial.println("Time out error");
		break;
    default:
		  Serial.println("Unknown error");
		break;
  }
  delay(2000);   // DHT11 is slow sensor, wait at least 2 sec
}
