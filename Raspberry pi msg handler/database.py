import os
import sqlite3
import threading
from dotenv import load_dotenv
import os

load_dotenv()
database_path = os.getenv("DATABASE_PATH")
db_lock = threading.Lock()
insert_values = ["sensor", "sensor_unit", "communication_unit", "most_recent_reading"]



def read(target_obj):
    if (target_obj not in insert_values):
        print("Error: target_obj wasnt in insert_values")
        return

    return

#Commas will separate all values that are passed and are lists
def write(target_obj, write_list, update = False):
    if (target_obj not in insert_values):
        print("Error: target_obj wasnt in insert_values")
        return

    global db_lock
    db_lock.acquire()
    with sqlite3.connect(database_path) as conn:
        print("balls")

    
    db_lock.release()
    conn.close()


def init():
    default_tables = ["""CREATE TABLE IF NOT EXISTS sensor (name TEXT, responses TEXT, requests TEXT)""",
                      """CREATE TABLE IF NOT EXISTS sensor_unit (id INTEGER UNIQUE NOT NULL, name TEXT, sensors TEXT)""",
                      """CREATE TABLE IF NOT EXISTS communication_unit (id INTEGER unique NOT NULL, name TEXT, sensor_units TEXT)""",
                      """CREATE TABLE IF NOT EXISTS most_recent_reading (sensor_name TEXT, reading TEXT, values TEXT, timestamp TEXT)"""]
    global database_path

    try:
        with sqlite3.connect(database_path) as conn:
            for command in default_tables:
                conn.execute(command)
        conn.close()
    except sqlite3.Error:
        print("Error: Unable to init database.")


def sanitize(input):
    return

