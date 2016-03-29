const int ledPin = 13;
const int buttonPin = 13;

int state = 0;
int pause = 0;
int buttonValue;
int sensorValue;

int analogPins[] = {
  A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15 }; 
  
int potsVals[] = {
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };

int potsLeds[] = {
  20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 };  
  
void setup() {
    Serial.begin(31250);
    pinMode(ledPin, OUTPUT);
    pinMode(buttonPin, INPUT);
    digitalWrite(ledPin, LOW);
    for (int i=0; i < 16; i++) {
      pinMode(potsLeds[i], OUTPUT);
    }
}

void noteOn(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
  }


void loop() {
  for (int i=0; i < 16; i++) {
        
    buttonValue = digitalRead(buttonPin);
    
    if (buttonValue == HIGH && state == 0) {
      digitalWrite(ledPin, HIGH);
      state = 1;
    }
    if (buttonValue == LOW && state == 1) {
      pause = 1;
    }
    if (buttonValue == HIGH && pause == 1) {
      digitalWrite(ledPin, LOW);
      pause = 0;
    }
    if (buttonValue == LOW && pause == 0) {
      state = 0;
    }
    sensorValue = analogRead(analogPins[i]); 
    sensorValue = sensorValue / 8;
    int cntrl = 176 + i;
    if (sensorValue <= 3) {
      sensorValue = 0;
      digitalWrite(potsLeds[i], LOW);
      noteOn(cntrl, 7, sensorValue); 
    } else {

      digitalWrite(potsLeds[i], HIGH);

      /*if (pause == 1) {
        delay(10);
      } else {  
      */
      
      
      //if ( abs(sensorValue - potsVals[i]) > 3 ) {
        noteOn(cntrl, 7, sensorValue); 
      //}
      
      potsVals[i] = sensorValue;
      delay(10);

     }
    }
}

  

