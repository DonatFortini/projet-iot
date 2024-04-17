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




