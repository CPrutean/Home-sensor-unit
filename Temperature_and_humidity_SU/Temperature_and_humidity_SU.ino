#include <DHT.h>
#include <esp_now.h>
#include <WiFi.h>
#include <string.h>

#define DHTTYPE DHT11
#define DHTPIN 4
DHT dht(DHTPIN, DHTTYPE);

typedef struct struct_message{
  char message[32];
  float value;
  u_int8_t urgencyLevel;
} struct_message;

//Board address here 
uint8_t broadcastAddress[] = {0x3C, 0x8A, 0x1F, 0xD3, 0xD6, 0xEC};
const char* module = "temp_and_humid_SU";
struct_message msg;
esp_now_peer_info_t peerInfo;

void setup() {
  dht.begin();
  Serial.begin(115200);
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

void parseMsg() {
  if (msg.message[0] == NULL && msg.value == NULL) {
    Serial.println("Message passed was null");
    exit(0);
  }

  if (strncmp(msg.message, "PULL TEMP", strlen("PULL TEMP")) == 0) {
    strncpy(msg.message, "TEMP", strlen("TEMP"));
    msg.value = requestTemp();
  } else if (strncmp(msg.message, "PULL HUMID", strlen("PULL HUMID"))) {
    strncpy(msg.message, "HUMID", strlen("HUMID"));
    msg.value = requestHumidity();
  } else if (strncmp(msg.message, "GET TYPE", strlen("GET TYPE"))) {
    strncpy(msg.message, "TYPE ", strlen("TYPE "));
    strncat(msg.message, moduleType, strlen(moduleType));
  }
  esp_err_t result = esp_now_send(broadcastAddress, (u_int8_t*) &msg, sizeof(msg));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Message failed to send");
  }
}

void loop() {

}

float requestTemp() {
  return dht.readTemperature();
}

float requestHumidity() {
  return dht.readHumidity();
}

void onDataSent(const u_int8_t *addr, esp_now_send_status_t status) {
  Serial.println("Packet sent");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery success":"Delivery failed");
}

void onDataRecv(const u_int8_t * adr, const u_int8_t * data, int len) {
  memcpy(&msg, incomingData, sizeof(msg));
  parseMsg();
}







