#include "VeDirectFrameHandler.h"
#include <U8x8lib.h>
#include "config.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

VeDirectFrameHandler myve;
WiFiMulti wifiMulti;
HTTPClient http;

#define rxPin 8
#define txPin 9

unsigned int counter = 0;
unsigned int lastGoodSSID = 9;
int status = WL_DISCONNECTED;

void setup() {
  Serial.begin(19200);
  delay(3000);
  Serial.println("serialTest: init");

  Serial1.begin( 19200, SERIAL_8N1, rxPin, txPin );
  Serial1.flush();
  status = ConnectWiFi();
  Serial.println("serialTest: start");
}

void loop() {    
    ReadVEData();
    EveryMinute();
}

void ReadVEData() {
    while ( Serial1.available() ) {
        myve.rxData( Serial1.read() );
    }
    yield();
}

void EveryMinute() {
    static unsigned long prev_millis;

    if (millis() - prev_millis > 60000) {
        PrintData();
        // Serial.println("EveryMinute: sending");
        if ( status == WL_CONNECTED ) {
          String url = "http://" + String(LOG_HOST) + ":" + String(LOG_PORT) + "/putData.php?data=$V,";
          for ( int i = 0; i < myve.veEnd; i++ ) {
            url.concat( myve.veName[i] );
            url.concat( "=" );
            url.concat( myve.veValue[i] );
            if ( i < myve.veEnd - 1 ) {
              url.concat( "," );
            }
          }
        //   Serial.printf("EveryMinute: myve.veEnd %i\n", myve.veEnd );
          url.replace("#","N");
          Serial.printf("EveryMinute: url %s\n", url.c_str());
          HTTPSend( url );
        }
        else {
          status = ConnectWiFi();
        }
        prev_millis = millis();
    }
}

int QuickConnect( int SSIDIndex) {
  int retryCount = 10;

  Serial.printf("QuickConnect: quick connecting to %s ", wifiSSID[ SSIDIndex ] );

  WiFi.begin( wifiSSID[ SSIDIndex ], wifiPassword[ SSIDIndex ] );
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  
  while ( WiFi.status() != WL_CONNECTED ) {
    Serial.print("." );
    retryCount--;
    if ( retryCount < 0 ) {
      Serial.printf("\nQuickConnect: failed to connect with status %u\n", WiFi.status() );
      return WiFi.status();
    }
    delay(1000);
  }
  Serial.println();

  if ( WiFi.status() == WL_CONNECTED ) {
    Serial.printf("QuickConnect: *** CONNECTED *** to %s with %s, WiFI RSSI %d\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI() );
  }
  return WiFi.status();
}

unsigned int WhichSSID() {

  for ( int i = 0; i < numSSIDs; i++ ) {
    if ( strcmp( wifiSSID[i], WiFi.SSID().c_str() ) == 0)
    Serial.printf("WhichSSID: using %u %s\n", i, WiFi.SSID().c_str() );
    return i;
  }

  return 9;
}

int ConnectWiFi() {
  
  char hostname[16];
  sprintf( hostname, "esp32-Victron" );
  WiFi.setHostname( hostname );
  Serial.printf( "ConnectWiFi: hostname set to %s\n", hostname );

  if ( lastGoodSSID  < numSSIDs ) {
    if ( QuickConnect( lastGoodSSID ) == WL_CONNECTED ) {
      return WL_CONNECTED;
    }
  } 

  Serial.println("wifi multi: connecting" );
  
  for ( int i = 0; i < numSSIDs; i++ ) {
    wifiMulti.addAP( wifiSSID[i], wifiPassword[i] );
  }

  // WiFi.setTxPower(WIFI_POWER_8_5dBm);
  wifiMulti.run();
  
  if ( WiFi.status() == WL_CONNECTED ) { 
    Serial.printf("wifi multi: *** CONNECTED *** to %s with %s, WiFI RSSI %d\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI() );
    lastGoodSSID = WhichSSID();
    Serial.printf("wifi multi: setting lastGoodSSID to index %u for %s\n", lastGoodSSID, WiFi.SSID().c_str() );
  }
  else {
    Serial.printf("wifi multi: failed to connect with status %u\n", WiFi.status() );
  }

  return WiFi.status();
}

void PrintData() {
    Serial.println("serialTest: print");
    for ( int i = 0; i < myve.veEnd; i++ ) {
      Serial.print(myve.veName[i]);
      Serial.print("=");
      Serial.println(myve.veValue[i]);
      if ( i < myve.veEnd - 1 ) {
        Serial.print(",");
      }
      else {
        Serial.println();
      }
    }
}

void HTTPSend( String putData ) {

  Serial.println("HTTPSend: start");
  Serial.printf("http send: sending %s\n", putData.c_str() );

  HTTPClient http;
   
  http.begin( putData );
  int httpCode = http.GET();

  // httpCode will be negative on error
  if(httpCode > 0) {
    // HTTP header has been sent and Server response header has been handled
    Serial.print("log send: written ");
    Serial.print(putData);
    Serial.printf(" with code = %d\n", httpCode);
  } 
  else {
    Serial.printf("log send: [HTTP] GET failed, error: %d %s\n", httpCode, http.errorToString(httpCode).c_str());
  }

  http.end(); 
}
