#ifndef __SENSOR
#define __SENSOR
#include <Arduino.h>
#include <string.h>
#include <DHT.h>
#include <time.h>

#define MAX_MSG_LENGTH 32
#define MAX_CMD_LENGTH 16
enum sensor_type {TEMP_AND_HUMID, GPS, TIME};
typedef struct _sensor_unit {
    enum sensor_type modules[3];
    char available_commands[6][16];
    DHT dht_sensor;
    _sensor_unit(uint8_t pin, uint8_t type) : dht_sensor(0, 0) {
        int i;
        for (i = 0; i < 3; i++) {
            if (modules[i] == NULL) {
                modules[i] = TEMP_AND_HUMID;
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