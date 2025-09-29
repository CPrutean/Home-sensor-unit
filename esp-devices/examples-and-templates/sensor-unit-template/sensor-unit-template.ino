#include <sensor_units.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

//For safety reasons dont expose your ESP32 mac addresses to the internet so create a 
//Folder titled 'configuration' and include a header file
//With the mac addresses for each device
//In this case im going to assume that the communication unit's mac address is
//named 'communication_unit_addr' and is an array of the appropriate type
#include <configuration.h>

sensor_unit SU;
msg_queue q;
//Define the sensor unit and communication unit pointers used in the libraries for functionality always define these
//These are required for the code to work properly
sensor_unit *sens_unit_ptr;
communication_unit *com_unit_ptr;

//Include the modules
//sensor_type is enum representing what sensors are available in your sensor units
//The types go as follows
//TEMP_AND_HUMID, GPS, MOTION_SENSOR

sensor_type modules[] = {};


//Define the proper DHT sensor type and the pin the DHT sensor is attached to

//#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)
//#define DHTPIN 0
//DHT dht(DHTPIN, DHTTYPE);

//GPS

//#define RXPIN 0
//#define TXPIN 0
//HardwareSerial gpsSerial(2); //Define communication VIA UART 2
//TinyGPSPlus gps;

//Motion sensor

//#define MOTION_PIN 0
//PIR motion;

const char* name = "Insert name here";

//Default ESPNOW communication callbacks for proper functionality


void onDataSent(const uint8_t* addr, esp_now_send_status_t status) {
  #ifdef DEBUG
  Serial.println("Packet sent");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery success":"Delivery failed");
  #endif
}

void onDataRecv(const uint8_t* adr, const uint8_t* data, int len) {
  #ifdef DEBUG
  Serial.println("Message recieved");
  #endif
  def_message_struct msg;
  memcpy(&msg, data, sizeof(msg));
  if (SU.queue->send(msg)) {
    #ifdef DEBUG
    Serial.println("Message succesfully sent to queue");
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("The message failed to be added to queue");
    #endif
  }
}



//Default message queue 
void queueHandlerTask(void* pvParamaters) {
  def_message_struct msg;
  for (;;) {
    if (SU.queue->receive(msg)) {
      def_message_struct response;
      handleRequestSU(msg, &response);
    }
  }
}


esp_now_peer_info_t peerInfo;
void setup() {
  //dht.begin();
  
  //Add the pin to the monitoring pins for 
  //motion.add(MOTION_PIN);

  //gpsSerial.begin(9600, SERIAL_8N1, RXPIN, TXPIN);

  SU.modules = modules;
  SU.modulesCount = sizeof(modules)/sizeof(modules[0]);
  SU.queue = &q;
  //SU.dht_sensor = &dht;
  //SU.gpsSerial = &gpsSerial;
  //SU.motion = &motion;

  SU.name[0] = '\0';
  def_message_struct temp;
  //Checks if a name has already been stored within EEPROM memory
  //If not initialize a new name
  if (!initSU(&SU1)) {
    snprintf(SU1.name, MAX_NAME_LEN, "%s", name);    
  }

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    #ifdef DEBUG
    Serial.println("Failed to init exiting");
    #endif
    exit(-1);
  }
  memcpy(peerInfo.peer_addr, communication_unit_addr, 6);
  memcpy(SU1.CU_ADDR, communication_unit_addr, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    #ifdef DEBUG
    Serial.println("Failed to add peer exiting");
    #endif
    exit(-1);
  }
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));

  xTaskCreatePinnedToCore(queueHandlerTask, "Queue handler task", 8192, NULL, 2, NULL, 1);
}

void loop() {
  // put your main code here, to run repeatedly:

}
