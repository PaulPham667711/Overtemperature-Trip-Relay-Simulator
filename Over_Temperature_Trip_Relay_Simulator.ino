#include <ESP32Servo.h>
Servo myServo;
const int dangerousSignal = 4;
const int warningSignal = 17;
const int safetySignal = 2;
const int countingDownSignal = 16;
const int SensorPin = 32;
const long interval = 1000;
unsigned long previousMillis = 0;
const int tripTemp = 100;
const int resetTemp = 75;
bool inFault = false;
bool tripped = false;
unsigned long faultStartTime = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
  pinMode(dangerousSignal, OUTPUT);
  pinMode(countingDownSignal, OUTPUT);
  pinMode(warningSignal, OUTPUT);
  pinMode(safetySignal, OUTPUT);
  digitalWrite(dangerousSignal, LOW);
  digitalWrite(countingDownSignal, LOW);
  digitalWrite(warningSignal, LOW);
  digitalWrite(safetySignal, HIGH);
  myServo.attach(14);
  myServo.write(0);
  Serial.println("System is operating");
  }

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {previousMillis= currentMillis;
  int rawADC = analogRead(SensorPin);
  long Temperature = map(rawADC, 0, 4095, 20, 140);
  if (tripped) 
  {
    if (Temperature < resetTemp)
    {
      tripped = false;
      inFault = false;
    } else if (Temperature >= tripTemp)
    { 
      myServo.write(90);
      digitalWrite(dangerousSignal, HIGH);
      digitalWrite(countingDownSignal, LOW);
      digitalWrite(warningSignal, LOW);
      digitalWrite(safetySignal, LOW);
      Serial.println("SYSTEM HAS BEEN TRIPPED - FAULT STILL ACTIVE");

    }
    else 
    {
      myServo.write(90);
      digitalWrite(dangerousSignal, HIGH);
      digitalWrite(countingDownSignal, LOW);
      digitalWrite(warningSignal, HIGH);
      digitalWrite(safetySignal, LOW);
      Serial.println("SYSTEM TRIPPED - LATCHED AWAITING RESET BELOWS 75");

    }
  }
  if (!tripped) {
  if (Temperature >= tripTemp){ 
   if (!inFault)
  {
    inFault = true;
    faultStartTime = currentMillis;
  }
  unsigned long elapsed = currentMillis - faultStartTime;
  unsigned long waitTime = 2000/(1+Temperature-tripTemp);
    if (elapsed >= waitTime){
      tripped = true;
     myServo.write(90);
     digitalWrite(dangerousSignal, HIGH);
     digitalWrite(countingDownSignal, LOW);
     digitalWrite(warningSignal, LOW);
     digitalWrite(safetySignal, LOW);
     Serial.println("SYSTEM HAS BEEN TRIPPED");
}
   else {
  myServo.write(0);
  digitalWrite(dangerousSignal, LOW);
  digitalWrite(countingDownSignal, HIGH);
  digitalWrite(warningSignal, LOW);
  digitalWrite(safetySignal, LOW);
  Serial.println("WARNING, TEMPERATURE EXCEEDING MAXIMUM!!!STARTING TO COUNTDOWN");
}
}

  else if(Temperature < tripTemp && Temperature >= resetTemp){
  inFault = false;
  myServo.write(0);
  digitalWrite(dangerousSignal, LOW);
  digitalWrite(countingDownSignal, LOW);
  digitalWrite(warningSignal, HIGH);
  digitalWrite(safetySignal, LOW);
  Serial.println("CAUTIOUS! System's temperature is above average.");
    }
  else if(Temperature < resetTemp){
  inFault = false;
  myServo.write(0);
  digitalWrite(dangerousSignal, LOW);
  digitalWrite(countingDownSignal,LOW);
  digitalWrite(warningSignal, LOW);
  digitalWrite(safetySignal, HIGH);
  Serial.println("System is operating");
    }
  }
}
}

