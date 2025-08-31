import serial
import threading
import queue
import time
import datetime
import json
import os
import api
from flask import Flask, request, jsonify

status = ["ONLINE", "ERROR", "OFFLINE"]
available_sensors = []

#constant cuID and suID for initializing communication units and sensor units
cuID = 1000
suID = 1000


# --- Configuration ---
SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 115200
serial_queue = queue.Queue()

LINE_END = '|\n'
VAL_SEPER = '|'
CMD_SEPER = ','
initCU = "INIT|PI|\n"


#Currently this script is only meant to handle one communication unit at a time. in the future further communication units can be expanded on by creating
#multiple serial objects with a list of serial ports

class Sensors:
    name = ""
    cmds = []
    responses = []
    def __init__(self, name, cmds, responses):
        self.name = name
        self.cmds = cmds
        self.responses = responses

class SensorUnit:
    name = ""
    sensors = []
    status = ""
    error_code = ""
    id = 0
    def __init__(self, name, sensors):
        if not isinstance(sensors, list):
            raise Exception("Invalid type passed into sensor array")
        else:
            for sensor in sensors:
                if not isinstance(sensor, Sensors):
                    raise Exception("Invalid sensor type.")
            self.name = name
            self.sensors = sensors
            global suID
            self.id = suID
            suID+=1


class CommunicationUnit:
    name = ""
    sens_units = []
    id = 0
    def __init__(self, sens_units, name = "Communication Unit"):
        if not isinstance(sens_units, list):
            raise Exception("Invalid type passed wasn't a list")
        else :
            for sensor_unit in sens_units:
                if not isinstance(sensor_unit, SensorUnit):
                    raise Exception("Invalid sensor unit type.")
            self.sens_units = sens_units
            global cuID
            self.id = cuID
            self.name = name
            cuID+=1


Sensor_units:list = []
Communication_units:list = [CommunicationUnit(Sensor_units)]


keywords = ["SENSOR", "Status", "Name", "Sens units", "NUM_OF_SU"]
def handle_msg(msg, lock):
    msg_keywords = msg.split(CMD_SEPER)
    if len(msg_keywords) < 2:
        print("Invalid message recieved")
        with open("logs.txt", "a") as f:
            f.write(f"{datetime.datetime.now()}: {msg}\n")
        return

    #When initializing a sensor type the structure of the request will come in as so
    #SENSOR|SENSOR_NAME|Commands,Seperated,By,Commas|Responses,Seperated,By,Commas|
    #This will list out the available requests that we can send to the sensors
    if msg_keywords[0] == "SENSOR":
        json_obj = return_sensor_jsonobj(msg_keywords)
        add_to_json("api.JSON", "sensors", json_obj)
    elif msg_keywords[0:5] == "Status":
        #Every message sent to a sensor unit will end with the index of the sensor unit that it is connected to.
        index = int(keywords[-1])
        error_value = msg_keywords[0].split(" ")
        error_value = error_value[1]

        #currently hardcoded 0 since there is only one communication unit
        Communication_units[0].sens_units[index].status = keywords[2]
        Communication_units[0].sens_units[index].error_code = error_value
        json_obj = return_su_json_obj(index)
        add_to_json("api.JSON", "sensor_units", json_obj, overwrite = True)

    elif msg_keywords[0] == "Name":
        index = int(msg_keywords[-1])
        Communication_units[0].sens_units[index].name = msg_keywords[1]
        json_obj = return_su_json_obj(index)
        add_to_json("api.JSON", "sensor_units", json_obj, overwrite = True)
    elif keywords[0] == "Sens units":
        index = int(msg_keywords[-1])
        #The request will come with the names so in this case we will assume that the list of sensors is always succesfully initialized
        #and we will just add the names to the sensor unit
        for item in msg_keywords[1:-1]:
            for sensor in available_sensors:
                if item == sensor.name:
                    Sensor_units[index].sensors.append(sensor)
                    break
        json_obj = return_su_json_obj(index)
        add_to_json("api.JSON", "sensor_units", json_obj, overwrite = True)
    elif keywords[0] == "NUM_OF_SU":
        num_of_sens_units = int(msg_keywords[-1])
        for i in range(num_of_sens_units):
            Communication_units[0].sens_units.append(SensorUnit("", []))
    else:
        ind = int(msg_keywords[-1])
        #What we do is we take the first word of the response and compare it against the other responses to find where in the api.JSON
        #file we should place the response.
        #We also log the message recieved into a log file for future reference.
        for sensor in Communication_units[0].sens_units[ind].sensors:
            for response in sensor.responses:
                if msg_keywords[0] == response:
                    json_object = return_sensor_reading_json_obj(ind, response, msg_keywords[1:-1], sensor.name)
                    add_to_json("api.JSON", "readings", json_object, overwrite = True)
                    break

    #Always log messages recieved
    with open("logs.txt", "a") as f:
        f.write(f"{datetime.datetime.now()}: {msg}\n")
    return msg_keywords[-1]

#TODO implement race condition handling for multiple python processes trying to access the same file
#filepath should be a string that specifies the path to the file we are trying to access
#Target object should contain a string that specifies whether we are adding to senors, sensor_units, or communication_units
#new_object should contain a json object to append to the target object
def add_to_json(filepath, target_object, new_object, overwrite = False, lock_main = None):
    if lock_main is None:
        print("Lock wasnt specified ")
        return {}

    lock_main.acquire()
    try:
        with open(filepath, "r") as f:
            data = json.load(f)
        if not isinstance(data, dict):
            print("Error: JSON file must contain a list of objects.")
        else:
            if overwrite and target_object != "readings":
                for entry in data[target_object]:
                    if entry[id] == new_object[id]:
                        data[target_object].remove(entry)
                        data.update(new_object)
                        break
            elif overwrite and target_object == "readings":
                for entry in data[target_object]:
                    if entry["id"] == new_object["id"] and entry["response"] == new_object["response"]:
                        data[target_object].remove(entry)
                        data.update(new_object)
                        break
            else:
                data[target_object].update(new_object)
    except FileNotFoundError:
        data = {}
    except e as Exception:
        print(f"Error loading JSON file: {e}")
        return
    finally:
        lock_main.release()

#returns the proper format for sensors, sensor units, and communication units

#Returns a json object with the current state of the sensor unit with the provided index
def return_su_json_obj(index):
    global Communication_units
    if (index < 0) or (index >= len(Communication_units[0].sens_units)):
        return {}
    sens_name_list = []
    for sensor_unit in Communication_units[0].sens_units:
        sens_name_list.append(sensor_unit.name)
    #
    return_val = {Communication_units[0].sens_units[index].id:{
        "name":Communication_units[0].sens_units[index].name,
        "sensors":sens_name_list,
        "status":Communication_units[0].sens_units[index].status,
        "error_code":Communication_units[0].sens_units[index].error_code,
    }}
    return return_val

def return_cu_json_obj(index = 0):
    #currently the index is hardcoded to 0 since there is only one communication unit more to be added in the future
    json_obj = {Communication_units[0].id:{
        "name":Communication_units[0].name,
        "sensor_units":[]
    }}
    for sensor_unit in Communication_units[0].sens_units:
        json_obj["sensor_units"].append(sensor_unit.id)
    return json_obj

def return_sensor_jsonobj(msg_keywords):
    if type(msg_keywords) != list:
        return {}
    cmds = msg_keywords[1].split(CMD_SEPER)
    responses = msg_keywords[2].split(CMD_SEPER)
    json_obj = {"name":msg_keywords[0], "cmds":cmds, "responses":responses}
    return json_obj

#Return the sensor reading with the id of the sensor unit that it is connected to
def return_sensor_reading_json_obj(index, response, reading, sensor_name = ""):
    return {
        response:reading,
        "sensor":sensor_name,
        "id":Communication_units[0].sens_units[index].id
    }


def serial_read(ser_port, queue):
    print("Starting serial read thread. press ctrl+c to exit.")
    while True:
        try:
            read_data = ser_port.read(size=1).decode('utf-8') + ser_port.read(ser_port.in_waiting).decode('utf-8')
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
    json_lock = threading.Lock()
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        ser.flush()
        ser.write(initCU.encode('utf-8'))
    except serial.SerialException as e:
        print(f"Serial Error: {e}")
        print(f"Check that a device is connected to {SERIAL_PORT} and that the port is correct.")

    reader_thread = threading.Thread(target=serial_read, args=(ser, serial_queue))
    reader_thread.start()
    api_thread = threading.Thread(target=api.main_api_thread, args=(json_lock, ser))
    api_thread.start()

    try:
        while True:
            buffer_chunk = serial_queue.get()
            handle_msg(buffer_chunk, json_lock)
    except KeyboardInterrupt:
        print("\nProgram interrupted by user. Exiting.")
    finally:
        if ser and ser.is_open:
            ser.close()
            print("Serial port closed.")
            reader_thread.join()
            api_thread.join()