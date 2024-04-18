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

url_fetch_data = "http://10.3.141.251/MLData"
url_set_position_canon = "http://10.3.141.251/set"

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

def calculer_angles(coordonnees_pixel: tuple, fov_horizontal: int, fov_vertical: int, distance_focale: int, resolution_image: tuple) -> tuple:
    """calcul les angles du canon avec les coordonnées d'un pixel sur l'écran de la caméra (ou du flux vidéo de la caméra)

    Args:
        coordonnees_pixel (tuple): les coordonnées du pixel
        fov_horizontal (int): champ de vision horizontal de la caméra
        fov_vertical (int): champ de vision vertical de la caméra
        distance_focale (int): distance focale de la caméra
        resolution_image (tuple): résolution de l'image de la caméra, exemple: (1920, 1080)

    Returns:
        tuple: _description_
    """
    u, v = coordonnees_pixel
    centre_image = (resolution_image[0] / 2, resolution_image[1] / 2)

    theta_rad = np.arctan2(
        u - centre_image[0], 
        distance_focale * fov_horizontal / resolution_image[0])
    
    theta = np.degrees(theta_rad)
    theta = 180 if theta < 0 else theta

    phi_rad = np.arctan2(
        v - centre_image[1], 
        distance_focale * fov_vertical / resolution_image[1])
    phi = np.degrees(phi_rad)
    phi = 180 if phi < 0 else phi

    print("Theta = ", theta*0.36)
    print("Phi = ", phi*0.36)
    print("Ratio horizontal = ", fov_horizontal/180)
    print("Ratio vertical = ", fov_vertical/180)
    a = theta * (fov_horizontal/180)
    b = phi * (fov_vertical/180)
    print(a, b)

    return(a, b)


def main():
    temps_initial = time.time()
    while True:

        results = fetch_data(url_fetch_data)
        print(results)
        temps_actuel = time.time()
        temps_ecoule = temps_actuel - temps_initial
        if temps_ecoule > 10:
            print("envoie message mqtt")
            client = connect_mqtt()
            client.publish(MQTT_TOPIC, str(results))
        # write_data()

if __name__ == "__main__":
    main()