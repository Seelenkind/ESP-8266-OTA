#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <timer.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ptbtime1.ptb.de", 3600);
Timer tiMer1;
Timer tiMer2;

int FW_VERSION = 10006;
const char *ssid = "Wifi Modem ismi", *password = "wifi sifresi";

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
          //Serial.write(TelnetClient[i].read());
          String TelnetText=TelnetClient[i].readStringUntil('\n');
          Serial.println(TelnetText);
        }
      }
    }
  }
}

void checkForUpdates() {
  String  fwVersionURL = "http://www.hilmi-soenmez.com/esp01/firmware.version";
  Serial.println( "Checking for firmware updates." );
  TelnetMsg( "Checking for firmware updates." );
  Serial.print( "Firmware version URL: " );
  TelnetMsg( "Firmware version URL: " );
  Serial.println( fwVersionURL );
  TelnetMsg( fwVersionURL );
  HTTPClient httpClient;
  httpClient.begin( fwVersionURL );
  int httpCode = httpClient.GET();
  if ( httpCode == 200 ) {
    String newFWVersion = httpClient.getString();
    Serial.print( "Current firmware version: " );
    TelnetMsg( "Current firmware version: " );
    Serial.println( FW_VERSION );
    TelnetMsg( String(FW_VERSION) );
    Serial.print( "Available firmware version: " );
    TelnetMsg( "Available firmware version: " );
    Serial.println( newFWVersion );
    TelnetMsg( newFWVersion );
    int newVersion = newFWVersion.toInt();
    if ( newVersion > FW_VERSION ) {
      Serial.println( "Preparing to update" );
      TelnetMsg( "Preparing to update" );
      String fwImageURL = "http://www.hilmi-soenmez.com/esp01/OTAESP8266_HTTP_get.ino.generic.bin";
      t_httpUpdate_return ret = ESPhttpUpdate.update( fwImageURL );
      switch (ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;
        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
        case HTTP_UPDATE_OK:
          Serial.println("HTTP_UPDATE_SUCCESFULL");
      }
    }
    else {
      Serial.println( "Already on latest version" );
      TelnetMsg( "Already on latest version" );
    }
  }
  else {
    Serial.print( "Firmware version check failed, got HTTP response code " );
    TelnetMsg( "Firmware version check failed, got HTTP response code " );
    Serial.println( httpCode );
  }
  httpClient.end();
}
void Telnet_ausgabe()
{
  TelnetMsg("Versiyon: " + String(FW_VERSION) );
  TelnetMsg("IP Nummer : " + WiFi.localIP().toString()) ;
}
void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(3000);
    ESP.restart();
  }
  TelnetServer.begin();
  TelnetServer.setNoDelay(true);

  tiMer1.setInterval(30000);
  tiMer1.setCallback(checkForUpdates);
  tiMer1.start();

  tiMer2.setInterval(15000);
  tiMer2.setCallback(Telnet_ausgabe);
  tiMer2.start();
}

void loop()
{
  Telnet();
  tiMer1.update();
  tiMer2.update();
}
