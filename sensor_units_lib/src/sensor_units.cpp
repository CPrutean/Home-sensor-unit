#include "sensor_units.h"

const char* temp_sensor_cmds[] = {"PULL TEMP", "PULL HUMID"};
const char* gps_sensor_cmds[] = {"PULL LOCATION"};
const char* time_sensor_cmds[] = {"PULL TIME"};
//Pass in an array with unlimited rows and the columns being 16 long
int return_available_commands(char** array, int len, enum sensor_type sensor) {
    if (len != 16) {
        return -1;
    }

    switch (sensor) {
        case(TEMP_AND_HUMID):
            strncpy(*(array), temp_sensor_cmds[0], 16);
            strncpy(*(array+1), temp_sensor_cmds[1], 16);
            break;
        case(GPS):
            strncpy(*(array), gps_sensor_cmds[0], 16);
            break;
        case(TIME):
            strncpy(*(array), time_sensor_cmds[0], 16);
            break;
        default:
            return -1;
    }
    return 0;
}

int handleRequest(enum sensor_type sensor, char* cmd_passed, def_message_struct *response) {
    memset(response, 0, sizeof(response));
    return 0;
}

void handleTempRequests(char* cmd_passed, def_message_struct *response) {

}

void handleGpsRequests(char* cmd_passed, def_message_struct *response) {

}

void handleTimeRequests(char* cmd_passed, def_message_struct *response) {

}

int main() {
    return 0;
}