from flask import Flask, request, jsonify
import threading
import json
path = "api.json"
lock = None
ser_port = None

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


app=Flask(__name__)
@app.route('/')
def return_all():
    lock.acquire()
    with open(path, "r") as f:
        data = json.load(f)
    lock.release()
    if not isinstance(data, dict):
        print("Error: JSON file must contain a list of objects.")
        return {"error":"Invalid JSON file"}
    else:
        return jsonify(data)
    lock.release()


@app.route('/sensors', methods = ["GET"])
def return_sensors(lock):
    lock.acquire()
    with open(path, "r") as f:
        data = json.load(f)
    lock.release()
    if not isinstance(data, dict):
        print("Error: JSON file must contain a list of objects.")
        return {"error":"Invalid JSON file"}
    else:
        return jsonify(data["sensors"])

@app.route('/sensor/<int:sensor_id>', methods = ["GET"])
def return_sensor(sensor_ind):
    lock.acquire()
    with open(path, "r") as f:
        data = json.load(f)
    lock.release()
    if not isinstance(data, dict):
        print("Error: JSON file must contain a list of objects.")
        return {"error":"Invalid JSON file"}
    return jsonify(data["sensors"][sensor_ind])
#will return all readings for a sensor unit
@app.route('/readings/', methods = ["GET"])
def return_readings():
    lock.acquire()
    with open(path, "r") as f:
        data = json.load(f)
    lock.release()
    if not isinstance(data, dict):
        print("Error: JSON file must contain a list of objects.")
        return {"error":"Invalid JSON"}
    else:
        return jsonify(data["readings"])

@app.route('/readings/<int:sensor_id>', methods = ["GET"])
def return_readings_for_sensor(sensor_id):
    lock.acquire()
    with open(path, "r") as f:
        data = json.load(f)
    lock.release()
    if not isinstance(data, dict):
        print("Error: JSON file must contain a list of objects.")
        return {"error":"Invalid JSON"}
    else:
        for reading in data["readings"]:
            if reading["sensor_id"] == sensor_id:
                return jsonify(reading)
        return {"error":"Sensor unit not found"}

@app.route('/readings/<string:response>', methods = ["GET"])
def return_readings_for_response(response):
    lock.acquire()
    with open(path, "r") as f:
        data = json.load(f)
    lock.release()
    if not isinstance(data, dict):
        print("Error: JSON file must contain a list of objects.")
        return {"error":"Invalid JSON"}
    else:
        list_of_readings = []
        for reading in data["readings"]:
            if reading["response"] == response:
                list_of_readings.append(reading)
        json_obj = {"readings":list_of_readings}
        return jsonify(json_obj)


@app.route('/sensor_units/', methods = ["GET"])
def return_sensor_units():
    lock.acquire()
    with open(path, "r") as f:
        data = json.load(f)
    lock.release()
    if not isinstance(data, dict):
        print("Error: JSON file must contain a list of objects.")
        return {"error":"Invalid JSON"}
    else:
        return jsonify(data["sensor_units"])

@app.route('/sensor_units/<int:sensor_unit_id>', methods = ["GET"])
def return_sensor_unit(sensor_unit_id):
    lock.acquire()
    with open(path, "r") as f:
        data = json.load(f)
    lock.release()
    if not isinstance(data, dict):
        print("Error: JSON file must contain a list of objects.")
        return {"error":"Invalid JSON"}
    else:
        for sensor_unit in data["sensor_units"]:
            if sensor_unit["id"] == sensor_unit_id:
                return jsonify(sensor_unit)
        return {"error":"Sensor unit not found"}

@app.route('/communication_units/', methods = ["GET"])
def return_communication_units():
    lock.acquire()
    with open(path, "r") as f:
        data = json.load(f)
    lock.release()
    if not isinstance(data, dict):
        print("Error: JSON file must contain a list of objects.")
        return {"error":"Invalid JSON"}
    else:
        return jsonify(data["communication_units"])
#TODO implement sending push commands to sensor units

@app.route('/sensor_units/<int:sens_unit_id>/send_command/<string:command>', methods = ['POST'])
def send_push(command, sens_unit_id):
    if ser_port is None:
        return {"success":False}

    lock.acquire()
    with open(path, "r") as f:
        data = json.load(f)
    lock.release()
    for sensor in data["sensors"]:
        for cmd in sensor["commands"]:
            if command[0:3] == "PUSH":
                continue
            elif cmd == command:
                ser_port.write(command.encode('utf-8'))
                return {"success":True}
    return {"success":False}

#Only works when called directly for testing purposes
if __name__ == '__main__':
    ser_port = None
    lock = threading.Lock()
    app.run(debug=True)
