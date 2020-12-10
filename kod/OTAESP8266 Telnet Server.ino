#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "xxx";
const char* password = "zzz";

uint8_t i;
bool ConnectionEstablished; // Flag for successfully handled connection

#define MAX_TELNET_CLIENTS 2

WiFiServer TelnetServer(23);
WiFiClient TelnetClient[MAX_TELNET_CLIENTS];

void TelnetMsg(String text)
{
  for (i = 0; i < MAX_TELNET_CLIENTS; i++)
  {
    if (TelnetClient[i] || TelnetClient[i].connected())
    {
      TelnetClient[i].println(text);
    }
  }
  delay(10);  // to avoid strange characters left in buffer
}

void Telnet()
{
  // Cleanup disconnected session
  for (i = 0; i < MAX_TELNET_CLIENTS; i++)
  {
    if (TelnetClient[i] && !TelnetClient[i].connected())
    {
      Serial.print("Client disconnected ... terminate session "); Serial.println(i + 1);
      TelnetClient[i].stop();
    }
  }

  // Check new client connections
  if (TelnetServer.hasClient())
  {
    ConnectionEstablished = false; // Set to false

    for (i = 0; i < MAX_TELNET_CLIENTS; i++)
    {
      // Serial.print("Checking telnet session "); Serial.println(i+1);

      // find free socket
      if (!TelnetClient[i])
      {
        TelnetClient[i] = TelnetServer.available();

        Serial.print("New Telnet client connected to session "); Serial.println(i + 1);

        TelnetClient[i].flush();  // clear input buffer, else you get strange characters
        TelnetClient[i].println("Welcome!");

        TelnetClient[i].print("Millis since start: ");
        TelnetClient[i].println(millis());

        TelnetClient[i].print("Free Heap RAM: ");
        TelnetClient[i].println(ESP.getFreeHeap());

        TelnetClient[i].println("----------------------------------------------------------------");

        ConnectionEstablished = true;

        break;
      }
      else
      {
        // Serial.println("Session is in use");
      }
    }

    if (ConnectionEstablished == false)
    {
      Serial.println("No free sessions ... drop connection");
      TelnetServer.available().stop();
      // TelnetMsg("An other user cannot connect ... MAX_TELNET_CLIENTS limit is reached!");
    }
  }

  for (i = 0; i < MAX_TELNET_CLIENTS; i++)
  {
    if (TelnetClient[i] && TelnetClient[i].connected())
    {
      if (TelnetClient[i].available())
      {
        //get data from the telnet client
        while (TelnetClient[i].available())
        {
          Serial.println(TelnetClient[i].read());
        }
      }
    }
  }
}

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

  TelnetServer.begin();
  TelnetServer.setNoDelay(true);
}

void loop() {
  ArduinoOTA.handle();
  Telnet();
  Serial.println("Ready");
  TelnetMsg("Ready");
  Serial.print("IP address: ");
  TelnetMsg("IP Adress");
  String IPadress = WiFi.localIP().toString();
  Serial.println(WiFi.localIP());
  TelnetMsg(IPadress);
  Serial.println("Version 10002");
  TelnetMsg("Version 10002");
  delay(3000);
}
