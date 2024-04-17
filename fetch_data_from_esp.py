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

def main():
    try:
        response = requests.get(url)
        results = response.json()
        print(results)
    except Exception as e:
        print(str(e))

    with serial.Serial(port, baudrate, timeout=1) as ser:
        while ser.isOpen():
            try:
                ser.write()
            except Exception as e:
                print(str(e))
            time.sleep(1)

if __name__ == "__main__":
    main()