import serial
import threading
import queue
import time
import datetime

# --- Configuration ---
SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 115200
serial_queue = queue.Queue()

LINE_END = '|\n'
VAL_SEPER = '|'
CMD_SEPER = ','
initCU = "init|PI|\n"

# --- Main Application Logic ---
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


ser = None

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    ser.flush()  # Clear any old data in the buffers.

    initial_msg = initCU
    ser.write((initial_msg + LINE_END).encode('utf-8'))
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            print(f"[{timestamp}] Received: {line}")
            time.sleep(10)

except serial.SerialException as e:
    print(f"Serial Error: {e}")
    print(f"Check that a device is connected to {SERIAL_PORT} and that the port is correct.")

except KeyboardInterrupt:
    print("\nProgram interrupted by user. Exiting.")
finally:
    if ser and ser.is_open:
        ser.close()
        print("Serial port closed.")