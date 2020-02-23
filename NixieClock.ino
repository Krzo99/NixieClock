#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#define EnableNix D6
#define Buzzer D1

#define STASSID "DG57A-ext"
#define STAPSK  "omrezje57a"

#define TimeToCheckOnline 1000 //In min
#define RetryConnectionTime 5000 //In ms

struct ShiftRegister {
  int SER;
  int RCLK;
  int SRCLK;
};

ShiftRegister HourReg = {D4, D2, D3};
ShiftRegister MinReg = {D5, D8, D7};

byte NumberCodes[] = {B0000, B0001, B0010, B0011, B0100, B0101, B0110, B0111, B1000, B1001};

unsigned int LastOnlineCheck = 0;
unsigned int SecsAtNextMin = 1000;

int Minutes = 0;
int Hours = 0;



void setup() {
  Serial.begin(115200);

  //Pins
  pinMode(HourReg.SER, OUTPUT);
  pinMode(HourReg.RCLK, OUTPUT);
  pinMode(HourReg.SRCLK, OUTPUT);

  pinMode(MinReg.SER, OUTPUT);
  pinMode(MinReg.RCLK, OUTPUT);
  pinMode(MinReg.SRCLK, OUTPUT);

  pinMode(EnableNix, OUTPUT);
  pinMode(Buzzer, OUTPUT);

  SetHours(0);
  SetMin(0);

  delay(1000);

  WiFi.mode(WIFI_STA);
  SetEnable(HIGH);

}

void SetEnable(bool En)
{
  digitalWrite(EnableNix, En);
}

void WriteLL(int Number, ShiftRegister Reg)
{
  if ((Number <= 99 and Number >= 0))
  {
    int Enica = Number % 10;
    int Desetica = Number / 10;

    byte EnicaByte = NumberCodes[Enica];
    byte DeseticaByte = NumberCodes[Desetica];

    //Desetice
    for (int i = 0; i < 4; i++)
    {
      digitalWrite(Reg.SER, DeseticaByte & B1000);
      DeseticaByte <<= 1;

      digitalWrite(Reg.SRCLK, LOW);
      digitalWrite(Reg.SRCLK, HIGH);
    }


    //Enice
    for (int i = 0; i < 4; i++)
    {
      digitalWrite(Reg.SER, EnicaByte & B1000);
      EnicaByte <<= 1;

      digitalWrite(Reg.SRCLK, LOW);
      digitalWrite(Reg.SRCLK, HIGH);
    }

    PushToReg(Reg);
  }
}
void PushToReg(ShiftRegister Reg)
{
  digitalWrite(Reg.RCLK, LOW);
  digitalWrite(Reg.RCLK, HIGH);
}

void SetHours(int Hours)
{
  WriteLL(Hours, HourReg);
}
void SetMin(int Mins)
{
  WriteLL(Mins, MinReg);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED)
  {
    ConnectToWifi();
  }
  else if (millis() - LastOnlineCheck > TimeToCheckOnline)
  {
    CheckTimeOnline();
  }

  if (millis() / 1000 >= SecsAtNextMin)
  {
    SecsAtNextMin = millis() / 1000 + 60;

    Minutes == 59 ? Minutes = 0, Hours++ : Minutes++;
    SetHours(Hours);
    SetMin(Minutes);

  }
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

        SetTime(tajm);
      }
    }
    http.end();
  }
}

void SetTime(int unix)
{
  Minutes = unix / 60 % 60;
  Hours = unix / 60 / 60 % 24;

  SecsAtNextMin = millis() / 1000 + 60 - unix % 60;

  Hours != 23 ? Hours++ : Hours = 0;

  SetHours(Hours);
  SetMin(Minutes);

}
