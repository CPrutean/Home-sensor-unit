import serial
import threading
import queue
import datetime
import api
import time
import os
import pathlib
import csv
import database
from dotenv import load_dotenv

status = ["ONLINE", "ERROR", "OFFLINE"]
available_sensors = []

#constant cuID and suID for initializing communication units and sensor units
cuID = 1000
suID = 1000


# --- Configuration ---
load_dotenv()
SERIAL_PORT = os.getenv("SERIAL_PORT")
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

    def __str__(self):
        return f"Sensor name: {self.name}\nCommands: {self.cmds}\nResponses: {self.responses}"

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
        return f"Sensor unit name: {self.name}\nSensors: {self.sensors}\nStatus: {self.status}\nError Code: {self.error_code}\nID: {self.id}"


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
        return f"Communication unit name: {self.name}\nSensor units: {self.sens_units}\nID: {self.id}"


Sensor_units:list = []
Communication_units:list = [CommunicationUnit(Sensor_units)]


keywords = ["SENSOR", "Status", "Name", "Sens units", "NUM_OF_SU"]
def handle_msg(msg):
    msg_keywords = msg.split(VAL_SEPER)

    for keyword in msg_keywords:
        if keyword == "":
            msg_keywords.remove(keyword)


    # Always log messages recieved
    with open("logs.txt", "a") as f:
        f.write(f"{datetime.datetime.now()}: {msg}\n")
        f.close()
    if (len(msg_keywords) < 2):
        print("Error: Message recieved wasnt formatted correctly")
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
        database.write("sensors", msg_keywords[1:])


    elif msg_keywords[0].split(" ")[0] == "Status":
        index = int(msg_keywords[-1])

        temp = msg_keywords[0].split(" ")
        error_value = temp[1]

        #currently hardcoded 0 since there is only one communication unit
        Communication_units[0].sens_units[index].status = msg_keywords[1]
        Communication_units[0].sens_units[index].error_code = error_value
        database.write("sensor_unit", str(Communication_units[0].sens_units[index]).split("\n"), update = True)



    elif msg_keywords[0] == "Name":
        index = int(msg_keywords[-1])
        Communication_units[0].sens_units[index].name = msg_keywords[1]
        database.write("sensor_unit", str(Communication_units[0].sens_units[index]).split("\n"), update = True)

    elif msg_keywords[0] == "Sens units":
        index = int(msg_keywords[-1])
        #The request will come with the names so in this case we will assume that the list of sensors is always succesfully initialized
        #and we will just add the names to the sensor unit
        for item in msg_keywords[1:-1]:
            for sensor in available_sensors:
                if item == sensor.name:
                    Sensor_units[index].sensors.append(sensor)
                    break
        database.write("sensor_unit", str(Sensor_units[index]).split("\n"), update = True)


    elif msg_keywords[0] == "NUM_OF_SU":
        num_of_sens_units = int(msg_keywords[-1])
        for i in range(num_of_sens_units):
            Communication_units[0].sens_units.append(SensorUnit("", []))
            database.write("sensor_unit", str(Communication_units[0].sens_units[i]).split("\n"))

        database.write("communication_unit", str(Communication_units[0]).split("\n"))
    else:
        ind = int(msg_keywords[-1])
        #What we do is we take the first word of the response and compare it against the other responses to find where in the api.JSON
        #file we should place the response.
        #We also log the message recieved into a log file for future reference.
        store_in_readings(msg_keywords)
        database.write("most_recent_reading", msg_keywords, update = True)


def store_in_readings(reading, reset = False):
    if not isinstance(reading, list):
        print("Error: reading wasnt a dictionary")
        return

    path = "Readings"
    path += "/" + datetime.datetime.now().strftime("%Y-%m-%d") + ".csv"

    path = pathlib.Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    header = ["sensor", "reading", "values", "timestamp"]
    insert_data = []

    global available_sensors
    for sensor in available_sensors:
        if sensor.name == reading[0]:
            insert_data.append(sensor.name)
            break
    insert_data.append(reading[0])
    tempStr = ""
    for value in reading[1:-1]:
        tempStr += value
        tempStr += " "
    insert_data.append(tempStr[0:-1])
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


threads = []
if __name__ == "__main__":
    print("Start monitoring serial port:")
    database.init()

    ser = None

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
