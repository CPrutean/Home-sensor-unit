from flask import Flask, request, jsonify
import os
import json
import threading
import serial
from dotenv import load_dotenv

load_dotenv()
json_lock = None
ser_port = None
new_flag = False
json_path = os.getenv("JSON_PATH")

def assign_lock_and_ser_port(lock_in, ser_port_in):
    global lock
    global ser_port
    if lock_in is None or ser_port_in is None:
        print("Error: lock_in or ser_port_in wasnt specified")
        exit(-1)

    lock = lock_in
    ser_port = ser_port_in



def run_app():
    app.run(host='0.0.0.0', port=80)

def set_new_flag():
    global new_flag
    new_flag = True

app=Flask(__name__)


@app.route('/')
def return_all():
    global json_path
    global json_lock
    json_lock.acquire()
    try:
        with open(json_path, "r") as f:
            return json.load(f)
    except Exception as e:
        print(f"Error: {e}")
        return{"ERROR":"FAILED TO READ JSON FILE"}
    except json.JSONDecodeError as e:
        print(f"Error: {e}")
        return{"ERROR":"FAILED TO PARSE JSON FILE"}
    finally:
        json_lock.release()

@app.route('/sensors', methods = ["GET"])
def return_sensors():
    global json_path
    global json_lock
    json_lock.acquire()
    try:
        with open(json_path, "r") as f:
            data = json.load(f)
        return jsonify(data["Sensors"])
    except Exception as e:
        print(f"Error: {e}")
        return{"ERROR":"FAILED TO READ JSON FILE"}
    except json.JSONDecodeError as e:
        print(f"Error: {e}")
        return{"ERROR":"FAILED TO PARSE JSON FILE"}
    finally:
        json_lock.release()


@app.route('/sensor/<int:sensor_id>', methods = ["GET"])
def return_sensor(sensor_id):
    global json_path
    global json_lock
    json_lock.acquire()
    try:
        with open(json_path, "r") as f:
            data = json.load(f)
        for sensor in data["Sensors"]:
            if sensor["ID"] == sensor_id:
                return jsonify(sensor)
        return{"ERROR":"NO SENSOR WITH THAT ID"}
    except Exception as e:
        return{"ERROR":"FAILED TO READ JSON FILE"}
    except json.JSONDecodeError as e:
        return{"ERROR":"FAILED TO PARSE JSON FILE"}
    finally:
        json_lock.release()

#will return all readings for a sensor unit
@app.route('/readings/', methods = ["GET"])
def return_readings():
    global json_path
    global json_lock
    json_lock.acquire()
    try:
        with open(json_path, "r") as f:
            data = json.load(f)
        return jsonify(data["Most recent readings"])
    except Exception as e:
        return{"ERROR":"FAILED TO READ JSON FILE"}
    except json.JSONDecodeError as e:
        return{"ERROR":"FAILED TO PARSE JSON FILE"}
    finally:
        json_lock.release()

@app.route('/readings/<string:sensor_name>', methods = ["GET"])
def return_readings_for_sensor(sensor_name):
    global json_path
    global json_lock
    json_lock.acquire()
    try:
        with open(json_path, "r") as f:
            data = json.load(f)
        reading_list = []
        found = False
        for reading in data["Most recent readings"]:
            if reading["Sensor"] == sensor_name:
                found = True
                reading_list.append(reading)
        if found:
            return jsonify({sensor_name:reading_list})
        else:
            return{"ERROR":"NO READINGS FOR THAT SENSOR"}
    except Exception as e:
        return{"ERROR":"FAILED TO READ JSON FILE"}
    except json.JSONDecodeError as e:
        return{"ERROR":"FAILED TO PARSE JSON FILE"}
    finally:
        json_lock.release()

@app.route('/readings/<string:Reading>', methods = ["GET"])
def return_readings_for_response(Reading):
    global json_path
    global json_lock
    json_lock.acquire()
    try:
        with open(json_path, "r") as f:
            data = json.load(f)
        for reading in data["Most recent readings"]:
            if reading["Reading"] == Reading:
                return jsonify({Reading:reading})
        return{"ERROR":"NO READINGS FOR THAT RESPONSE"}
    except Exception as e:
        return{"ERROR":"FAILED TO READ JSON FILE"}
    except json.JSONDecodeError as e:
        return{"ERROR":"FAILED TO PARSE JSON FILE"}
    finally:
        json_lock.release()


@app.route('/sensor_units/', methods = ["GET"])
def return_sensor_units():
    global json_path
    global json_lock
    json_lock.acquire()
    try:
        with open(json_path, "r") as f:
            data = json.load(f)
        return jsonify({"Sensor units": data["Sensor units"]})
    except Exception as e:
        return{"ERROR":"FAILED TO READ JSON FILE"}
    except json.JSONDecodeError as e:
        return{"ERROR":"FAILED TO PARSE JSON FILE"}
    finally:
        json_lock.release()


@app.route('/sensor_units/<int:sensor_unit_id>', methods = ["GET"])
def return_sensor_unit(sensor_unit_id):
    global json_path
    global json_lock
    json_lock.acquire()
    try:
        with open(json_path, "r") as f:
            data = json.load(f)
        for sensor_unit in data["Sensor units"]:
            if sensor_unit["ID"] == sensor_unit_id:
                return jsonify(sensor_unit)
        return{"ERROR":"NO SENSOR WITH THAT ID"}
    except Exception as e:
        return{"ERROR":"FAILED TO READ JSON FILE"}
    except json.JSONDecodeError:
        return{"ERROR":"FAILED TO PARSE JSON FILE"}
    finally:
        json_lock.release()

@app.route('/communication_units/', methods = ["GET"])
def return_communication_units():
    global json_path
    global json_lock
    json_lock.acquire()
    try:
        with open(json_path, "r") as f:
            data = json.load(f)
        return jsonify({"Communication units": data["Communication units"]})
    except Exception as e:
        return{"ERROR":"FAILED TO READ JSON FILE"}
    except json.JSONDecodeError as e:
        return{"ERROR":"FAILED TO PARSE JSON FILE"}
    finally:
        json_lock.release()

@app.route('communication_units/<string:send_command>', methods = ['POST'])
def send_push(send_command):
    global ser_port
    temp = send_command + "\n"
    ser_port.write(send_command.encode('utf-8'))
    return "OK"

#Only works when called directly for testing purposes
if __name__ == '__main__':
    ser_port = None
    lock = threading.Lock()
    app.run(debug=True)
