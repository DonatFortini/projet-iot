### Contrôleur de Pointeur Laser via Caméra

Ce projet IoT implique la création d'un serveur avec une API REST hébergée localement sur l'ESP32-CAM, qui intègre également un modele de TinyML pour la reconnaissance d'objets (humains dans ce cas). Ce modele est généré avec le framework Edge Impulse. L'objectif principal du projet est que lorsque le modèle détecte un humain, il sérialise la bounding box et la stocke sur le serveur pour qu'elle puisse être récupérée par l'API. Ensuite, un ordinateur connecté à l'Arduino pourra exécuter `fetch_data_from_esp.py` pour récupérer les données de l'API et les transformer afin d'orienter la trajectoire du pointeur laser.

#### Installation

```shell
python3 -m venv iot_venv
source iot_venv/bin/activate
pip install -r requirement.txt
```

#### Matériel Requis

##### Partie Pointeur Laser
- Arduino Uno R3
- Module Laser Rouge VMA434
- 2 Servomoteurs Classiques

##### Partie Caméra
- ESP32-CAM (AI THINKER ESP32CAM)

#### Démo et photo



### Description de l'IOT 
Notre IOT est composé d'une carte Arduino, d'une carte ESP32 codée en C, une tourelle, d'un Joystick et d'un PC. Au niveau des capteurs il est composé d'une caméra sur laquelle tourne un algorithme de machine learning et d'un joystick. 
Le Joystick permet de controler la tourelle et de pointer le laser sur une personne manuellement. 

### Fonctionnement général de L'IOT
Lorsque la caméra detecte, grâce à un modèle d'apprentissage, une personne, la tourelle pivote et pointe un rayon laser la personne détectée. Un mode manuel est disponible en appuyant sur le joystick.

### Fonctionnement détaillé de l'IOT
la carte Arduino est connecté en série au bloc caméra (capteur Caméra + ESP32), c'est cette carte qui va exposer une API REST qui permet de récuperées les données de détection de la caméra.
Le PC recuppere les données de bounding box du json en utilisant la méthode GET de l'API. Le PC calcule l'angle qui va permettre à la tourelle de viser la personne détectée.
Le PC envoie ensuite via l'API l'angle à la caméra, celle-ci envoie en série cette donnée à la carte Arduino, cette dernière connectée la tourelle, va faire pivoter la tourelle ce qui va permettre de viser la personne détectée. 
Enfin le PC envoie, grâce à une librarie de client MQTT, les données au Broker MQTT. 







