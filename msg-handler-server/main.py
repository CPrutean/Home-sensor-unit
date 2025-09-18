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

# --- CONFIGURATION ---
load_dotenv()
SERIAL_PORT = os.getenv("SERIAL_PORT")
JSON_PATH = os.getenv("JSON_PATH")
BAUD_RATE = 115200

# --- SHARED RESOURCES (using locks for thread safety) ---
json_lock = threading.Lock()
logs_lock = threading.Lock()
serial_queue = queue.Queue()

# --- CLASSES (Refactored for better OOP) ---

class Sensors:
    # Use a class variable for the ID counter, not a global one.
    id_counter = 0

    def __init__(self, name, cmds, responses):
        self.name = name
        self.cmds = cmds
        self.responses = responses
        self.id = Sensors.id_counter
        Sensors.id_counter += 1

    def __str__(self):
        # Use .join() for correct and efficient string formatting.
        command_string = ",".join(self.cmds)
        response_string = ",".join(self.responses)
        return f"Sensor name:{self.name}\nCommands:{command_string}\nResponses:{response_string}\nID:{self.id}"

    # Add a to_dict method for clean JSON serialization.
    def to_dict(self):
        return {
            "ID": self.id,
            "Name": self.name,
            "Commands": self.cmds,
            "Responses": self.responses
        }

class SensorUnit:
    id_counter = 0
    def __init__(self, name, sensors, status="OFFLINE", error_code=""):
        if not all(isinstance(s, Sensors) for s in sensors):
            raise TypeError("Invalid sensor type in list.")
        self.name = name
        self.sensors = sensors
        self.status = status
        self.error_code = error_code
        self.id = SensorUnit.id_counter
        SensorUnit.id_counter += 1

    def to_dict(self):
        return {
            "ID": self.id,
            "Name": self.name,
            "Status": self.status,
            "Error Code": self.error_code,
            "Sensors": [s.name for s in self.sensors]
        }

class CommunicationUnit:
    id_counter = 0
    def __init__(self, sens_units, name="Communication Unit"):
        if not all(isinstance(su, SensorUnit) for su in sens_units):
            raise TypeError("Invalid sensor unit type in list.")
        self.sens_units = sens_units
        self.name = name
        self.id = CommunicationUnit.id_counter
        CommunicationUnit.id_counter += 1

    def to_dict(self):
        return {
            "ID": self.id,
            "Name": self.name,
            "Sensor units": [su.id for su in self.sens_units]
        }

class RecentReading:
    def __init__(self, sensor, reading, values, value):
        self.sensor = sensor
        self.reading = reading
        self.values = values
        self.timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.value = value

    def to_dict(self):
        return {
            "Sensor": self.sensor,
            "Reading": self.reading,
            "Values": self.values,
            "Timestamp": self.timestamp,
            "Value": self.value
        }

# --- APPLICATION STATE ---
# It's better to contain state in a single class or dictionary if it gets complex.
# For now, we'll keep these but aim to pass them as arguments.
available_sensors = []
sensor_units = []
communication_unit = CommunicationUnit([]) # Assuming one main CU

# --- CORE LOGIC ---

def handle_msg(msg):
    log(msg)
    # Safely split and remove empty strings
    msg_parts = [part for part in msg.split('|') if part]

    if len(msg_parts) < 2:
        print("Error: Malformed message received")
        return

    keyword = msg_parts[0]

    # SENSOR|SENSOR_NAME|CMD1,CMD2|RESP1,RESP2
    if keyword == "SENSOR":
        name, cmds_str, responses_str = msg_parts[1], msg_parts[2], msg_parts[3]
        cmds = [c for c in cmds_str.split(',') if c]
        responses = [r for r in responses_str.split(',') if r]
        sensor_obj = Sensors(name, cmds, responses)
        available_sensors.append(sensor_obj)
        write_to_json("Sensors", sensor_obj.to_dict())

    # NUM_OF_SU|3
    elif keyword == "NUM_OF_SU":
        num_units = int(msg_parts[1])
        for _ in range(num_units):
            su = SensorUnit("", [])
            sensor_units.append(su)
            communication_unit.sens_units.append(su)
            write_to_json("Sensor units", su.to_dict())
        write_to_json("Communication units", communication_unit.to_dict(), update=True)

    # All other messages seem to include an index at the end.
    else:
        try:
            # Assume index is always the last part for status, name, etc.
            index = int(msg_parts[-1])
            su = communication_unit.sens_units[index]

            if keyword.startswith("Status"):
                su.status = msg_parts[1]
                su.error_code = keyword.split(" ")[1] if " " in keyword else ""
                write_to_json("Sensor units", su.to_dict(), update=True, update_key=su.id)
            elif keyword == "Name":
                su.name = msg_parts[1]
                write_to_json("Sensor units", su.to_dict(), update=True, update_key=su.id)
            elif keyword == "Sens units":
                sensor_names = msg_parts[1:-1]
                su.sensors = [s for s in available_sensors if s.name in sensor_names]
                write_to_json("Sensor units", su.to_dict(), update=True, update_key=su.id)
            else: # Assumed to be a sensor reading
                response_keyword = keyword
                values = msg_parts[1:-2]
                sensor_name = next((s.name for s in available_sensors if response_keyword in s.responses), None)
                value = msg_parts[-2]
                if not sensor_name:
                    print(f"Error: No sensor found for response '{response_keyword}'")
                    return

                reading = RecentReading(sensor_name, response_keyword, values, value)
                store_in_readings(reading)
                write_to_json("Most recent readings", reading.to_dict(), update=True)

        except (ValueError, IndexError) as e:
            print(f"Error processing message '{msg}': {e}")


def write_to_json(target_obj, value_dict, update=False, update_key=None):
    # This function now accepts a dictionary directly. No more string parsing!
    json_lock.acquire()
    try:
        try:
            with open(JSON_PATH, 'r') as f:
                data = json.load(f)
        except (FileNotFoundError, json.JSONDecodeError):
            # If file is missing or corrupt, start fresh
            data = {"Sensors": [], "Sensor units": [], "Communication units": [], "Most recent readings": []}

        if not update:
            data[target_obj].append(value_dict)
        else:
            # Unified update logic
            found = False
            # Special case for "Most recent readings", which updates based on sensor name
            if target_obj == "Most recent readings":
                #Needs to math the reading and sensor objects to upodate
                for reading in data[target_obj]:
                    if reading["Sensor"] == value_dict["Sensor"] and reading["Reading"] == value_dict["Reading"]:
                        data[target_obj].remove(reading)
                        data[target_obj].append(value_dict)
                        found = True
                        break
                if not found:
                    data[target_obj].append(value_dict)
                with open(JSON_PATH, 'w') as f:
                    json.dump(data, f, indent=4)
                return

            else: # All other objects update based on ID
                key_to_match = "ID"
                value_to_match = update_key

            for i, item in enumerate(data[target_obj]):
                if item.get(key_to_match) == value_to_match:
                    data[target_obj][i] = value_dict # Replace item in-place
                    found = True
                    break
            if not found:
                data[target_obj].append(value_dict)

        with open(JSON_PATH, 'w') as f:
            json.dump(data, f, indent=4)

    except Exception as e:
        print(f"Error writing to JSON: {e}")
    finally:
        json_lock.release()

def store_in_readings(reading_obj):
    # This function now accepts the object and accesses its attributes directly.
    if not isinstance(reading_obj, RecentReading):
        print("Error: Invalid object passed to store_in_readings.")
        return

    log_directory = pathlib.Path("Readings")
    log_directory.mkdir(parents=True, exist_ok=True)
    path = log_directory / f"{datetime.datetime.now().strftime('%Y-%m-%d')}.csv"

    header = ["sensor", "reading", "values", "value", "timestamp"]
    row_data = {
        "sensor": reading_obj.sensor,
        "reading": reading_obj.reading,
        "values": " ".join(reading_obj.values), # Join values into a single string
        "value": reading_obj.value,
        "timestamp": reading_obj.timestamp
    }

    file_exists = path.is_file()
    with open(path, "a", newline='') as f:
        writer = csv.DictWriter(f, fieldnames=header)
        if not file_exists:
            writer.writeheader()
        writer.writerow(row_data)

def log(msg):
    logs_lock.acquire()
    try:
        # Use 'with' which handles file closing automatically.
        with open("logs.txt", "a") as f:
            f.write(f"{datetime.datetime.now()}: {msg}\n")
    except Exception as e:
        print(f"Error writing to log: {e}")
    finally:
        logs_lock.release()

def init_json():
    # Write the CU object to JSON on startup
    with open(JSON_PATH, "w") as f:
        json.dump({"Sensors": [], "Sensor units": [], "Communication units": [], "Most recent readings": []}, f, indent=4)
    write_to_json("Communication units", communication_unit.to_dict())

def serial_read_thread(ser_port, queue):
    print("Starting serial read thread...")
    while True:
        try:
            if ser_port.in_waiting > 0:
                line = ser_port.readline().decode('utf-8').strip()
                if line: # Avoid processing empty lines
                    print(f"MESSAGE RECEIVED: {line}")
                    queue.put(line)
        except serial.SerialException:
            print("Serial port disconnected. Exiting thread.")
            break
        except Exception as e:
            print(f"An error occurred in serial thread: {e}")
            break

def collect_readings_thread(serial_port):
    while True:
        try:
            msg = "PULL|ALL|\n"
            serial_port.write(msg.encode('utf-8'))
            time.sleep(60) # Wait for 1 minute
        except serial.SerialException:
            print("Serial port disconnected. Exiting collection thread.")
            break

def handle_serial_input(ser_queue):
    while True:
        # Main thread processes messages from the queue
        message = ser_queue.get()
        handle_msg(message)

# --- MAIN EXECUTION ---
if __name__ == "__main__":
    print("Starting serial monitoring application...")
    init_json()
    ser = None
    threads = []

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        ser.flush()
        ser.write("INIT|PI|\n".encode('utf-8'))
        print(f"Successfully opened serial port {SERIAL_PORT}")

        # Start the reader thread
        reader = threading.Thread(target=serial_read_thread, args=(ser, serial_queue))
        reader.daemon = True # Allows main thread to exit even if this one is blocking
        reader.start()
        threads.append(reader)


        # Start the periodic data collection thread
        collector = threading.Thread(target=collect_readings_thread, args=(ser,))
        collector.daemon = True
        collector.start()
        threads.append(collector)

        handler = threading.Thread(target=handle_serial_input, args=(serial_queue,))
        handler.daemon = True
        handler.start()
        threads.append(handler)

        api.run_app()


    except serial.SerialException as e:
        print(f"FATAL: Could not open serial port. {e}")
    except KeyboardInterrupt:
        print("\nShutdown signal received.")
    finally:
        if ser and ser.is_open:
            ser.close()
            print("Serial port closed.")
        print("Application terminated.")