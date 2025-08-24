import serial
import threading
import queue
import time
import datetime
import json

status = ["ONLINE", "ERROR", "OFFLINE"]
available_sensors = []
# --- Configuration ---
SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 115200
serial_queue = queue.Queue()

LINE_END = '|\n'
VAL_SEPER = '|'
CMD_SEPER = ','
initCU = "init|PI|\n"

#Currently this script is only meant to handle one communication unit at a time. in the future further communication units can be expanded on by creating
#multiple serial objects with a list of serial ports

class Sensors:
    name = ""
    cmds = []
    responses = []
    def __init__(self, name, cmds, responses):
        name = name
        cmds = cmds

class SensorUnit:
    name = ""
    sensors = []
    status = ""
    def __init__(self, name, sensors):
        if any(sensors) != type(Sensors):
            raise Exception("Invalid type passed into sensor array")
        else:
            name = name
            sensors = sensors


class CommunicationUnit:
    sens_units = []
    def __init__(self, sens_units):
        if any(sens_units) != type(SensorUnit):
            raise Exception("Invalid sensor unit type.")
        else :
            sens_units = sens_units

#NEVER CHANGE THE ASSIGNMENTS OF THIS
Sensor_units = []
Communication_units = [CommunicationUnit(Sensor_units)]

#TODO implement multiple communication units and communication unit signatures
keywords = ["SENSOR", "Status", "Name", "Sens units", "NUM_OF_SU"]
def handle_msg(msg):
    msg_keywords = msg.split(CMD_SEPER)
    #When initializing a sensor type the structure of the request will come in as so
    #SENSOR|SENSOR_NAME|Commands,Seperated,By,Commas|Responses,Seperated,By,Commas|
    #This will list out the available requests that we can send to the sensors
    if msg_keywords[0] == "SENSOR":
        cmds = keywords[2].split(CMD_SEPER)
        responses = keywords[3].split(CMD_SEPER)
        sensor = Sensors(keywords[1], cmds, responses)
        available_sensors.append(sensor)
    elif msg_keywords[0:5] == "Status":
        #Every message sent to a sensor unit will end with the index of the sensor unit that it is connected to.
        index = int(keywords[-1])
        error_value = msg_keywords[0].split(" ")
        error_value = error_value[1]
        #currently hardcoded 0 since there is only one communication unit
        Communication_units[0].sens_units[index].status = keywords[2]
    elif msg_keywords[0] == "Name":
        index = int(msg_keywords[-1])
        Communication_units[0].sens_units[index].name = keywords[1]
    elif keywords[0] == "Sens units":
        index = int(msg_keywords[-1])
        #The request will come with the names so in this case we will assume that the list of sensors is always succesfully initialized
        #and we will just add the names to the sensor unit
        for item in msg_keywords[1:-1]:
            for sensor in available_sensors:
                if (item == sensor.name):
                    Sensor_units[index].sensors.append(sensor)
                    break
    elif keywords[0] == "NUM_OF_SU":
        num_of_sens_units = int(msg_keywords[-1])
        for i in range(num_of_sens_units):
            Communication_units[0].sens_units.append(SensorUnit([], []))
    #TODO implement message logging and updating proper API files
    #else:
        #Here we handle the information coming in so we compare against the each sensor and see what the response is then log it into our log fail
        #and update the valid json file



def serial_read(ser_port, queue):
    print("Starting serial read thread. press ctrl+c to exit.")
    while True:
        try:
            read_data = ser_port.read(1).decode('UTF-8') + ser_port.read(ser_port.in_waiting).decode('UTF-8')
            if read_data:
                queue.put(read_data)
        except serial.SerialException as e:
            print(f"Serial Error: {e}")
            break
        except KeyboardInterrupt:
            print("\nProgram interrupted by user. Exiting.")
    print("Serial read thread terminated.")



if __name__ == "__main__":
    print("Start monitoring serial port:")

    ser = None

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        ser.flush()  # Clear any old data in the buffers.

    except serial.SerialException as e:
        print(f"Serial Error: {e}")
        print(f"Check that a device is connected to {SERIAL_PORT} and that the port is correct.")

    reader_thread = threading.Thread(target=serial_read, args=(ser, serial_queue))
    reader_thread.start()

    try:
        while True:
            buffer_chunk = serial_queue.get()
            #include here data handling method
    except KeyboardInterrupt:
        print("\nProgram interrupted by user. Exiting.")
    finally:
        if ser and ser.is_open:
            ser.close()
            print("Serial port closed.")