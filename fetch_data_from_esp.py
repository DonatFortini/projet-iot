import requests
import json
import serial
import sys 
import time
import numpy as np

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
    angle_hor = theta * (fov_horizontal/180)
    angle_ver = phi * (fov_vertical/180)
    print(int(angle_hor + 90), int(angle_ver + 90))

    return int(angle_hor), int(angle_ver)

def main():
    #fetch_data(url)
    #write_data()
    #time.sleep(0.5)
    coordonnees_pixel = (1920/2, 1)
    fov_hor = 65
    fov_ver = 19
    distance_focale = 0.21
    resolution_image = (1920, 1080)
    print(coordonnees_pixel, fov_hor, fov_ver, distance_focale, resolution_image)
    calculer_angles(coordonnees_pixel, fov_hor, fov_ver, distance_focale, resolution_image)

if __name__ == "__main__":
    main()