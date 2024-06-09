//LORA SENDER
//#define E32_TTL_1W // E32 30d modülleri için bunu aktif et
#include "LoRa_E32.h"
#include "DHT.h" 
#include <SoftwareSerial.h>

#define DHTPIN 2
#define DHTTYPE DHT22

#define TRIG_PIN 8  
#define ECHO_PIN 9

const int AOUTpin = A3;
const int DOUTpin = 7;

const int sensorPin = A1; 

DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial mySerial(3, 4); //PCB versiyon 4.3 den sonra bu şekilde olmalı
LoRa_E32 FixajSS(&mySerial);
 
#define M0 7
#define M1 6
 
//PARAMETRE AYARLARI
#define Adres 1   //0--65000 arası bir değer girebilirsiniz. Diğer Modüllerden FARKLI olmalı
#define Kanal 20  //Frekans değeri Diğer Modüllerle AYNI olmalı.
//E32 için 0--31 arasında bir sayı giriniz.
//E32 433 için Frekans = 410 + kanal değeri
 
#define GonderilecekAdres 2 //Mesajın gönderileceği LoRa nın adresi
 
struct Signal {
  byte sicaklik[4];
  byte nem[4];
  byte co[4];
  byte yagmur[4];
  byte mesafe[4];
} data;
 
 
void setup() {
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.begin(9600);
  FixajSS.begin();
  dht.begin();
 
  LoraE32Ayarlar();
 
  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);
 
  delay(500);
  Serial.println("LoRa Sistemi Hazır");
}
 
void loop() {
  float co = analogRead(AOUTpin);
  float sicaklik = dht.readTemperature();
  float nem = dht.readHumidity();
  float yagmur = analogRead(sensorPin);
  float mesafe = measureDistance();
  
  memcpy(data.sicaklik, &sicaklik, sizeof(sicaklik));
  memcpy(data.nem, &nem, sizeof(nem));
  memcpy(data.co, &co, sizeof(co));
  memcpy(data.yagmur, &yagmur, sizeof(yagmur));
  memcpy(data.mesafe, &mesafe, sizeof(mesafe));

  ResponseStatus rs = FixajSS.sendFixedMessage(highByte(GonderilecekAdres), lowByte(GonderilecekAdres), Kanal, &data, sizeof(Signal));
  delay(2000);
}
 
float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2; // Mesafeyi santimetre cinsinden hesapla
  return distance;
}

void LoraE32Ayarlar() {
  digitalWrite(M0, HIGH);
  digitalWrite(M1, HIGH);
 
  ResponseStructContainer c;
  c = FixajSS.getConfiguration();
  Configuration configuration = *(Configuration*)c.data;
 
  //DEĞİŞEBİLEN AYARLAR
  // Üstte #define kısmında ayarlayınız
  configuration.ADDL = lowByte(Adres);
  configuration.ADDH = highByte(Adres);
  configuration.CHAN = Kanal;
 
  //SEÇENEKLİ AYARLAR
  configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;  //Veri Gönderim Hızı 2,4 varsayılan
  //configuration.SPED.airDataRate = AIR_DATA_RATE_000_03;  //Veri Gönderim Hızı 0,3 En uzak Mesafe
  //configuration.SPED.airDataRate = AIR_DATA_RATE_101_192; //Veri Gönderim Hızı 19,2 En Hızlı
 
 
  configuration.OPTION.transmissionPower = POWER_20;  //Geönderim Gücü max Varsayılan
  //configuration.OPTION.transmissionPower = POWER_10;  //Geönderim Gücü min
  //configuration.OPTION.transmissionPower = POWER_30; // E32 30d modülleri için bunu aktif et
 
  //GELİŞMİŞ AYARLAR
  configuration.SPED.uartBaudRate = UART_BPS_9600;
  configuration.SPED.uartParity = MODE_00_8N1;
  configuration.OPTION.fec = FEC_0_OFF;
  //configuration.OPTION.fec = FEC_1_ON;
  configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
  //configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
  configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
  configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
 
  // Ayarları KAYDET ve SAKLA
  ResponseStatus rs = FixajSS.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  c.close();
}