#include <LCD_I2C.h>
#include <esp_now.h>
#include <string.h>
#include <WiFi.h>

#define bool char
#define true 1
#define false 0

#define buttonPin 21


LCD_I2C lcd(0x27, 16, 2);

typedef struct struct_message{
  char message[32];
  float value;
  u_int8_t urgencyLevel;
} struct_message;

const u_int8_t broadcastAddress[] = {0x3C, 0x8A, 0x1F, 0xD5, 0x44, 0xF8};
esp_now_peer_info_t peerInfo;

struct_message send;
struct_message recieve;


void setup() {
  // put your setup code here, to run once:
  lcd.begin(4, 5);
  lcd.backlight();
  pinMode(buttonPin, INPUT);
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing espNOW");
    exit(0);
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = true;

  if (esp_now_add_peer(&peerInfo)!= ESP_OK) {
    Serial.println("Failed to add peer");
    exit(0);
  }
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));
}


void onButtonPress() {
  strncpy(send.message, "PULL TEMP", strlen(send.message));
  esp_err_t result = esp_now_send(broadcastAddress, (u_int8_t*)&send, sizeof(send));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(recieve.message);
  lcd.setCursor(strlen(recieve.message), 0);
  lcd.print(recieve.value);

  lcd.setCursor(0, 1);
  strncpy(send.message, "PULL HUMID", strlen("PULL HUMID"));
  result = esp_now_send(broadcastAddress, (u_int8_t*)&send, sizeof(send));
  
  lcd.setCursor(strlen(recieve.message), 1);
  lcd.print(recieve.value);
}

int temp;
int button;
void loop() {
  button = digitalRead(buttonPin);
  // put your main code here, to run repeatedly:
  if (button == HIGH && temp == LOW) {
    onButtonPress();
  }
  temp = button;
}



void onDataSent(const u_int8_t *addr, esp_now_send_status_t status) {
  Serial.println("Packet sent");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery success":"Delivery failed");
}

void onDataRecv(const u_int8_t * adr, const u_int8_t * data, int len) {
  memcpy(&recieve, data, sizeof(recieve));
}

