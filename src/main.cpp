#include <Arduino.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include "HTTPSRedirect.h"

//Wi-Fi
#define WIFI_SSID "123"
#define WIFI_PASS "kikikiki"

// #define WIFI_SSID "Wokwi-GUEST"
// #define WIFI_PASS ""


//NTP
#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     0
#define UTC_OFFSET_DST 0

// Enter Google Script Deployment ID:
const char *GScriptId = "AKfycbwSBDsMClSsl_v87HMqavQa6zQZZuWTdSvevL_H4U2qDqh9SwCAThDV_MaHuskcRa91";

// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

// Google Sheets setup (do not edit)
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;


// Declare variables that will be published to Google Sheets
int value0 = 0;
int value1 = 0;
int value2 = 0;

struct tm timeinfo;

void setup() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  delay(2000);
  Serial.println(" Connected!");

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);

  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(host);

  bool flag = false;
  for (int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
       flag = true;
       Serial.println("Connected");
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    return;
  }
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
 
}

void wifi_check(){
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
}

void ntp_get_local_time(){
  if (!getLocalTime(&timeinfo)) {
    Serial.println("NTP Connection Error");
    return;
  }
  // Serial.println(&timeinfo, "Date: %d/%m/%Y");
  // Serial.println(&timeinfo, "Time: %H:%M:%S %Z");

}

void send_log_data(){

  value0 ++;
  value1 = random(0,10);
  value2 = random(0,10);

  static bool flag = false;

  if (!flag){
    flag = true;
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    Serial.println("Error creating client object!");
  }

  // Create json object string to send to Google Sheets
  payload = payload_base + "\"" + value0 + "," + value1 + "," + value2 + "\"}";
  
  
  // Publish data to Google Sheets
  Serial.println("Publishing data...");
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
  }
  else{
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
  }

}

void loop() {

  wifi_check();
  // ntp_get_local_time();


  send_log_data();
  delay(1000);
}
