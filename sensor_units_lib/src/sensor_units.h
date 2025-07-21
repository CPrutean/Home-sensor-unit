#ifndef __SENSOR
#define __SENSOR
#include <Arduino.h>
#include <string.h>
#include <DHT.h>

#define MAX_MSG_LENGTH 32
enum sensor_type {TEMP_AND_HUMID, GPS, TIME};
typedef struct _sensor {
    enum sensor_type *modules;
    char available_commands[][16];
} sensor;

typedef struct def_message_struct {
    char message[MAX_MSG_LENGTH];
    int urgency;
    float value;
} def_message_struct;

int return_available_commands(char** array, int len, enum sensor_type sensor);
int handleRequest(enum sensor_type sensor, char* cmd_passed, def_message_struct *response);


#endif