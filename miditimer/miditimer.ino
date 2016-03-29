#include "Timer.h"

Timer t;

int tempo;
int note;


static const uint8_t buttonPin[] = {
  A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15 };

static const uint8_t ledPin[] = {
  2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17 };

int buttonState[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

int buttonPushed[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  

int volatile Pin = -1;

int volatile stepsSlot[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

int volatile stepNoteSlot[] = {
  60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60};


long lastmillis = 0;

int state = 0;

int sw = 0;

long num = 0;


void setup()
{
  Serial.begin(9600);
//  Serial.begin(31250);
  
  pinMode(18, OUTPUT);
  pinMode(19, INPUT_PULLUP);
  
  for (int v=0; v < 16; v++ ) {
    pinMode(ledPin[v], OUTPUT);      
  }
  
  for (int v=0; v < 16; v++ ) {
    pinMode(buttonPin[v], INPUT_PULLUP);
 }
}


void loop()
{
//####################################################################################################

  if (digitalRead(19) == 0 && state == 0) {
    state = 1;
    if (sw == 1) {
      sw = 0; 
    } else {
      sw = 1;
    }
  }
  
  if (digitalRead(19) == 1 && state == 1) {
    state = 0;
  }

  if (sw == 1) {
    digitalWrite(18, HIGH);
  }   

  if (sw == 0) {
    digitalWrite(18, LOW);
  }

//####################################################################################################

  if (tempo == 0) {
    tempo = 83;
    int tickEvent = t.every(tempo, playNote);
  }
    
  t.update();
  
//####################################################################################################
//# Nacteni stavu pinu

  for (int thisPin=0; thisPin < 16; thisPin++) {
      
      buttonState[thisPin] = analogRead(buttonPin[thisPin]);

//##############################################################################################
//# Zapnuti a vypnuti stepu

      if (buttonState[thisPin] > 1000 && buttonPushed[thisPin] == 0) {
        buttonPushed[thisPin] = 1;

        if (stepsSlot[thisPin] == 0 ) {
          stepsSlot[thisPin] = 1;
          digitalWrite(ledPin[thisPin], HIGH);
        } else {
          stepsSlot[thisPin] = 0;
          digitalWrite(ledPin[thisPin], LOW);
        }
      }

//##############################################################################################
//# Navyseni noty

      if (buttonState[thisPin] < 1024 && buttonState[thisPin] > 500 && buttonPushed[thisPin] == 0) {
        buttonPushed[thisPin] = 1;
       
        if (stepNoteSlot[thisPin] != 78) {
          stepNoteSlot[thisPin]++;
          if (sw == 1) { 
           noteOn(0x90,stepNoteSlot[thisPin] , 0x45);
         }
        }
      }

//##############################################################################################
//# Snizeni noty

      if (buttonState[thisPin] < 500 && buttonState[thisPin] > 300 && buttonPushed[thisPin] == 0) {
        buttonPushed[thisPin] = 1;
        
        if (stepNoteSlot[thisPin] != 60) {
           stepNoteSlot[thisPin]--;
         if (sw == 1){ 
           noteOn(0x90,stepNoteSlot[thisPin] , 0x45);
         }
        }
      }

//##############################################################################################
//# Odemknuti

      if (buttonState[thisPin] < 300 && buttonPushed[thisPin] == 1) {
        buttonPushed[thisPin] = 0;
        noteOn(0x90,stepNoteSlot[thisPin] , 0x00);
      }
  }
  
//####################################################################################################
//# Zobrazeni poctu spusteni

  if (millis() - lastmillis >= 1000) {
    lastmillis = millis();
    Serial.println(num);
  }     

  num++;
  
//####################################################################################################

}


void playNote()
{ 
  if (stepsSlot[Pin] == 0 ) {
    digitalWrite(ledPin[Pin], LOW);
  }

  if (Pin == 15) {
    Pin = -1;
  }

  noteOn(0x90, note, 0x00);
  Pin++;
  note = stepNoteSlot[Pin];  

  if (stepsSlot[Pin] == 1) {
    noteOn(0x90, stepNoteSlot[Pin], 0x45);
  }

  digitalWrite(ledPin[Pin], HIGH);
}

void noteOn(int cmd, int pitch, int velocity) {
//  Serial.write(cmd);
//  Serial.write(pitch);
//  Serial.write(velocity);
// Serial.print(cmd);
//  Serial.print(" - ");
//  Serial.print(pitch);
// Serial.print(" - ");
//  Serial.println(velocity);
}
