#include "esp_rainmaker_secrets.h"
#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

static uint8_t dht_pin = 4; 
static uint8_t water_pin = 32;    

DHT dht(dht_pin, DHT11);

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize RainMaker devices
static TemperatureSensor temp_c("Temperature (C)");
static TemperatureSensor temp_f("Temperature (F)");
static TemperatureSensor humidity("Humidity");
static TemperatureSensor water_level("Water Level");
static Device display_changer("Display Changer", "custom.device.display_changer");

int display_mode = 1;

void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx) {
    const char *device_name = device->getDeviceName();
    const char *param_name = param->getParamName();
    if (strcmp(param_name, "Display Mode") == 0) {
        display_mode = val.val.i;
        Serial.printf("Display Mode updated to %d\n", display_mode);
        param->updateAndReport(val);
    }
}

void sysProvEvent(arduino_event_t *sys_event)
{
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_PROV_START:
#if CONFIG_IDF_TARGET_ESP32
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", SERVICE_NAME, POP);
      printQR(SERVICE_NAME, POP, "ble");
#else
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on SoftAP\n", SERVICE_NAME, POP);
      printQR(SERVICE_NAME, POP, "softap");
#endif
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.printf("\nConnected to Wi-Fi!\n");
      delay(500);
      break;
    case ARDUINO_EVENT_PROV_CRED_RECV: {
        Serial.println("\nReceived Wi-Fi credentials");
        Serial.print("\tSSID : ");
        Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
        Serial.print("\tPassword : ");
        Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
        break;
      }
    case ARDUINO_EVENT_PROV_INIT:
      wifi_prov_mgr_disable_auto_stop(10000);
      break;
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      Serial.println("Stopping Provisioning!!!");
      wifi_prov_mgr_stop_provisioning();
      break;
  }
}

void setup() {
    Serial.begin(921600);

    pinMode(dht_pin, INPUT);
    dht.begin();
    
    lcd.init();
    lcd.backlight();
    
    Node my_node;
    my_node = RMaker.initNode("Panco");
    
    Param display_mode_param("Display Mode", "custom.param.display_mode", value(1), PROP_FLAG_READ | PROP_FLAG_WRITE);
    display_mode_param.addBounds(value(1), value(4), value(1));
    display_mode_param.addUIType(ESP_RMAKER_UI_SLIDER);
    display_changer.addParam(display_mode_param);
    
    display_changer.addCb(write_callback);
    
    temp_c.updateAndReportParam("Temperature", 0);
    temp_f.updateAndReportParam("Temperature", 0);
    humidity.updateAndReportParam("Temperature", 0);
    water_level.updateAndReportParam("Temperature", 0);

    my_node.addDevice(temp_c);
    my_node.addDevice(temp_f);
    my_node.addDevice(humidity);
    my_node.addDevice(water_level);
    my_node.addDevice(display_changer);
    
    RMaker.start();
    
    WiFi.onEvent(sysProvEvent);

    #if CONFIG_IDF_TARGET_ESP32
      WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, POP, SERVICE_NAME);
    #else
      WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, POP, SERVICE_NAME);
    #endif
}

void loop() {
    float temp_Celsius = dht.readTemperature();
    float temp_Fahrenheit = dht.readTemperature(true);
    float hum = dht.readHumidity();
    float water = analogRead(water_pin);
    
    Serial.print("Temp: ");
    Serial.print(temp_Celsius);
    Serial.print(" C / ");
    Serial.print(temp_Fahrenheit);
    Serial.print(" F, Hum: ");
    Serial.print(hum);
    Serial.print("%, Water: ");
    Serial.println(water);
    
    temp_c.updateAndReportParam("Temperature", temp_Celsius);
    temp_f.updateAndReportParam("Temperature", temp_Fahrenheit);
    humidity.updateAndReportParam("Temperature", hum);
    water_level.updateAndReportParam("Temperature", water);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    
    switch (display_mode) {
        case 1:
            lcd.print("Temp: ");
            lcd.print(temp_Celsius);
            lcd.print("C");
            break;
        case 2:
            lcd.print("Temp: ");
            lcd.print(temp_Fahrenheit);
            lcd.print("F");
            break;
        case 3:
            lcd.print("Hum: ");
            lcd.print(hum);
            lcd.print("%");
            break;
        case 4:
            lcd.print("Water: ");
            lcd.print(water);
            break;
    }
    
    delay(2000);
}