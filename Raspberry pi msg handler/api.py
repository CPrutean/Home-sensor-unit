from flask import Flask, request, jsonify
import threading

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
    return


@app.route('/sensors', methods = ["GET"])
def return_sensors(lock):
    return

@app.route('/sensor/<int:sensor_id>', methods = ["GET"])
def return_sensor(sensor_ind):
    return

#will return all readings for a sensor unit
@app.route('/readings/', methods = ["GET"])
def return_readings():
    return

@app.route('/readings/<int:sensor_id>', methods = ["GET"])
def return_readings_for_sensor(sensor_id):
    return

@app.route('/readings/<string:response>', methods = ["GET"])
def return_readings_for_response(response):
    return


@app.route('/sensor_units/', methods = ["GET"])
def return_sensor_units():
    return

@app.route('/sensor_units/<int:sensor_unit_id>', methods = ["GET"])
def return_sensor_unit(sensor_unit_id):
    return

@app.route('/communication_units/', methods = ["GET"])
def return_communication_units():
    return

@app.route('/sensor_units/<int:sens_unit_id>/send_command/<string:command>', methods = ['POST'])
def send_push(command, sens_unit_id):
    return

#Only works when called directly for testing purposes
if __name__ == '__main__':
    ser_port = None
    lock = threading.Lock()
    app.run(debug=True)
