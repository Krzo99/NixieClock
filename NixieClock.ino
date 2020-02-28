#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "NixieLib.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#define EnableNix D6
#define Buzzer D1
#define Button D0

#define STASSID "DG57A-ext"
#define STAPSK  "omrezje57a"

#define TimeToCheckOnline 600 //In min
#define RetryConnectionTime 5000 //In ms


Clock Clock(EnableNix, Buzzer, Button);

//WifiStuff
unsigned int LastOnlineCheck = 0;
bool bFirstCheck = true;
bool bServerRunning = false;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  //Pins
  Clock.Init();
  
  WiFi.mode(WIFI_STA);
  delay(1000);

}

void handleRoot()
{
  server.send_P(200, "text/html", "200");
}
void handleSetAlarm()
{
  if (server.hasArg("time"))
  {
    String GotData = server.arg("time");
    if (GotData.length() == 5)
    {
      int H = GotData.substring(0,2).toInt();
      int M = GotData.substring(3).toInt();   

      Clock.SetAlarm(H, M);
      
      server.send(200, "text/html",  "Success");
      return;
    }
  }
  
  server.send(200, "text/html",  "Failed");
}
void handleNotFound()
{
  server.send_P(200, "text/html", "404");
}

void handleResetAlarm()
{
  Clock.ResetAlarm();
  server.send_P(200, "text/html", "Success");
}

void handleCheckAlarm()
{
  Clock.BlinkAlarm();
  server.send(200, "text/html", "Check Clock");
}

void loop() {

  //Wifi stuff
  if (WiFi.status() != WL_CONNECTED)
  {
    ConnectToWifi();
    bServerRunning = false;
  }
  else if (!bServerRunning)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      server.on("/", handleRoot);
      server.on("/setAlarm", handleSetAlarm);
      server.on("/resetAlarm", handleResetAlarm);
      server.on("/checkAlarm", handleCheckAlarm);
      server.onNotFound(handleNotFound);
      server.begin();
      Serial.println("");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.print("MAC address: ");
      Serial.println(WiFi.macAddress());
      if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
        Serial.println("mDNS responder started");
      } else {
        Serial.println("Error setting up MDNS responder!");
      }
      bServerRunning = true;
    }
  }
  else if (millis() - LastOnlineCheck > TimeToCheckOnline * 1000 * 60 || bFirstCheck)
  {
    CheckTimeOnline();
  }

  Clock.Loop();
  server.handleClient();
}

void ConnectToWifi()
{
  static int LastAtempt = 0;
  if (millis() - LastAtempt >= RetryConnectionTime)
  {
    LastAtempt = millis();
    WiFi.begin(STASSID, STAPSK);
  }
}

void CheckTimeOnline()
{
  WiFiClient client;
  HTTPClient http;

  if (http.begin(client, "http://worldtimeapi.org/api/ip")) {  // HTTP
    int httpCode = http.GET();

    if (httpCode > 0 && httpCode == 200) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();

        unsigned int sizeOfpayload = 2048;
        char Jsonbuff[sizeOfpayload];
        http.getString().toCharArray(Jsonbuff, sizeOfpayload);

        DynamicJsonDocument JsonDoc(sizeOfpayload);
        DeserializationError er = deserializeJson(JsonDoc, Jsonbuff);
        int tajm = JsonDoc["unixtime"];

        Clock.SetTime(tajm);
        LastOnlineCheck = millis();
        bFirstCheck = false;
      }
    }
    http.end();
  }
}
