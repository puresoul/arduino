// Import required libraries
#include "Time.h"
#include "TimeLib.h"
#include "TimeAlarms.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "WiFiUdp.h"

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
  .h = -1,
  .m = -1,
  .s = 31,
  .min = 0,
  .max = 0,
  .type = TIME,
  .action = UP,
  .pin = LED1,
  },
  {
  .h = -1,
  .m = -1,
  .s = 15,
  .min = 0,
  .max = 0,
  .type = TIME,
  .action = UP,
  .pin = LED2,
  },
// Definice akce vypnout, kazdou 1 sekundu
  {
  .h = -1,
  .m = -1,
  .s = 1,
  .min = 0,
  .max = 0,
  .type = TIME,
  .action = DOWN,
  .pin = LED1,
  },
  {
  .h = -1,
  .m = -1,
  .s = 45,
  .min = 0,
  .max = 0,
  .type = TIME,
  .action = DOWN,
  .pin = LED2,
  },
};

//=========================================================================s

const char* ssid = "MujO2Internet_095882";
const char* password = "U2u27yf7";

unsigned int localPort = 8888;  // local port to listen for UDP packets
const int timeZone = 1;

IPAddress timeServer(91, 206, 8, 36);
WiFiUDP Udp;

void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  
  for(int i=0; i<=sizeof(pins); i++) {
    pinMode(pins[i], OUTPUT);
  }
  
  UpdateNTP();
  Alarm.timerRepeat(1, Scheduler);
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
}

void Worker(int i) {
  if (actions[i].type == TIME) {
    printTime();
    Serial.print("ACTION TIME = ");
    Serial.print(actions[i].pin);
    Serial.print(" - ");
    Serial.println(actions[i].action);
    digitalWrite(actions[i].pin, actions[i].action);
  }
  if (actions[i].type == TEMP) {
    //Serial.println("ACTION TEMP");
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
  Alarm.delay(0);
}
//=========================================================

void UpdateNTP() {
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
}

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
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
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
