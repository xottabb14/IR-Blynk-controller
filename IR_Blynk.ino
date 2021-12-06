#include "config.h"
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>


//ir-settings
const uint16_t kCaptureBufferSize = 1024;
const uint8_t kTimeout = 50;
const uint16_t kFrequency = 38000;

//пин приема
const uint16_t kRecvPin = 14;

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, false);

//пин отправки сигнала
const uint16_t kIrLed = 4;
IRsend irsend(kIrLed);

decode_results results;


uint16_t *raw_save = {};
uint16_t size = 0;
String RawStr = "";

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

BLYNK_WRITE(V1) //функция, отслеживающая изменение виртуального пина 0
{
  String X_requires = param.asStr(); //переменная текущего состояния виртуального пина
  size = (uint16_t)(getValue(X_requires,'-',0).toInt());
  String Massiv = getValue(X_requires,'-',1);
  for(int i=0;i<=size;i++){
    raw_save[i]=strtoul((getValue(Massiv,',',i).c_str()), NULL, 0);
  }
  irsend.sendRaw(raw_save, size, kFrequency);
}

void setup() //основная функция, выполняется один раз при подаче питания на микроконтроллер
{
  Serial.begin(9600); //открываем серийный порт, чтобы видеть как проходит подключение к серверу blynk
  irrecv.enableIRIn();
  irsend.begin();
  Blynk.begin(auth, ssid, pass); //авторизируемся на сервере
}

void ReadIR() {
  if (irrecv.decode(&results)) {
    Blynk.virtualWrite(V0, "");
    if (getCorrectedRawLength(&results) >= 30) {
      raw_save = resultToRawArray(&results);
      size = getCorrectedRawLength(&results);
      RawStr=String(size)+"-";
      for(int i=0;i<size;i++){
        RawStr=RawStr+String(raw_save[i])+",";
      }
      RawStr=RawStr.substring(0,RawStr.length()-1);
    }
    irrecv.resume();
  }
}

void loop()
{
  ReadIR(); //считываем данные с инфракрасника
  Blynk.virtualWrite(V0, RawStr);
  RawStr = "";
  Blynk.run();
}
