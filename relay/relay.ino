extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}
#include "VeDirectFrameHandler.h"
#include "config.h"
#include <AsyncMqttClient.h>
#include <WiFi.h>
#include <HTTPClient.h>

VeDirectFrameHandler myve;

#define rxPin 8
#define txPin 9

#define TEST_DATA "PID=0XA075,FW=175,SERN=HQ260994ACG,V=13790,I=20,VPV=28740,PPV=2,CS=5,MPPT=1,OR=0X00000000,ERR=0,LOAD=ON,IL=0,H19=41,H20=2,H21=15,H22=3,H23=8,HSDS=16"
#define MQTT_HOST "datto"
#define MQTT_PORT 1883
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;
// victronMQTT[] = {
//   "V":   "battery_voltage",
//   "I":   "battery_current",
//   "VPV": "panel_voltage",
//   "PPV": "panel_power",
//   "CS":  "battery_state",
//   "MPPT":"MPPT",
//   "OR":  "off_reason",
//   "ERR": "error",
//   "LOAD":"load",
//   "IL":  "load_current",
//   "H19":  "total_yield",
//   "H20":  "yield_today",
//   "H21":  "maximum_power_today",
//   "H22":  "yield_yesterday",
//   "H23":  "maximum_power_yesterday",
//   "HSDS": "day_sequence_number"
// };

HTTPClient http;

void setup() {
  Serial.begin(19200);
  delay(3000);
  Serial.println("serialTest: init");

  Serial1.begin( 19200, SERIAL_8N1, rxPin, txPin );
  Serial1.flush();

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  connectToWifi();

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
          String url = "http://" + String(LOG_HOST) + ":" + String(LOG_PORT) + "/putData.php?data=$V,";
          String victronData;
          for ( int i = 0; i < myve.veEnd; i++ ) {

            // don't send product ID, serial number, or firmware version
            if ( strcmp(myve.veName[i],"PID") && strcmp(myve.veName[i],"SER#") && strcmp(myve.veName[i],"FW") ) {

              String topic = "HQ260994A/";
              topic.concat( myve.veName[i] );
              uint16_t packetIdPub1 = mqttClient.publish( topic.c_str(), 1, true, myve.veValue[i] );
  
              victronData.concat( myve.veName[i] );
              victronData.concat( "=" );
              victronData.concat( myve.veValue[i] );
              if ( i < myve.veEnd - 1 ) {
                victronData.concat( "," );
              }
            }
          }

          victronData.replace("#","N"); // probably not needed now
          url.concat(victronData);
          Serial.printf("EveryMinute: url %s\n", url.c_str());
          HTTPSend( url );
          // #ifdef TEST_DATA
          //   uint16_t packetIdPub1 = mqttClient.publish( "victron", 1, true, victronData.c_str() );
          // #else
          //   uint16_t packetIdPub1 = mqttClient.publish( "victron", 1, true, TEST_DATA );
          // #endif
        prev_millis = millis();
    }
}

void connectToWifi() {
  // Serial.println("Connecting to Wi-Fi...");
  Serial.printf("Connecting to Wi-Fi...%s %s\n", wifiSSID , wifiPassword );
  WiFi.begin( wifiSSID , wifiPassword );
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case IP_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
    }
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

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}