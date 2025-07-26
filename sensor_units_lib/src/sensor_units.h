#ifndef __SENSOR
#define __SENSOR
#include <Arduino.h>
#include <string.h>
#include <DHT.h>
#include <time.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>

#define GPS_BAUD 9600
#define MAX_MSG_LENGTH 32
#define MAX_CMD_LENGTH 16

enum sensor_type {TEMP_AND_HUMID, GPS, TIME};

typedef struct _sensor_unit {
    enum sensor_type modules[3];
    char available_commands[6][16];
    DHT dht_sensor;
    HardwareSerial* gpsSerial;
    TinyGPSPlus gps;
    _sensor_unit(sensor_type sensors[3], uint8_t DHT_SETUP[2], uint8_t GPS_SETUP[2]) : dht_sensor(DHT_SETUP[0], DHT_SETUP[1]) {
        int i;
        int sensor_ind = 0;
        for (i = 0; i < 3; i++) {
            if (modules[i] == NULL) {
                modules[i] = sensors[sensor_ind++];
            }
        }

    }
} sensor_unit;

typedef struct def_message_struct {
    char message[MAX_MSG_LENGTH];
    int urgency;
    float value;
    unsigned int MSG_ID;
} def_message_struct;

typedef struct communication_unit {
    char* SSID;
    char* pswd;

} communication_unit;

int return_available_commands(char** array, int len, enum sensor_type sensor);
int handleRequest(char* cmd_passed, def_message_struct *response, sensor_unit CU);

#endif