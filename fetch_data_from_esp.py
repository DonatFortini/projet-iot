import requests
import json
import serial
import sys 
import time

url = "http://172.20.10.3/data"
if sys.platform == "linux":
    try:
        port = "/dev/ttyUSB0"
    except:
        port = "/dev/ttyUSB0/"
else:
    port = "COM4"

baudrate = 115200

def fetch_data(url_param: str):
    try:
        while True:
            response = requests.get(url_param)
            results = response.json()
            print(results)
    except Exception as e:
        print(str(e))

def write_data():
    try:
        with serial.Serial(port, baudrate, timeout=1) as ser:
            while ser.isOpen():
                try:
                    ser.write()
                except Exception as e:
                    print(str(e))
    except:
        print("pas de port serial")

def main():
    fetch_data(url)
    write_data()    
    time.sleep(0.5)

if __name__ == "__main__":
    main()