#ifndef __SENSOR
#define __SENSOR
#include <Arduino.h>
#include <string.h>
#include <DHT.h>
#define MAX_MSG_LENGTH 32
#define MAX_CMD_LENGTH 16
enum sensor_type {TEMP_AND_HUMID, GPS};
typedef struct _sensor {
    enum sensor_type *modules;
    char available_commands[][16];
    DHT dht();
} sensor;

typedef struct def_message_struct {
    char message[MAX_MSG_LENGTH];
    int urgency;
    float value;
    unsigned int MSG_ID;
} def_message_struct;

int return_available_commands(char** array, int len, enum sensor_type sensor);
int handleRequest(enum sensor_type module, char* cmd_passed, def_message_struct *response, sensor CU);

#endif