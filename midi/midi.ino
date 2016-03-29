
// Piny pro tlacitka
int buttonPin[] = {
  A0,10,A4,A2,A1,11,A3,A5,12,13 }; 

// Piny pro diody
int ledPin[] = {
  2,4,3,5,6,7,9,8 };
  
// Prubehovy stav tlacitka
int buttonState[] = {
  0,0,0,0,0,0,0,0 };

// Aktualni stav tlacitka, nutno pro ziskani samostatneho zmacknuti behem cyklu
int buttonPushed[] = {
  0,0,0,0,0,0,0,0 };

// Pole, kde jsou ulozeny informace o vsech krocich, zda maji hrat 
int stepsSlot[] = {
  0,0,0,0,0,0,0,0 };

// Identifikator pro vypis na consoli
int buttonNum = 0;

// Pocty diod a tlacitek, pro dynamicke skalovani
int DiodeCount = 8;
int ButtonCount = 10;

int NoteSlot[] = {
  30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89 };

int StepNoteSlot[] = {
  30,30,30,30,30,30,30,30 };

// Promena kterou se predava 1/0, zda ma aktualni krok hrat
int lastPlayStep = 0;

int noteStep = 0;

void setup() {
  Serial.begin(31250);
  //Serial.begin(9600);
  // Nastaveni diod
  for (int thisPin=0; thisPin < DiodeCount; thisPin++ ) {
    pinMode(ledPin[thisPin], OUTPUT);      
  }
  // Nastaveni tlacitek
  for (int thisPin=0; thisPin < ButtonCount; thisPin++ ) {
    pinMode(buttonPin[thisPin], INPUT_PULLUP);
  }
}

// promena i nese aktualni cislo kroku a predava se funkci readSwitches
void loop(){
  for (int i=0; i<DiodeCount; i++) {  
    int tempo = 250;
    // Precteni stavu tlacitek
    readSwitches(i);
    stepSetup(i);
    noteSetup();
    //for (int y=0; y<DiodeCount; y++) {  
    //  Serial.print(StepNoteSlot[y]);
    //  Serial.print(",");
    //}
    //Serial.println();
    digitalWrite(ledPin[i], HIGH);
    if (lastPlayStep == 1) {
      // Zapnuti diody
      
      // Zapnuti noty
      noteOn(0x90, StepNoteSlot[i], 0x45);
      // Prodleva se zapnutou notou
      delay(100);
      // Vypnuti noty
      noteOn(0x90, StepNoteSlot[i], 0x00); 
      // Odecteni prodlevy od pauzy tempa
      tempo = tempo - 100;
    }
    // Pauza tempa
    delay (tempo);      
    if (lastPlayStep != 1) {
    digitalWrite(ledPin[i], LOW);      
    }
    // Vypnuti dio<dy

  }
}

// Precteni stavu vsech tlacitek, zaznamenani o zapnutych a vypnutych, predani informace zpet do hlavni smycky
void readSwitches(int val)
{
  // Cyklus, ktery precte stav vsech tlacitek
  for (int thisPin=0; thisPin < DiodeCount; thisPin++ ) {
      buttonState[thisPin] = digitalRead(buttonPin[thisPin]);
      buttonNum = buttonPin[thisPin];
      
      // Debug informace
      //Serial.print(buttonNum);
      //Serial.print(" - ");
      //Serial.print(buttonState[thisPin]);
      //Serial.print(" - ");
      //Serial.println(stepsSlot[thisPin]);

      // Overeni ze tlacitko bylo zmacknuto a ulozeni informace
      if (buttonState[thisPin] == LOW && buttonPushed[thisPin] == 0) {
        buttonPushed[thisPin] = 1;
        
        // Debug informace
        //Serial.print("Zmacknuto - ");
        //Serial.println(buttonNum);

        // prideleni priznak kroku a rozsviceni diody na prislusnem kroku, ze ma hrat a pri pristim zmacknuti prestane a zhasne diodu 
        if (stepsSlot[thisPin] == 0 ) {
          stepsSlot[thisPin] = 1;
          digitalWrite(ledPin[thisPin], HIGH);          
        } else {
          stepsSlot[thisPin] = 0;
          digitalWrite(ledPin[thisPin], LOW);
        }
      } 

      // Uvolneni informace o zmacknutem tlacitku, jinac by dochazelo k neustalemu generovani zmacknuti
      if (buttonState[thisPin] == HIGH && buttonPushed[thisPin] == 1) {
        buttonPushed[thisPin] = 0;
      }
  }  
  // Predani informace o kroku do hlavni smycky
  lastPlayStep =  stepsSlot[val];
}

void noteSetup() {
  int Note = StepNoteSlot[noteStep];
   if (digitalRead(12) == LOW && Note != 0 ) {
     Note--;
   }
   if (digitalRead(13) == LOW && Note != 57 ) {
     Note++;
   }
   StepNoteSlot[noteStep] = Note;
}

void stepSetup(int val) {
  int sw = 0;
  int bt = val;
  if ( digitalRead(12) == LOW && digitalRead(13) == LOW ) {
    delay (1000);
    while ( sw != 1 ) {
      delay (150);
      digitalWrite(ledPin[bt], HIGH);          
      if (digitalRead(12) == LOW  && bt != 0) {
        digitalWrite(ledPin[bt], LOW);          
        bt--;
        digitalWrite(ledPin[bt], HIGH);          
      }
      if (digitalRead(13) == LOW  && bt != 7 ) {
        digitalWrite(ledPin[bt], LOW);          
        bt++;
        digitalWrite(ledPin[bt], HIGH);          
      }

      if ( digitalRead(19) == LOW ) {
        digitalWrite(ledPin[bt], LOW);          
        noteStep = bt;
        sw = 1;
        delay (1000);
      }  
    }
  }
       
}

// Odeslani noty do midi
void noteOn(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}
