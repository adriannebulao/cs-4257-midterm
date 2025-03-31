#define BLYNK_PRINT Serial
#include "blynk_secrets.h"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHT_PIN 4    
#define WATER_PIN 32   
#define DHT_TYPE DHT11 

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int display_mode = 1; 

BLYNK_WRITE(V0) {  
  display_mode = param.asInt();
  Serial.print("Display mode changed to: ");
  Serial.println(display_mode);
}

void setup() {
  Serial.begin(921600);
  dht.begin();
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Connect to WiFi & Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

void loop() {
  Blynk.run(); 

  float temp_c = dht.readTemperature();    
  float temp_f = dht.readTemperature(true);  
  float humidity = dht.readHumidity();       
  int water_level = analogRead(WATER_PIN);  

  Blynk.virtualWrite(V1, temp_c);
  Blynk.virtualWrite(V2, temp_f);
  Blynk.virtualWrite(V3, humidity);
  Blynk.virtualWrite(V4, water_level);

  Serial.print("Temp: "); Serial.print(temp_c);
  Serial.print(" C, "); Serial.print(temp_f);
  Serial.print(" F, Hum: "); Serial.print(humidity);
  Serial.print("%, Water: "); Serial.println(water_level);

  lcd.clear();
  lcd.setCursor(0, 0);

  if (display_mode == 1) {
    lcd.print("Temp: "); lcd.print(temp_c); lcd.print(" C");
  } else if (display_mode == 2) {
    lcd.print("Temp: "); lcd.print(temp_f); lcd.print(" F");
  } else if (display_mode == 3) {
    lcd.print("Humidity: "); lcd.print(humidity); lcd.print("%");
  } else if (display_mode == 4) {
    lcd.print("Water Level: ");
    lcd.setCursor(0, 1);
    lcd.print(water_level);
  }

  delay(2000);
}
