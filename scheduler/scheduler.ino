// Import required libraries
#include <SdFat.h>
#include <CSVFile.h>
#include "TimeLib.h"
#include "TimeAlarms.h"
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

//================ Skce urcena pro upravy nastaveni

#define PIN_SPI_CLK 13
#define PIN_SPI_MOSI 11
#define PIN_SPI_MISO 12
#define PIN_SD_CS 10

#define PIN_OTHER_DEVICE_CS -1
#define SD_CARD_SPEED SPI_FULL_SPEED 
#define FILENAME "CSV.csv"


#define ALLTIME -1

// action
#define UP 1
#define DOWN 0

// type
#define SWITCH 0
#define BUTTON 1

// subtype
#define SCHED 0
#define MAX 1
#define MIN 2

// sensor
#define NONE 0
#define TEMP 1
#define HUM 2

// Zde je potreba pojemnovat konkretni piny a zapsat je do pole pins

#define LED1 12
#define LED2 11

const int pins[] = {LED1, LED2};

typedef struct {
  int h;
  int m;
  int s;
  int arg ;
  int type;
  int subtype;
  int sensor;
  int action;
  int pin;
} Action;

Action actions[] = {
  // Ukazka akce SWITCH, umoznuje zapnout, anebo vypnout pomoci hodnoty "action" v pravidelny cas
  {
    // Tato definice znamena kazdou prvni sekundu v minute, kazdou hodinu
    .h = ALLTIME,
    .m = ALLTIME,
    .s = 31,
    .arg = 0,
    .type = SWITCH,
    .subtype = SCHED,
    .sensor = NONE,
    // UP, tedy zapnout
    .action = UP,
    .pin = LED2,
  },
  // Ukazka akce BUTTON, umoznuje provest libovolne dlouhe zapnuti a nasledne vypnuti
  {
    .h = ALLTIME,
    .m = ALLTIME,
    .s = 1,
    .arg = 1000,
    .type = BUTTON,
    .subtype = MAX,
    .sensor = TEMP,
    .action = 3000,
    .pin = LED1,
  },
};

//=========================================================================s

SdFat sd;
CSVFile csv;

const byte BUFFER_SIZE = 5;
char buffer[BUFFER_SIZE + 1];

int sleep[sizeof(actions)/sizeof(Action)];
unsigned long lastmillis[sizeof(actions)/sizeof(Action)];

uint32_t delayMS;

int temp;
int hum;

const byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
const int NTP_PACKET_SIZE = 48;

byte packetBuffer[NTP_PACKET_SIZE];

const unsigned int localPort = 8888;
const int timeZone = 1;

//IPAddress timeServer(91, 206, 8, 36);
IPAddress timeServer(129, 6, 15, 28);

#define DHTPIN 2

#define DHTTYPE DHT22

DHT_Unified dht(DHTPIN, DHTTYPE);

EthernetUDP UDP;

void StartNET() {
  Ethernet.init(10);
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println(F("Ethernet cable is not connected."));
    }
  }
  UDP.begin(localPort);
}

bool TimeCheck(int i) {
  int h = hour();            // the hour now  (0-23)
  int m = minute();          // the minute now (0-59)
  int s = second();
  if (actions[i].s == s || actions[i].s == ALLTIME) {
    if (actions[i].s == m || actions[i].m == ALLTIME) {
      if (actions[i].s == h || actions[i].h == ALLTIME) {
        return true;
      }
    }
  }
  return false;
}

bool SubtypeCheck(int i, int arg) {
  if (actions[i].subtype == MIN) {
    if (actions[i].arg < arg) return true;
  }
  if (actions[i].subtype == MAX) {
    if (actions[i].arg > arg) return true;
  }
  return false;
}

void printTime() {
  int h = hour();            // the hour now  (0-23)
  int m = minute();          // the minute now (0-59)
  int s = second();
  Serial.print(h);
  Serial.print(F(":"));
  Serial.print(m);
  Serial.print(F(":"));
  Serial.println(s);
}

void Worker(int i, int t) {
  if (t != 0) {
    digitalWrite(actions[i].pin, UP);
    editPin(actions[i].pin,"1");
    lastmillis[i] = millis();
    sleep[i] = t;
  } else {
    if (actions[i].action == UP || actions[i].action == DOWN) { 
      digitalWrite(actions[i].pin, actions[i].action);    
      if (actions[i].action == UP) editPin(actions[i].pin,"1");
      if (actions[i].action == DOWN) editPin(actions[i].pin,"0");
    }
  }
}

void TypeCheck(int i) {
  printTime();
  Serial.print(F("TEMP = "));
  Serial.print(temp);
  Serial.print(F(", HUM = "));
  Serial.println(hum);
  Alarm.delay(10);  
  if (actions[i].type == SWITCH) {
    Serial.print(F("SWITCH - "));
    Serial.println(actions[i].pin);
    if (actions[i].sensor == TEMP && actions[i].subtype != SCHED) {
      if (SubtypeCheck(i,temp)) Worker(i,0);
      return;
    }
    if (actions[i].sensor == HUM && actions[i].subtype != SCHED) {
      if (SubtypeCheck(i,hum)) Worker(i,0);
      return;
    }
    if (actions[i].sensor == NONE) {
      Worker(i,0);
      return;     
    } 
  }
  if (actions[i].type == BUTTON) {
    Serial.print(F("BUTTON - "));
    Serial.println(actions[i].pin);
    if (actions[i].sensor == TEMP && actions[i].subtype != SCHED) {
      if (SubtypeCheck(i,temp)) Worker(i,actions[i].action);
      return;
    }
    if (actions[i].sensor == HUM && actions[i].subtype != SCHED) {
      if (SubtypeCheck(i,hum)) Worker(i,actions[i].action);
      return;
    }
    if (actions[i].sensor == NONE) {
      Worker(i,actions[i].action);
      return;      
    }
  }
}

void Scheduler() {
  for (int i = 0; i < sizeof(actions)/sizeof(Action); i++) {
    if (TimeCheck(i)) TypeCheck(i);
  }
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
  for (int i = 0; i <= sizeof(actions)/sizeof(Action); i++) {
    if (sleep[i] != 0) {
      if (millis() - lastmillis[i] >= sleep[i]) {
        digitalWrite(actions[i].pin, DOWN);
        editPin(actions[i].pin,"0");
        sleep[i] = 0;
      }
    }
  }
}
//=========================================================

void editPin(int p, char s) {
  for (int i = 0; i <= sizeof(pins)/sizeof(int); i++) {
    if (pins[i] == p) {
      csv.gotoField(p);
      csv.editField(s);
    }
  }
}

void initSdFile() {
  if (sd.exists(FILENAME) && !sd.remove(FILENAME))
  {
    Serial.println(F("Failed init remove file"));
    return;
  }
  if (!csv.open(FILENAME, O_RDWR | O_CREAT)) {
    Serial.println(F("Failed open file"));
  }
}

time_t getNtpTime() {
  while (1) {
    while (UDP.parsePacket() > 0) ; // discard any previously received packets
    Serial.println(F("Transmit NTP Request"));
    sendNTPpacket(timeServer);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
      int size = UDP.parsePacket();
      if (size >= NTP_PACKET_SIZE) {
        Serial.println(F("Receive NTP Response"));
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
    Serial.println(F("No NTP Response :-("));
    Alarm.delay(3000);
  }
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

void setup() {
  Serial.begin(115200);
  for (int i = 0; i <= sizeof(pins)/sizeof(int); i++) {
    pinMode(pins[i], OUTPUT);
  }
    // Setup pinout
  pinMode(PIN_SPI_MOSI, OUTPUT);
  pinMode(PIN_SPI_MISO, INPUT);
  pinMode(PIN_SPI_CLK, OUTPUT);
  //Disable SPI devices
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);
  
  #if PIN_OTHER_DEVICE_CS > 0
  pinMode(PIN_OTHER_DEVICE_CS, OUTPUT);
  digitalWrite(PIN_OTHER_DEVICE_CS, HIGH);
  #endif //PIN_OTHER_DEVICE_CS > 0
  
  StartNET();
  setSyncProvider(getNtpTime);
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
  
  initSdFile();
  buffer[BUFFER_SIZE] = '\0';

  for (int i = 0; i <= sizeof(pins)/sizeof(int); i++) {
    csv.gotoField(i);
    csv.readField(buffer, BUFFER_SIZE);
    if (buffer == "1") digitalWrite(pins[i], UP);
  }
  
  Alarm.timerRepeat(1, Scheduler);
}
