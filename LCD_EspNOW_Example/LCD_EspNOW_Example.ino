#include <LCD_I2C.h>
#include <esp_now.h>
#include <string.h>

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

struct_message msg;


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
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}


void onButtonPress() {
  strncpy(msg.message, "PULL TEMP", strlen(msg.message));
  esp_err_t result = esp_send_now(broadcastAddress, (u_int8_t*)&msg, sizeof(msg));
  if (result == ESP_OK) {
    Serial.println("Message was sent");
  } else {
    Serial.println("Message failed to send exiting");
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg.message);
  lcd.setCursor(strlen(msg.message), 0);
  lcd.print(msg.value);
  
  lcd.setCursor(0, 1);
  strncpy(msg.message, "PULL HUMID", strlen("PULL HUMID"));
  esp_err_t result = esp_send_now(broadcastAddress, (u_int8_t*)&msg, sizeof(msg));
  if (result == ESP_OK) {
    Serial.println("Message was sent");
  } else {
    Serial.println("Message failed to send exiting");
  }
  lcd.setCursor(strlen(msg.message), 1);
  lcd.print(msg.value);
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
  memcpy(&msg, incomingData, sizeof(msg));
}

