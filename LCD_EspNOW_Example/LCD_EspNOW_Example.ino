#include <LCD_I2C.h>
#include <esp_now.h>

#define buttonPin 21


LCD_I2C lcd(0x27, 16, 2);

typedef struct struct_message{
  char* message;
  float value;
  int urgencyLevel;
} struct_message;

const u_int8_t broadcastAddress[] = {};
void setup() {
  // put your setup code here, to run once:
  lcd.begin(4, 5);
  lcd.backlight();
  pinMode(buttonPin, INPUT);
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop() {
  // put your main code here, to run repeatedly:


}
