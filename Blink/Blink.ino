
int PIN[] = {3,4,5,6,7,8};

int BUT[] = {9,10,11,12};

int ORD[] = {0,0,0,0};

int VAL[] = {0,0,0,0};

void setup() {   
Serial.begin(9600);  

  for (int i = 0; i<6; i++) {
    pinMode(PIN[i], OUTPUT);
  }

  for (int i = 0; i<5; i++) {
    pinMode(BUT[i], INPUT_PULLUP);
  }

}

void controller(int v, int i) {
 switch (v) {
    case 1:
    if ( VAL[i] == 0 ) {
      Serial.println("case 1");
      digitalWrite(PIN[0], HIGH);
      digitalWrite(PIN[2], HIGH);
      digitalWrite(PIN[4], HIGH);
      ORD[i] = 1;
      break;
      }
      
    if ( VAL[i] == 1 && ORD[i] == 1 ) {
      digitalWrite(PIN[0], LOW);
      digitalWrite(PIN[2], LOW);
      digitalWrite(PIN[4], LOW);
      break;
      }
      
   case 2:
   if ( VAL[i] == 0 ) {
     Serial.println("case 2");
     digitalWrite(PIN[1], HIGH);
     digitalWrite(PIN[3], HIGH);
     digitalWrite(PIN[4], HIGH);
     ORD[i] = 1;
     break;
     }
     
   if ( VAL[i] == 1 && ORD[i] == 1 ) {
     digitalWrite(PIN[1], LOW);
     digitalWrite(PIN[3], LOW);
     digitalWrite(PIN[4], LOW);
     ORD[i] = 0;
     break;
     }
     
   case 3:
   if ( VAL[i] == 0 ) {
     Serial.println("case 3");
     digitalWrite(PIN[0], HIGH);
     digitalWrite(PIN[2], HIGH);
     digitalWrite(PIN[5], HIGH);
     ORD[i] = 1;
     break;
     }
     
  if ( VAL[i] == 1 && ORD[i] == 1 ) {
     digitalWrite(PIN[0], LOW);
     digitalWrite(PIN[2], LOW);
     digitalWrite(PIN[5], LOW);     
     ORD[i] = 0;
     break;
     }
     
  case 4:
  if ( VAL[i] == 0 ) {
     Serial.println("case 4");
     digitalWrite(PIN[1], HIGH);
     digitalWrite(PIN[3], HIGH);  
     digitalWrite(PIN[5], HIGH);
     ORD[i] = 1;
     break;
     }
     
  if ( VAL[i] == 1 && ORD[i] == 1 ) {
     digitalWrite(PIN[1], LOW);
     digitalWrite(PIN[3], LOW);
     digitalWrite(PIN[5], LOW);     
     ORD[i] = 0;
     break;
    }
    
}

}

void cont(int v, int i) {
 switch (v) {
    case 1:
    if ( i == 0 ) {
      digitalWrite(PIN[0], HIGH);
      digitalWrite(PIN[2], HIGH);
      digitalWrite(PIN[4], HIGH);
      break;
      }
      
    if ( i == 1 ) {
      digitalWrite(PIN[0], LOW);
      digitalWrite(PIN[2], LOW);
      digitalWrite(PIN[4], LOW);
      break;
      }
      
   case 2:
   if ( i == 0 ) {
     digitalWrite(PIN[1], HIGH);
     digitalWrite(PIN[3], HIGH);
     digitalWrite(PIN[4], HIGH);
     break;
     }
     
   if ( i == 1 ) {
     digitalWrite(PIN[1], LOW);
     digitalWrite(PIN[3], LOW);
     digitalWrite(PIN[4], LOW);
     break;
     }
     
   case 3:
   if ( i == 0 ) {
     digitalWrite(PIN[0], HIGH);
     digitalWrite(PIN[2], HIGH);
     digitalWrite(PIN[5], HIGH);
     break;
     }
     
  if ( i == 1 ) {
     digitalWrite(PIN[0], LOW);
     digitalWrite(PIN[2], LOW);
     digitalWrite(PIN[5], LOW);     
     break;
     }
     
  case 4:
  if ( i == 0 ) {
     digitalWrite(PIN[1], HIGH);
     digitalWrite(PIN[3], HIGH);  
     digitalWrite(PIN[5], HIGH);
     break;
     }
     
  if ( i == 1 ) {
     digitalWrite(PIN[1], LOW);
     digitalWrite(PIN[3], LOW);
     digitalWrite(PIN[5], LOW);     
     break;
    }
    
}

}


void loop() {

for (int i = 0; i<4; i++) {
  int v = i+1;
  VAL[i] = digitalRead(BUT[i]);  
  controller(v,i);
}

delay(10);

}


/*
  cont(1,0);
  cont(3,0);
  
  delay(1000);

  cont(1,1);
  cont(3,1);

  delay(10);
  
  cont(2,0);
  cont(4,0);
  
  delay(1000);

  cont(2,1);
  cont(4,1);

  delay(10);
*/
