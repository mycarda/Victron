#include "config.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <rom/rtc.h>
#include "esp_mac.h"

// pins valid for esp32 super mini
#define LED_PIN 8
#define SDA 6
#define SCL 7
#define SERIAL PrependTime();Serial

#define uS_TO_MINUTES 60000000  // Conversion factor for micro seconds to minutes (60 million)
#define SLEEP_15_MINUTES 900000000 // 15 minutes in micro seconds

RTC_DATA_ATTR unsigned int lastGoodSSID = 9;

uint8_t mac_base[6] = {0};
int status = WL_DISCONNECTED;

// AHT20 aht20;
WiFiMulti wifiMulti;
HTTPClient http;

void PrependTime() {
  unsigned long millisNow = millis();
  unsigned long hours = millisNow / 3600000;
  unsigned long minutes = millisNow / 60000;
  unsigned long seconds = millisNow / 1000;
  unsigned long milliSeconds = millisNow-seconds*1000;
  Serial.printf("%02d:%02d:%02d.%03d ", hours%24, minutes%60, seconds%60, milliSeconds );  
}

unsigned int WhichSSID() {

  for ( int i = 0; i < numSSIDs; i++ ) {
    if ( strcmp( wifiSSID[i], WiFi.SSID().c_str() ) == 0)
    SERIAL.printf("WhichSSID: using %u %s\n", i, WiFi.SSID().c_str() );
    return i;
  }

  return 9;
}

int QuickConnect( int SSIDIndex) {
  int retryCount = 10;

  SERIAL.printf("QuickConnect: quick connecting to %s ", wifiSSID[ SSIDIndex ] );

  WiFi.begin( wifiSSID[ SSIDIndex ], wifiPassword[ SSIDIndex ] );
  // WiFi.setTxPower(WIFI_POWER_8_5dBm);
  
  while ( WiFi.status() != WL_CONNECTED ) {
    Serial.print("." );
    retryCount--;
    if ( retryCount < 0 ) {
      SERIAL.printf("\nQuickConnect: failed to connect with status %u\n", WiFi.status() );
      return WiFi.status();
    }
    delay(1000);
  }
  Serial.println();

  if ( WiFi.status() == WL_CONNECTED ) {
    SERIAL.printf("QuickConnect: *** CONNECTED *** to %s with %s, WiFI RSSI %d\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI() );
  }
  return WiFi.status();
}

int ConnectWiFi() {
  
  // WiFi.mode(WIFI_STA);
  // WiFi.begin();
  // WiFi.setAutoReconnect(false);
  char hostname[16];
  sprintf( hostname, "esp32-Victron" );
  WiFi.setHostname( hostname );
  SERIAL.printf( "ConnectWiFi: hostname set to %s\n", hostname );

  if ( lastGoodSSID  < numSSIDs ) {
    if ( QuickConnect( lastGoodSSID ) == WL_CONNECTED ) {
      return WL_CONNECTED;
    }
  } 

  SERIAL.println("wifi multi: connecting" );
  
  for ( int i = 0; i < numSSIDs; i++ ) {
    wifiMulti.addAP( wifiSSID[i], wifiPassword[i] );
  }

  // WiFi.setTxPower(WIFI_POWER_8_5dBm);
  wifiMulti.run();
  
  if ( WiFi.status() == WL_CONNECTED ) { 
    SERIAL.printf("wifi multi: *** CONNECTED *** to %s with %s, WiFI RSSI %d\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI() );
    lastGoodSSID = WhichSSID();
    SERIAL.printf("wifi multi: setting lastGoodSSID to index %u for %s\n", lastGoodSSID, WiFi.SSID().c_str() );
  }
  else {
    SERIAL.printf("wifi multi: failed to connect with status %u\n", WiFi.status() );
  }

  return WiFi.status();
}

void HTTPSend( String putData ) {

  //  SERIAL.printf("http send: sending %s\n", putData.c_str() );

  HTTPClient http;
   
  http.begin( putData );
  int httpCode = http.GET();

  // httpCode will be negative on error
  if(httpCode > 0) {
    // HTTP header has been sent and Server response header has been handled
    SERIAL.print("log send: written ");
    Serial.print(putData);
    Serial.printf(" with code = %d\n", httpCode);
  } 
  else {
    SERIAL.printf("log send: [HTTP] GET failed, error: %d %s\n", httpCode, http.errorToString(httpCode).c_str());
  }

  http.end(); 
}

void setup(void)
{ 
  Serial.begin(115200);
  delay(1000);
  Serial.println("Victron relay");

  delay(250);
}

void loop(void)
{ 
  // static char temperatureString[5];
  // static char humidityString[6];
  // dtostrf( aht20.getTemperature(), 3, 1, temperatureString );
  // dtostrf( aht20.getHumidity(), 3, 1, humidityString );

  // SERIAL.printf("%s C, %s %%\n", temperatureString, humidityString );

  String url = "http://" + String(LOG_HOST) + ":" + String(LOG_PORT) +
                "/putData.php?data=$Z," +
                "10.4" + "," + 
                "56.3" + "," + 
                "123456";
  SERIAL.println(url);
  // SERIAL.printf("Device ID: %s\n", String(deviceID) );

  if ( status == WL_CONNECTED ) {
    HTTPSend( url );
  }
  else {
    status = ConnectWiFi();
  }

  delay( 120 * 1000);
  
}
