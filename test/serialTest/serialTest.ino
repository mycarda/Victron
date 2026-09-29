#include "VeDirectFrameHandler.h"
#include <U8x8lib.h>

VeDirectFrameHandler myve;

#define rxPin 3
#define txPin 4

unsigned int counter = 0;

void setup() {
  Serial.begin(9600);
  delay(3000);
  Serial.println("serialTest: init");

  Serial1.begin( 19200, SERIAL_8N1, rxPin, txPin );
  Serial1.flush();
  Serial.println("serialTest: start");
}

void loop() {    
    ReadVEData();
    EverySecond();
}

void ReadVEData() {
    while ( Serial1.available() ) {
        myve.rxData(Serial1.read());
    }
    yield();
}

void EverySecond() {
    static unsigned long prev_millis;

    if (millis() - prev_millis > 1000) {
        PrintData();
        prev_millis = millis();
    }
}

void PrintData() {
    Serial.println("serialTest: print");
    for ( int i = 0; i < myve.veEnd; i++ ) {
    Serial.print(myve.veName[i]);
    Serial.print("= ");
    Serial.println(myve.veValue[i]);    
    }
}