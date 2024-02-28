#include <Wire.h> 
#include "MAX30105.h" 
#include "heartRate.h" 
#include <inttypes.h> 
#include <lm75.h> 
#include <Adafruit_MPU6050.h> 
#include <Adafruit_Sensor.h> 
#include <ESP8266WiFi.h> 
#include <FirebaseArduino.h> 
/* Kbalosuz ag Bilgileri */ 
#define WLAN_SSID  "pluton" 
#define WLAN_PASSWORD "123456789" 
/***Fireebase Kurulum***/ 
#define FIREBASE_HOST   "https://akillibileklik-9260a-default-rtdb.firebaseio.com/" //url 
#define FIREBASE_AUTH   "AIzaSyCoyAvfa5Z59urvK8Agi7MB1pxUXF2xj8o" //firebase t�retilen key 
Adafruit_MPU6050 mpu; 
MAX30105 particleSensor; 
TempI2C_LM75 termo = TempI2C_LM75(0x48,TempI2C_LM75::nine_bits); 
int temperatureAddress = 0x48; //Sicaklik 
int heartrateAddress = 0x57; //Nabiz 
int accelerometerAddress = 0x68; //Ivme 
const byte RATE_SIZE = 4; //Daha fazla ortalama almak i�in bunu art�r�n. 4 iyidir. 
byte rates[RATE_SIZE]; //Nab�z dizisi 
byte rateSpot = 0; 
long lastBeat = 0; //Son vuru�un ger�ekle�ti�i saat 
float beatsPerMinute; 
int beatAvg; 
int durum = 0; 
void setup() 
{  
Serial.begin(115200); 
Serial.println("Start"); 
Serial.print("Actual temp "); 
Serial.print(termo.getTemp()); 
Serial.println(" oC"); 
delay(2000); 
Wire.beginTransmission(heartrateAddress); 
/////////////////NABIZ/////////////////// 
if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Default olarak I2C port kullanmaktad�r. 
{ 
Serial.println("MAX30105 bulunamadi. Lutfen gucu kontrol ediniz."); 
while (1); 
} 
Serial.println("Isaret parmaginizi sabit bir bas�ncla sensore yerlestirin."); 
particleSensor.setup(); //Sens�r� varsay�lan ayarlarla yap�land�r�n 
particleSensor.setPulseAmplitudeRed(heartrateAddress); //Sens�r�n �al��t���n� belirtmek i�in K�rm�z� LED'i 
d���k konuma getirin 
particleSensor.setPulseAmplitudeGreen(0); //Ye�il LED'i kapat�n 
/////////////////�VME////////////////////// 
while (!Serial) 
delay(10);  
Serial.println("Adafruit MPU6050 test ediliyor!"); 
// Ba�latmay� deneyin! 
if (!mpu.begin()) { 
Serial.println("MPU6050 bulunamad�"); 
while (1) { 
delay(10); 
} 
} 
Serial.println("MPU6050 Bulundu!"); 
mpu.setAccelerometerRange(MPU6050_RANGE_8_G); 
Serial.print("Ivmeolcer aral�g� su sekilde ayarlandi: "); 
switch (mpu.getAccelerometerRange()) { 
  case MPU6050_RANGE_2_G: 
    Serial.println("+-2G"); 
    break; 
  case MPU6050_RANGE_4_G: 
    Serial.println("+-4G"); 
    break; 
  case MPU6050_RANGE_8_G: 
    Serial.println("+-8G"); 
    break; 
  case MPU6050_RANGE_16_G: 
    Serial.println("+-16G"); 
    break; 
  } 
  mpu.setGyroRange(MPU6050_RANGE_500_DEG); 
  Serial.print("Gyro aral�g� suna ayarlandi:"); 
  switch (mpu.getGyroRange()) { 
  case MPU6050_RANGE_250_DEG: 
    Serial.println("+- 250 deg/s"); 
    break; 
  case MPU6050_RANGE_500_DEG: 
    Serial.println("+- 500 deg/s"); 
    break; 
  case MPU6050_RANGE_1000_DEG: 
    Serial.println("+- 1000 deg/s"); 
    break; 
  case MPU6050_RANGE_2000_DEG: 
    Serial.println("+- 2000 deg/s"); 
    break; 
  } 
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ); 
  Serial.print("Filtre bant genisligi su sekilde ayarlandi:"); 
  switch (mpu.getFilterBandwidth()) { 
  case MPU6050_BAND_260_HZ: 
    Serial.println("260 Hz"); 
    break; 
  case MPU6050_BAND_184_HZ: 
    Serial.println("184 Hz"); 
    break; 
  case MPU6050_BAND_94_HZ: 
    Serial.println("94 Hz"); 
    break; 
  case MPU6050_BAND_44_HZ: 
    Serial.println("44 Hz"); 
    break; 
  case MPU6050_BAND_21_HZ: 
    Serial.println("21 Hz"); 
    break; 
  case MPU6050_BAND_10_HZ: 
    Serial.println("10 Hz"); 
    break; 
  case MPU6050_BAND_5_HZ: 
    Serial.println("5 Hz"); 
    break; 
  } 
  Serial.println(""); 
  delay(100); 
  //connect to wifi. 
WiFi.begin("pluton","123456789"); 
Serial.print("connecting"); 
Serial.println(); 
Serial.print("connected:"); 
Serial.println(WiFi.localIP()); 
Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH); 
Firebase.setString("durum","0"); 
} 
void loop() 
{ 
delay(1000); 
/* 
 * int temperatureAddress = 0x48; //Sicaklik 
* int heartrateAddress = 0x57; //Nabiz 
* int accelerometerAddress = 0x68; //Ivme 
*/ 
sensors_event_t a, g, temp; 
mpu.getEvent(&a, &g, &temp); 
long irValue = particleSensor.getIR(); 
if (irValue < 50000) 
{ //olu durum 
Serial.println("Nab�z yok..."); 
Firebase.getString("durum"); 
durum = 1; 
} 
else if(a.acceleration.y < -5) 
{  
} 
Serial.println(" !!!!! Bay�ld����"); 
Firebase.setString("durum","1"); 
durum = 2; 
else if(termo.getTemp()> 35) 
{ 
} 
Serial.println("Ani Sicaklik  Artisi G�r�ld�"); 
durum = 3; 
else 
{ 
} 
} 
irValue = irValue / 1453; 
Serial.print("Nabiz="); 
Serial.println(irValue); 
Serial.print("Sicaklik="); 
Serial.println(termo.getTemp()); 
Serial.println("Kisinin degerleri normaldir");