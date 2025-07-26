#include "sensor_units.h"

const char* temp_sensor_cmds[2] = {"PULL TEMP", "PULL HUMID"};
const char* gps_sensor_cmds[1] = {"PULL LOCATION"};
const char* time_sensor_cmds[1] = {"PULL TIME"};

extern HardwareSerial gpsSerial;
//Pass in an array with unlimited rows and the columns being 16 long
int return_available_commands(char** array, int len, enum sensor_type sensor) {
    if (len != MAX_CMD_LENGTH) {
        return -1;
    }
    int i;
    int len;
    if (sensor == TEMP_AND_HUMID) {
        len = sizeof(temp_sensor_cmds)/sizeof(temp_sensor_cmds[0]);
        for (i = 0; i < len; i++) {
            strncpy(*(array+i), temp_sensor_cmds[i], MAX_CMD_LENGTH);
        }
    } else if (sensor == GPS) {
        len = sizeof(gps_sensor_cmds)/sizeof(gps_sensor_cmds[0]);
        for (i = 0; i < len; i++) {
            strncpy(*(array+i), gps_sensor_cmds[i], MAX_CMD_LENGTH);
        }
    } else {
        return -1;
    }
    return 0;
}

int handleRequest(enum sensor_type module, char* cmd_passed, def_message_struct *response, sensor_unit SU) {
    memset(response, 0, sizeof(response));
    if (module == TEMP_AND_HUMID) {
        handleTempRequests(cmd_passed, response, SU);
    } else if (module = GPS) {

    } else if (module = TIME) {
        
    } else {
        return -1;
    }
    return 0;
}

int handleTempRequests(char* cmd_passed, def_message_struct *response, sensor_unit SU) {
    if (strncmp(cmd_passed, temp_sensor_cmds[0], 16) == 0) {
        strncpy(response->message, "TEMP", 32);
        response->value = SU.dht_sensor.readTemperature();
    } else if (strncmp(cmd_passed, temp_sensor_cmds[1], MAX_CMD_LENGTH)) {
        strncpy(response->message, "HUMID", 32);
        response->value = SU.dht_sensor.readHumidity();
    } else {
        return -1;
    }
    return 0;
}

int handleGpsRequests(char* cmd_passed, def_message_struct *response) {
    
}

int main() {
    return 0;
}