import serial
import threading
import queue
import datetime
import json
import api
import time
import os
import asyncio

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
def handle_msg(msg, lock=None):
    msg_keywords = msg.split(VAL_SEPER)

    for keyword in msg_keywords:
        if keyword == "":
            msg_keywords.remove(keyword)

    if type(lock) != type(threading.Lock()):
        raise Exception("Lock wasnt initialized as a threading.Lock")

    # Always log messages recieved
    with open("logs.txt", "a") as f:
        f.write(f"{datetime.datetime.now()}: {msg}\n")
        f.close()

    if len(msg_keywords) < 2:
        print("Invalid message recieved")
        with open("logs.txt", "a") as f:
            f.write(f"Error message:\n");
            f.write(f"{datetime.datetime.now()}: {msg}\n")
        return

    #When initializing a sensor type the structure of the request will come in as so
    #SENSOR|SENSOR_NAME|Commands,Seperated,By,Commas|Responses,Seperated,By,Commas|
    #This will list out the available requests that we can send to the sensors
    if msg_keywords[0] == "SENSOR":
        global available_sensors
        cmds = msg_keywords[2].split(CMD_SEPER)
        responses = msg_keywords[3].split(CMD_SEPER)

        for cmd in cmds:
            if cmd == "":
                cmds.remove(cmd)

        for response in responses:
           if response == "":
                responses.remove(response)

        available_sensors.append(Sensors(msg_keywords[1], cmds, responses))

        json_obj = return_sensor_jsonobj(msg_keywords[1], cmds, responses)
        add_to_json("api.json", "sensors", json_obj, lock_main = lock)
    elif msg_keywords[0].split(" ")[0] == "Status":
        index = int(msg_keywords[-1])

        temp = msg_keywords[0].split(" ")
        error_value = temp[1]

        #currently hardcoded 0 since there is only one communication unit
        Communication_units[0].sens_units[index].status = msg_keywords[1]
        Communication_units[0].sens_units[index].error_code = error_value
        json_obj = return_su_json_obj(index)
        add_to_json("api.json", "sensor_units", json_obj, overwrite = True, lock_main = lock)

    elif msg_keywords[0] == "Name":
        index = int(msg_keywords[-1])
        Communication_units[0].sens_units[index].name = msg_keywords[1]
        json_obj = return_su_json_obj(index)
        add_to_json("api.json", "sensor_units", json_obj, overwrite = True, lock_main = lock)
    elif msg_keywords[0] == "Sens units":
        index = int(msg_keywords[-1])
        #The request will come with the names so in this case we will assume that the list of sensors is always succesfully initialized
        #and we will just add the names to the sensor unit
        for item in msg_keywords[1:-1]:
            for sensor in available_sensors:
                if item == sensor.name:
                    Sensor_units[index].sensors.append(sensor)
                    break
        json_obj = return_su_json_obj(index)
        add_to_json("api.json", "sensor_units", json_obj, overwrite = True, lock_main = lock)
    elif msg_keywords[0] == "NUM_OF_SU":
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
                    add_to_json("api.json", "readings", json_object, overwrite = True, lock_main = lock)
                    break


#filepath should be a string that specifies the path to the file we are trying to access
#Target object should contain a string that specifies whether we are adding to senors, sensor_units, or communication_units
#new_object should contain a json object to append to the target object
def add_to_json(filepath, target_object, new_object, overwrite = False, lock_main = None, reset_file = False):
    if type(lock_main) != type(threading.Lock()):
        print("Lock in add_to_json wasnt specified")
        exit(-1)

    lock_main.acquire()
    try:
        # Step 1: Read existing data or initialize if file doesn't exist.
        if os.path.exists(filepath):
            with open(filepath, "r") as f:
                try:
                    data = json.load(f)
                except json.JSONDecodeError:
                    # Handle cases where the file is empty or malformed
                    data = {}
        else:
            # If the file doesn't exist, start with an empty dictionary.
            data = {}

        # Ensure the top-level structure is a dictionary
        if not isinstance(data, dict):
            print(f"Error: JSON file at {filepath} must contain a dictionary.")
            return
        if reset_file:
            data = {
                "sensors":[],
                "sensor_units":[],
                "communication_units":[],
                "readings":[]
            }
            with open(filepath, "w") as f:
                json.dump(data, f, indent=4)
            return

        # Ensure the target key exists and is a list.
        if target_object not in data or not isinstance(data[target_object], list):
            data[target_object] = []

        found_and_updated = False
        if overwrite and not target_object=="readings":
            if 'id' not in new_object:
                print("Error: new_object must have an 'id' key for overwrite to work. from object")
                print(new_object)
                return
        elif overwrite and target_object=="readings":
            for reading in data["readings"]:
                if reading.keys() == new_object.keys():
                    data["readings"].remove(reading)
                    data["readings"].append(new_object)
                    found_and_updated = True
                    break

        if not found_and_updated:
            data[target_object].append(new_object)

        with open(filepath, "w") as f:
            json.dump(data, f, indent=4)

    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        print(new_object)
    finally:
        lock_main.release()


#returns the proper format for sensors, sensor units, and communication units

#Returns a json object with the current state of the sensor unit with the provided index
def return_su_json_obj(index):
    global Communication_units
    if (index < 0) or (index >= len(Communication_units[0].sens_units)):
        return {}
    sens_name_list = []
    for sensor_unit in Communication_units[0].sens_units[index].sensors:
        sens_name_list.append(sensor_unit.name)
    #
    return_val = {"id":Communication_units[0].sens_units[index].id,
        "name":Communication_units[0].sens_units[index].name,
        "sensors":sens_name_list,
        "status":Communication_units[0].sens_units[index].status,
        "error_code":Communication_units[0].sens_units[index].error_code,
    }
    return return_val

def return_cu_json_obj(index = 0):
    #currently the index is hardcoded to 0 since there is only one communication unit more to be added in the future
    json_obj = {"id":Communication_units[0].id, "name":Communication_units[0].name, "sensor_units":[]}
    for sensor_unit in Communication_units[0].sens_units:
        json_obj["sensor_units"].append(sensor_unit.id)
    return json_obj

def return_sensor_jsonobj(name, cmds = [], responses = []):
    if type(cmds or responses) != list:
        print("Error: cmds or responses wasnt a list")
        return {}
    if name is None or type(name) != str:
        print("name in sensor unit wasnt a string or wasnt specified")
        return {}
    if len(cmds) == 0 or len(responses) == 0:
        print("cmds or responses was empty")
        return {}

    json_obj = {"name":name, "cmds":cmds, "responses":responses}
    return json_obj

#Return the sensor reading with the id of the sensor unit that it is connected to
def return_sensor_reading_json_obj(index, response, reading, sensor_name = ""):
    return {
        response:reading,
        "sensor":sensor_name,
        "id":Communication_units[0].sens_units[index].id
    }

def collect_readings(serial_port):
    while True:
        msg = "PULL|ALL|\n"
        serial_port.write(msg.encode('utf-8'))
        time.sleep(60)


def serial_read(ser_port, queue):
    print("Starting serial read thread. press ctrl+c to exit.")
    while True:
        try:
            if ser_port.in_waiting > 0:
                buffer_chunk = ser_port.readline().decode('utf-8').strip()
                print("MESSAGE RECIEVED:" + buffer_chunk)
                queue.put(buffer_chunk)
        except serial.SerialException as e:
            print(f"Serial Error: {e}")
            break
        except KeyboardInterrupt:
            print("\nProgram interrupted by user. Exiting.")
    print("Serial read thread terminated.")


threads = []
if __name__ == "__main__":
    print("Start monitoring serial port:")

    ser = None
    json_lock = threading.Lock()
    if type(json_lock) != type(threading.Lock()):
        raise Exception("json_lock wasnt initialized as a threading.Lock")
    else:
        print("Lock successfully created.")

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        ser.flush()
        ser.write(initCU.encode('utf-8'))
    except serial.SerialException as e:
        print(f"Serial Error: {e}")
        print(f"Check that a device is connected to {SERIAL_PORT} and that the port is correct.")

    reader_thread = threading.Thread(target=serial_read, args=(ser, serial_queue, ))
    reader_thread.start()
    threads.append(reader_thread)

    api_thread = threading.Thread(target=api.main_api_thread, args=(json_lock, ser, ))
    api_thread.start()
    threads.append(api_thread)

    probing_thread = threading.Thread(target=collect_readings, args=(ser, ))
    probing_thread.start()
    threads.append(probing_thread)

    add_to_json("api.json", "sensors", {}, lock_main = json_lock, reset_file = True)
    add_to_json("api.json", "communication_units", return_cu_json_obj(), overwrite = True, lock_main = json_lock)

    print("Threads started, reading messages")
    try:
        while True:
            buffer_chunk = serial_queue.get()
            handle_msg(buffer_chunk, lock=json_lock)
            time.sleep(0.005)

    except KeyboardInterrupt:
        print("\nProgram interrupted by user. Exiting.")
    finally:
        if ser and ser.is_open:
            ser.close()
            print("Serial port closed.")
            for thread in threads:
                thread.join()
                thread.close()
