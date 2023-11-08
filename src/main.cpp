

#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "123";
const char* password = "kikikiki";
// const char* ssid = "Wokwi-GUEST";
// const char* password = "";

const char* scriptUrl = "https://script.google.com/macros/s/AKfycbymgyVVDZHEu0dTTVvOxvIEu8aKQ6a7vCY7DgIcenpLcuhRjOdtjGxNJXJvPrli4kKS/exec";

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

void loop() {
  HTTPClient http;

  http.begin(scriptUrl);
  http.addHeader("Content-Type", "application/json");

  // String json = "{\"temperature\": 25.5, \"humidity\": 60.0}"; // Ganti dengan data sensor sesuai kebutuhan.
  String json = "{\"nomer\": 1, \"pressure\": 1, \"water_flow\": 950, \"battery\": 4.2}"; 

  int httpResponseCode = http.POST(json);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  delay(5000); // Kirim data setiap 5 detik (sesuaikan dengan kebutuhan Anda).
}
