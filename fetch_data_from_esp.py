import random

import requests
import json
import serial
import sys 
import time
import paho.mqtt.client as mqtt_client
import ssl

MQTT_CLIENT_ID = f'publish-{random.randint(0, 1000)}'
MQTT_BROKER = "10.3.141.1"
MQTT_USER = "pi-corte"
MQTT_PASSWORD = "pi-corte"
MQTT_TOPIC = "iot/canon"
MQTT_PORT = 1883


url = "http://172.20.10.3/MLData"
if sys.platform == "linux":
    try:
        port = "/dev/ttyUSB0"
    except:
        port = "/dev/ttyUSB0/"
else:
    port = "COM4"

baudrate = 115200
def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(MQTT_CLIENT_ID)
    client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    client.on_connect = on_connect
    client.connect(MQTT_BROKER, MQTT_PORT)
    return client

def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")

    client.subscribe(MQTT_TOPIC)
    client.on_message = on_message




def fetch_data(url_param: str):
    try:
        while True:
            response = requests.get(url_param)
            results = response.json()
            return results
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


def sendDataToMqttBroker():
    print()


def main():
    # results = fetch_data(url)
    # print(results)
    client = connect_mqtt()
    client.publish(MQTT_TOPIC, "ilyas")
    # write_data()
    time.sleep(0.5)

if __name__ == "__main__":
    main()