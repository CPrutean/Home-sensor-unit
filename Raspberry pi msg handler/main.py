import serial
import threading
import queue
import datetime
import api
import time
import os
import pathlib
import csv
from dotenv import load_dotenv
import json


status = ["ONLINE", "ERROR", "OFFLINE"]
available_sensors = []


#constant cuID and suID for initializing communication units and sensor units
cuID = 1000
suID = 1000
sensID = 1000


# --- Configuration ---
load_dotenv()
SERIAL_PORT = os.getenv("SERIAL_PORT")
json_path = os.getenv("JSON_PATH")
json_lock = threading.Lock()
logs_lock = threading.Lock()


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
    id = 0
    def __init__(self, name, cmds, responses):
        self.name = name
        self.cmds = cmds
        self.responses = responses
        global sensID
        self.id = sensID
        sensID+=1

    def __str__(self):
        command_string = ""
        for command in self.cmds:
            if len(self.cmds) <= 0:
                command_string += command
                command_string += ","
        command_string = command_string[0:-1]
        response_string = ""
        for response in self.responses:
            if len(self.responses) <= 0:
                response_string += response
                response_string += ","
        response_string = response_string[0:-1]

        return f"Sensor name: {self.name}\nCommands: {command_string}\nResponses: {response_string}\nID:{self.id}"

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
    def __str__(self):
        sensor_string = ""
        for sensor in self.sensors:
            sensor_string += sensor.name
            sensor_string += ","
        sensor_string = sensor_string[0:-1]
        return f"Name:{self.name}\nSensors:{sensor_string}\nStatus:{self.status}\nError Code:{self.error_code}\nID:{self.id}"


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
    def __str__(self):
        temp = ""
        for sensor_unit in self.sens_units:
            temp+=sensor_unit.id
            temp+=","
        temp = temp[0:-1]
        return f"Name:{self.name}\nSensor units:{temp}\nID:{self.id}"

class Most_recent_readings:
    sensor = ""
    reading = ""
    values = []
    timestamp = ""
    def __str__(self):
        values_string = ""
        for value in self.values:
            values_string += value
            values_string += ","
        values_string = values_string[0:-1]

        return f"Sensor:{self.sensor}\nReading:{self.reading}\nValues:{values_string}\nTimestamp:{self.timestamp}"

    def __init__(self, sensor, reading, values, timestamp):
        self.sensor = sensor
        self.reading = reading
        self.values = values
        self.timestamp = timestamp

Sensor_units:list = []
Communication_units:list = [CommunicationUnit(Sensor_units)]


keywords = ["SENSOR", "Status", "Name", "Sens units", "NUM_OF_SU"]
def handle_msg(msg):
    msg_keywords = msg.split(VAL_SEPER)

    for keyword in msg_keywords:
        if keyword == "":
            msg_keywords.remove(keyword)
    global available_sensors

    log(msg)

    if (len(msg_keywords) < 2):
        print("Error: Message recieved wasnt formatted correctly")
        return

    #When initializing a sensor type the structure of the request will come in as so
    #SENSOR|SENSOR_NAME|Commands,Seperated,By,Commas|Responses,Seperated,By,Commas|
    #This will list out the available requests that we can send to the sensors
    if msg_keywords[0] == "SENSOR":
        cmds = msg_keywords[2].split(CMD_SEPER)
        responses = msg_keywords[3].split(CMD_SEPER)

        for cmd in cmds:
            if cmd == "":
                cmds.remove(cmd)

        for response in responses:
           if response == "":
                responses.remove(response)
        obj = Sensors(msg_keywords[1], cmds, responses)
        available_sensors.append(obj)
        write_to_json("Sensors", str(obj))



    elif msg_keywords[0].split(" ")[0] == "Status":
        index = int(msg_keywords[-1])

        temp = msg_keywords[0].split(" ")
        error_value = temp[1]

        #currently hardcoded 0 since there is only one communication unit
        Communication_units[0].sens_units[index].status = msg_keywords[1]
        Communication_units[0].sens_units[index].error_code = error_value
        write_to_json("Sensor units", str(Communication_units[0].sens_units[index]), update = True, update_key = Communication_units[0].sens_units[index].id)


    elif msg_keywords[0] == "Name":
        index = int(msg_keywords[-1])
        Communication_units[0].sens_units[index].name = msg_keywords[1]
        write_to_json("Sensor units", str(Communication_units[0].sens_units[index]), update = True, update_key = Communication_units[0].sens_units[index].id)

    elif msg_keywords[0] == "Sens units":
        index = int(msg_keywords[-1])
        #The request will come with the names so in this case we will assume that the list of sensors is always succesfully initialized
        #and we will just add the names to the sensor unit
        for item in msg_keywords[1:-1]:
            for sensor in available_sensors:
                if item == sensor.name:
                    Sensor_units[index].sensors.append(sensor)
                    break
        write_to_json("Sensor units", str(Sensor_units[index]), update = True, update_key = Sensor_units[index].id)


    elif msg_keywords[0] == "NUM_OF_SU":
        num_of_sens_units = int(msg_keywords[-1])
        for i in range(num_of_sens_units):
            Communication_units[0].sens_units.append(SensorUnit("", []))
            write_to_json("Sensor units", str(Communication_units[0].sens_units[i]))

    else:
        ind = int(msg_keywords[-1])
        #Assume a reading comes in the form of:
        #RESPONSE|Values|Another value|Index
        temp_sensor = ""
        for sensor in available_sensors:
            if msg_keywords[0] in sensor.responses:
                temp_sensor = sensor.name
                break
        if temp_sensor == "":
            print("Error: No sensor found with the response")
            return

        reading = Most_recent_readings(temp_sensor, msg_keywords[0], msg_keywords[1:-1], datetime.datetime.now().strftime("%m-%d-%Y: %H:%M:%S"))
        store_in_readings(msg_keywords)
        write_to_json("Most recent readings")


def log(msg):
    # Always log messages recieved
    logs_lock.acquire()
    try:
        with open("logs.txt", "a") as f:
            f.write(f"{datetime.datetime.now()}: {msg}\n")
            f.close()
    except Exception as e:
        print(f"Error: {e}")
    finally:
        logs_lock.release()



def store_in_readings(reading, reset = False):
    if not isinstance(reading, list):
        print("Error: reading wasnt a dictionary")
        return

    path = "Readings"
    path += "/" + datetime.datetime.now().strftime("%Y-%m-%d") + ".csv"

    path = pathlib.Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    header = ["sensor", "reading", "values", "timestamp"]
    header = ["sensor", "reading", "values", "timestamp"]
    insert_data = []

    global available_sensors
    for sensor in available_sensors:
        if sensor.name == reading[0]:
            insert_data.append(sensor.name)
            break
    insert_data.append(reading[0])
    temp_str = ""
    for value in reading[1:-1]:
        temp_str += value
        temp_str += " "
    insert_data.append(temp_str[0:-1])
    insert_data.append(datetime.datetime.now().strftime("%m-%d-%Y: %H:%M:%S"))

    with open(path, "w", newline='') as f:
        writer = csv.DictWriter(f, fieldnames=header)
        writer.writeheader()
        writer.writerow({"sensor":insert_data[0], "reading":insert_data[1], "values":insert_data[2], "timestamp":insert_data[3]})


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

obj_key = ["Sensors", "Sensor units", "Communication units", "Most recent readings"]

def return_json_obj(string):
    obj_split = string.split("\n")
    obj = {}
    for item in obj_split:
        if item == "":
            continue
        obj.update({item.split(":")[0]:item.split(":")[1]})
    return obj

def write_to_json(target_obj, value, update = False, update_key = None):
    global json_path
    global json_lock
    global obj_key
    if target_obj not in obj_key:
        print("Error: Invalid object name")
        return
    if update and update_key is None:
        print("Error: update_key wasnt specified")
        return


    if not target_obj == "Most recent readings" and not isinstance(update_key, int):
        print("Error: update_key wasnt an integer")
        return

    json_lock.acquire()
    #update_key needs to be a list of the form [sensor, reading]

    try:
        with open(json_path, "w") as f:
            data = json.load(f)

        if not update:
            data[target_obj].append(return_json_obj(value))
        elif target_obj == "Most recent readings":
            return_json_obj(value)
            found = False
            for item in data[target_obj]:
                if item["Sensor"] == value["Sensor"] and item["Reading"] == value["Reading"]:
                    data[target_obj].remove(item)
                    data.append(value)
                    found = True
                    break
            if not found:
                data.append(value)
        else:
            for item in data[target_obj]:
                if item["ID"] == update_key:
                    data[target_obj].remove(item)
                    data[target_obj].append(return_json_obj(value))
                    break
        json.dump(data, f, indent=4)
    except Exception as e:
        print(f"Error: {e}")
    except json.JSONDecodeError as e:
        print(f"Error: {e}")
    finally:
        json_lock.release()

def init_json():
    global json_path
    global json_lock
    json_lock.acquire()
    try:
        with open(json_path, "w") as f:
            json.dump({
                "Sensors": [],
                "Sensor units": [],
                "Communication units": [],
                "Most recent readings": []
            }, f, indent=4)
    except Exception as e:
        print(f"Error: {e}")
    except json.JSONDecodeError as e:
        print(f"Error: {e}")
    finally:
        json_lock.release()


threads = []
if __name__ == "__main__":
    print("Start monitoring serial port:")
    init_json()
    ser = None

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        ser.flush()
        ser.write(initCU.encode('utf-8'))
    except serial.SerialException as e:
        print(f"Serial Error: {e}")
        print(f"Check that a device is connected to {SERIAL_PORT} and that the port is correct.")

    api.assign_lock_and_ser_port(json_lock, ser)

    reader_thread = threading.Thread(target=serial_read, args=(ser, serial_queue, ))
    reader_thread.start()
    threads.append(reader_thread)

    api.run_app()

    probing_thread = threading.Thread(target=collect_readings, args=(ser, ))
    probing_thread.start()
    threads.append(probing_thread)

    print("Threads started, reading messages")
    try:
        while True:
            buffer_chunk = serial_queue.get()
            handle_msg(buffer_chunk)
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
