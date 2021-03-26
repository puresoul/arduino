// Import required libraries
#include "EEPROM.h"
#include "Time.h"
#include "TimeLib.h"
#include "TimeAlarms.h"
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>



uint32_t delayMS;
typedef struct {
     int h;
     int m;
     int s;
     int min;
     int max;
     int type;
     int action;
     int pin;
} Action;

//================ Skce urcena pro upravy nastaveni
#define ALLTIME -1

// action
#define UP 1
#define DOWN 0

// type
#define TIME 0
#define TEMP 1

// Zde je potreba pojemnovat konkretni piny a zapsat je do pole pins

#define LED1 12
#define LED2 13

int pins[] = {LED1, LED2};

Action actions[] = {
// Definice akce zapnout, kazdou 31 sekundu
  {
  .h = ALLTIME,
  .m = ALLTIME,
  .s = 31,
  .min = 0,
  .max = 0,
  .type = TIME,
  .action = UP,
  .pin = LED1,
  },
  {
  .h = ALLTIME,
  .m = ALLTIME,
  .s = 15,
  .min = 0,
  .max = 0,
  .type = TIME,
  .action = UP,
  .pin = LED2,
  },
// Definice akce vypnout, kazdou 1 sekundu
  {
  .h = ALLTIME,
  .m = ALLTIME,
  .s = 1,
  .min = 0,
  .max = 0,
  .type = TIME,
  .action = DOWN,
  .pin = LED1,
  },
  {
  .h = ALLTIME,
  .m = ALLTIME,
  .s = 45,
  .min = 0,
  .max = 0,
  .type = TIME,
  .action = DOWN,
  .pin = LED2,
  },
};

//=========================================================================s

int temp;
int hum;

#define DHTPIN 2     // Digital pin connected to the DHT sensor 

#define DHTTYPE    DHT22     // DHT 22 (AM2302)

DHT_Unified dht(DHTPIN, DHTTYPE);

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned int localPort = 8888;       // local port to listen for UDP packets
const int timeZone = 1;
//const char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server
//IPAddress timeServer(91, 206, 8, 36);

IPAddress timeServer(129,6,15,28);

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
EthernetUDP UDP;

void setup(){  
  Serial.begin(115200);
  StartNET();
  for(int i=0; i<=sizeof(pins); i++) {
    pinMode(pins[i], OUTPUT);
  }
  sensor_t sensor;

  setSyncProvider(getNtpTime);
  dht.begin();
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
  Alarm.timerRepeat(1, Scheduler);
}

void StartNET() {
  Ethernet.init(10);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
  }
  UDP.begin(localPort);
}

void Scheduler() {

  int h = hour();            // the hour now  (0-23)
  int m = minute();          // the minute now (0-59)
  int s = second();
  for(int i=0; i<=sizeof(actions); i++) {
    if (actions[i].s == s || actions[i].s == -1 || actions[i].type != TIME) {
      if (actions[i].s == m || actions[i].m == -1 || actions[i].type != TIME) {
        if (actions[i].s == h || actions[i].h == -1 || actions[i].type != TIME) {
          Worker(i);
        }
      }
    }
  }
  int address = 0;
}

bool Checker(int i, int min, int max) {
  if (actions[i].min > min) return false;
  if (actions[i].max < max) return false;  
  return true;
}

void Worker(int i) {
  if (actions[i].type == TIME) {
    printTime();
    Serial.print("ACTION TIME = ");
    Serial.print(actions[i].pin);
    Serial.print(" - ");
    Serial.println(actions[i].action);
    digitalWrite(actions[i].pin, actions[i].action);
    return;
  } else {
    Serial.print("Temp = ");
    Serial.print(temp);
    Serial.print(" Hum = ");
    Serial.println(hum);
    return;
    if (actions[i].h >= 0) {
      return;
    }
    if (actions[i].m <= 0) {
      return;
    }
  }
}

void printTime() {
  int h = hour();            // the hour now  (0-23)
  int m = minute();          // the minute now (0-59)
  int s = second();
  Serial.print(h);
  Serial.print(":");
  Serial.print(m);
  Serial.print(":");
  Serial.println(s);
}

void loop() {
  Alarm.delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (event.temperature) {
    temp = event.temperature;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (event.relative_humidity) {
    hum = event.relative_humidity;
  }
  Alarm.delay(1000);
}
//=========================================================

time_t getNtpTime()
{
  while (UDP.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = UDP.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      UDP.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  UDP.beginPacket(address, 123); //NTP requests are to port 123
  UDP.write(packetBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}
