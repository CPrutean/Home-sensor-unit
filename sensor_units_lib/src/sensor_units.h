#ifndef __SENSOR
#define __SENSOR
#include <Arduino.h>


enum sensor_type {TEMP_AND_HUMID, GPS, TIME};
typedef struct _sensor {
    enum sensor_type *modules;
    char available_commands[][16];
} sensor;

typedef struct def_message_struct {
    char message[32];
    int urgency;
    float value;
} def_message_struct;

void return_available_commands(char** array);

#endif