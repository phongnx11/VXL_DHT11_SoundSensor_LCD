#include <ESP8266WiFi.h>
#include <Wire.h>
#include <DHT.h>
 
#include <LiquidCrystal_I2C.h> // Khai bao cac thu vien ESP8266, DHT, LCD
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
 
const int sampleWindow = 50;                       // Khoang thoi gian lay mau        
unsigned int sample;
 
#define SENSOR_PIN A0
#define PIN_QUIET D3
#define PIN_MODERATE D4
#define PIN_LOUD D5
#define DHTPIN D7

DHT dht(DHTPIN, DHT11); // Khai bao loai cam bien nhiet do do am chon
 
String apiKey = "ZG36HY9NCYEGRWHG"; // Nhap API Key lay Thingspeak
const char *ssid = "Xuan Hanh";     
const char *pass = "66668888";
const char* server = "api.thingspeak.com";
 
WiFiClient client;
 
void setup ()  
{   
  pinMode (SENSOR_PIN, INPUT); // Dat chan cam bien am thanh va cam bien DHT11 la chan input 
  pinMode (DHTPIN, INPUT);
  pinMode(PIN_QUIET, OUTPUT);
  pinMode(PIN_MODERATE, OUTPUT);
  pinMode(PIN_LOUD, OUTPUT); 
 
  digitalWrite(PIN_QUIET, LOW);
  digitalWrite(PIN_MODERATE, LOW);
  digitalWrite(PIN_LOUD, LOW);
  
  Serial.begin(115200);
  dht.begin();

  lcd.init();
 
  // Bat den nen
  lcd.backlight();
 
  Serial.println("Connecting to ");
  Serial.println(ssid);
  
  lcd.setCursor(0, 0);
  lcd.print("Connecting to...");
 
  lcd.setCursor(0, 1);
  lcd.print(ssid);
 
  WiFi.begin(ssid, pass);
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.println("WiFi connected");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connected...");
    delay(4000);
    lcd.clear();
}  
 
   
void loop ()  
{ 
   unsigned long startMillis= millis();                   // Lay thoi gian bat dau do 
   float peakToPeak = 0;                                  // peak-to-peak level
   float h = dht.readHumidity();
   float t = dht.readTemperature();
   unsigned int signalMax = 0;                            //Gia tri lon nhat
   unsigned int signalMin = 1024;                         //Gia tri nho nhat
 
                                                          // Thu thap du lieu trong 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(SENSOR_PIN);                    //doc tin hien analog tu cam bien am thanh
      if (sample < 1024)                                  // gia tri cua tin hieu analog chi trong khoang nho hon 1024
      {
         if (sample > signalMax)
         {
            signalMax = sample;                           // Luu muc level cao nhat
         }
         else if (sample < signalMin)
         {
            signalMin = sample;                           // Luu muc level thap nhat
         }
      }
   }

   if (isnan(h)||isnan(t)){
      Serial.println("Failed to read from DHT sensor!"); // Neu khong doc duoc mot trong hai thong so thi quay lai
      return;
    }

  lcd.setCursor(0,0);
  lcd.print("Humi:");
  lcd.setCursor(5,0);
  lcd.print(h);
  lcd.setCursor(11,0);
  lcd.print("%");

  lcd.setCursor(0,1);
  lcd.print("Temp:");
  lcd.setCursor(5,1);
  lcd.print(t);
  lcd.setCursor(11,1);
  lcd.print("C");

  delay(3000);

  lcd.clear();
    
 
   peakToPeak = signalMax - signalMin;                    // Lay khoang cach giua hai muc cao nhat va thap nhat cua tin hieu
   int db = map(peakToPeak,20,900,49.5,90);             // quy chuan sang don vi dB
 
  lcd.setCursor(0, 0);
  lcd.print("Loudness: ");
  lcd.print(db);
  lcd.print("dB");
  
  if (db <= 60)
  {
    lcd.setCursor(0, 1);
    lcd.print("Level: Quite");
    digitalWrite(PIN_QUIET, HIGH);
    digitalWrite(PIN_MODERATE, LOW);
    digitalWrite(PIN_LOUD, LOW);
  }
  else if (db > 60 && db<85)
  {
    lcd.setCursor(0, 1);
    lcd.print("Level: Medium");
    digitalWrite(PIN_QUIET, LOW);
    digitalWrite(PIN_MODERATE, HIGH);
    digitalWrite(PIN_LOUD, LOW);
  }
  else if (db>=85)
  {
    lcd.setCursor(0, 1);
    lcd.print("Level: High");
    digitalWrite(PIN_QUIET, LOW);
    digitalWrite(PIN_MODERATE, LOW);
    digitalWrite(PIN_LOUD, HIGH);
 
  }
   
 
  if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com
  {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(db);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "&field3=";
    postStr += String(t); 
    postStr += "\r\n\r\n\r\n";
    
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
   
  }
    client.stop();
 
   delay(200);      // thingspeak cần độ trễ tối thiểu 15 giây giữa các lần cập nhật
   lcd.clear();
}
