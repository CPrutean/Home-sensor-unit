#import serial
#import time
from datetime import time

import serial

pyKeywordArr = [["PULL", "PUSH"], ["TEMP AND HUMID", "GPS", "ALL"]]
pySensorCmds = [["TEMP", "HUMID", "ALL"], ["LAT AND LONG", "ALL"]]
pyStrSeperator = '|'
pyLineEnd = '\n'
sendMsgBool = False
ser = serial.Serial('/dev/ttyUSB0', 38400, timeout=1)
ser.flush()


def sendMsg(msg):
    try:
        msg_to_send = msg+'\n'
        ser.write(msg_to_send.encode('utf-8'))
    except serial.SerialException as e:
        print(e)
        print("Check if the device is connected")
    except KeyboardInterrupt:
        print("Exit")
    finally:
        #locals defines a variable that is in the local scope
        if 'ser' in locals():
            ser.close()


#infinite loop for receiving messages
while True:
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8')
        msg_recv_time = time.time
        msg_date = time.date