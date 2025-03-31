#include "arduino_secrets.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include "thingProperties.h" 

#define DHT_PIN 4      
#define WATER_PIN 32   
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(921600);
  dht.begin();
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  display_mode = 1;

  // Initialize IoT Cloud properties
  initProperties();
  
  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  // Debugging
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update(); 

  temp_c = dht.readTemperature();     
  temp_f = dht.readTemperature(true);  
  humidity = dht.readHumidity();     

  water_level = analogRead(WATER_PIN);

  Serial.print("Temp: ");
  Serial.print(temp_c);
  Serial.print(" C, ");
  Serial.print(temp_f);
  Serial.print(" F, Hum: ");
  Serial.print(humidity);
  Serial.print("%, Water: ");
  Serial.println(water_level);

  lcd.clear();
  
  lcd.setCursor(0, 0);
  if (display_mode == 1) {
    lcd.print("Temp: ");
    lcd.print(temp_c);
    lcd.print(" C");
  } else if (display_mode == 2) {
    lcd.print("Temp: ");
    lcd.print(temp_f);
    lcd.print(" F");
  } else if (display_mode == 3) {
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.print("%");
  } else if (display_mode == 4) {
    lcd.print("Water Level:");
    lcd.setCursor(0, 1);
    lcd.print(water_level);
  }

  delay(2000);
}

void onDisplayModeChange() {
  Serial.println("Display mode changed!");
}
