/*
Tester: Chris Ruettimann<chris@bitbull.ch>
Date: 2016-01-02
Board: Arduino Pro Mini 3.3V 8MHz 
MySensors DHT22 Sensor Node with NRF24L01 Transmitter
=====================================================
DHT22 CONNECTION
----------------
PIN1: 3.3V
PIN2: Arduino D2, 3.3v via 10kOhm
PIN3: empty
PIN4: GND

NRF24L01 CONNECTION:
--------------------
GND1 : GND
VCC2 : 3.3V
CE3  : Arduino D08
CSN4 : Arduino D10
SCK5 : Arduino D13
MOSI6: Arduino D11
MISO7: Arduino D12
IRQ8 : empty
*/

#include <SPI.h>
#include <MySensor.h>
#include <DHT.h>

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
int BATTERY_SENSE_PIN = A0;
#define HUMIDITY_SENSOR_DIGITAL_PIN 2
unsigned long SLEEP_TIME = 300000; // Sleep time between reads (in milliseconds)


   float batteryMax = 4400 ; // Battery max level in mV
   float batteryMin = 3000 ; // Battery min level in mV (LiPo)

MySensor gw;
DHT dht;
float lastTemp;
float lastHum;
boolean metric = true;
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
int oldBatteryPcnt = -1;


void setup() {
  Serial.begin(9600);
  gw.begin();
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN);

  // Send the Sketch Version Information to the Gateway
  gw.sendSketchInfo("Humidity", "1.0");
  gw.sendSketchInfo("Battery Meter", "1.0");
  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  metric = gw.getConfig().isMetric;
  Serial.println("Setup done...");
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());
  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed reading temperature from DHT");
  } else if (temperature != lastTemp) {
    lastTemp = temperature;
    if (!metric) {
      temperature = dht.toFahrenheit(temperature);
    }
    gw.send(msgTemp.set(temperature, 1));
    Serial.print("T: ");
    Serial.println(temperature);
  }

  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum) {
    lastHum = humidity;
    gw.send(msgHum.set(humidity, 1));
    Serial.print("H: ");
    Serial.println(humidity);
  }
   float batteryV = readVcc();
   Serial.print("V: ");
   Serial.println(batteryV / 1000);
   int batteryPcnt = (batteryV - batteryMin) / (batteryMax - batteryMin) * 100  ;
   Serial.print("Battery Level: ");
   Serial.print(batteryPcnt);
   Serial.println("%");
  if (oldBatteryPcnt != batteryPcnt) {
    gw.sendBatteryLevel(batteryPcnt);
    oldBatteryPcnt = batteryPcnt;
  }
 gw.sleep(SLEEP_TIME); //sleep a bit
}

long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}


