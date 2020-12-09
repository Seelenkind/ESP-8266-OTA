#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "WiFi Modem ismi";
const char* password = "WiFi Şifresi";

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(3000);
    ESP.restart();
  }
  ArduinoOTA.setHostname("ESP01_OTA");
  ArduinoOTA.setPassword("admin");
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Version 10001");
  delay(3000);
}
