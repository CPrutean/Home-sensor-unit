import os
import sqlite3
import threading
from dotenv import load_dotenv
import os

load_dotenv()
database_path = os.getenv("DATABASE_PATH")
db_lock = threading.Lock()


def read(target_obj):
    return


def write(target_obj):
    return

def init():
    #All values which need a list will be seperated by commas
    default_tables = ["""CREATE TABLE IF NOT EXISTS sensor (sensor_id INTEGER UNIQUE NOT NULL, responses TEXT, requests TEXT)""",
                      """CREATE TABLE IF NOT EXISTS sensor_unit (id INTEGER UNIQUE NOT NULL, name TEXT, sensors TEXT)""",
                      """CREATE TABLE IF NOT EXISTS communication_unit (id INTEGER unique NOT NULL, name TEXT, sensor_units TEXT)""",
                      """CREATE TABLE IF NOT EXISTS most_recent_reading (sensor_name TEXT, reading TEXT, values TEXT, timestamp TEXT)"""]

    try:
        with sqlite3.connect(database_path) as conn:
            for command in default_tables:
                conn.execute(command)
    except sqlite3.Error:
        print("Error: Unable to init database.")

def sanitize():
    return

    return
