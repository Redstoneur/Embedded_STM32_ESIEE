import asyncio
import json
import os
import sys
import termios
from tarfile import version

import paho.mqtt.client as mqtt
import serial
import uvicorn
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from CapteurInformation import CapteurInformation
from LedRGB import LedRGB

# ---------------- Configuration ----------------

# Serial port configuration (adapted to Rx/Tx pins)
SERIAL_PORT = os.getenv("SERIAL_PORT", "/dev/serial0")
BAUD_RATE = int(os.getenv("BAUD_RATE", 115200))

# MQTT configuration
MQTT_BROKER = os.getenv("MQTT_BROKER", "test.mosquitto.org")  # MQTT broker
MQTT_PORT = int(os.getenv("MQTT_PORT", 1883))
MQTT_TOPIC = os.getenv("MQTT_TOPIC", "rfid/text")  # topic to publish

# API configuration
API_HOST = os.getenv("API_HOST", "0.0.0.0")
API_PORT = int(os.getenv("API_PORT", 8000))

# ---------------- Variables globales ----------------

# Dictionary to store sensor information
capteurInformation: CapteurInformation = CapteurInformation(
    Temperature=0.0,
    Humidity=0.0,
    RGB=LedRGB(Red=0, Green=0, Blue=0, State=False),
    Led=False,
    Buzzer=False,
    Button=False
)

# Dictionary to define the type of sensor information
typeCapteurInformation: dict = {
    "Temperature": float,
    "Humidity": float,
    "RGB": {
        "red": int,
        "green": int,
        "blue": int,
        "state": bool
    },
    "Led": bool,
    "Buzzer": bool,
    "Button": bool
}

commandeBuffer: list[str] = []

# ---------------- Initialisation MQTT ----------------

# Création du client MQTT
mqtt_client = mqtt.Client()


def connect_mqtt() -> bool:
    """
    Connect to the MQTT broker.
    :return: True if connected, False otherwise
    """
    try:
        mqtt_client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
        mqtt_client.loop_start()
        return True
    except Exception as e:
        print(f"Error while connecting to MQTT : {e}")
        return False


# Connect to the MQTT broker
mqtt_connected = connect_mqtt()


def send_mqtt(value) -> None:
    """
    Publish a message on the specified MQTT topic.
    :param value: message to publish
    :return: None
    """
    if mqtt_connected:
        try:
            mqtt_client.publish(MQTT_TOPIC, payload=value)
            print(f"Published on MQTT : {value}")
        except Exception as e:
            print(f"Error while publishing on MQTT : {e}")


# ---------------- Fonction de lecture sur le port série ----------------

def validate_and_update(data, type_info, values):
    for key, value in data.items():
        if key in type_info.keys():
            if isinstance(value, dict) and isinstance(type_info[key], dict):
                if key not in values:
                    values[key] = {}
                try:
                    validate_and_update(value, type_info[key], values[key])
                except ValueError as e:
                    print(f"Erreur de validation pour la clé : {key} : {e}")
            elif isinstance(value, type_info[key]):
                values[key] = value
            else:
                raise ValueError(f"Type invalide pour la clé : {key}")
        else:
            raise ValueError(f"Clé invalide : {key}")


def save_data(data: dict):
    global typeCapteurInformation
    global capteurInformation

    values = {}

    # vérifier si la trame contient les informations des capteurs
    if not all(key in data for key in typeCapteurInformation.keys()):
        raise ValueError("La trame ne contient pas toutes les informations des capteurs.")

    # Vérifier si la trame contient les informations des capteurs valides et
    # sauvegarder dans values
    validate_and_update(data, typeCapteurInformation, values)

    # Mettre à jour le dictionnaire des capteurs
    capteurInformation = CapteurInformation.from_dict(values)


async def serial_controller():
    """
    Lit en continu une trame sur le port série et la convertit en texte hexadécimal.
    Met à jour la variable globale 'latest_text' et publie la trame sur MQTT.
    """

    global commandeBuffer

    latest_text: str = ""
    init: bool = True
    ser: serial.Serial = None

    while True:
        if init:
            try:
                ser = serial.Serial(
                    port=SERIAL_PORT,
                    baudrate=BAUD_RATE,
                    parity=serial.PARITY_NONE,
                    stopbits=serial.STOPBITS_ONE,
                    bytesize=serial.EIGHTBITS,
                    timeout=1
                )
            except serial.SerialException as e:
                print(f"Erreur d'ouverture du port série : {e}")
                sys.exit(1)
            init = False

        try:
            try:
                ser.flush()  # Vider le buffer d'entrée
            except termios.error as e:
                print(f"Erreur de flush du port série : {e}")
            line = ser.readline()  # Lire une ligne sur le port série
            if line:
                # Conversion de la trame en chaîne hexadécimale en majuscules
                # On peut utiliser la méthode .hex() pour simplifier
                trame = line.hex().upper()

                # Affichage de la trame hexadécimale
                print(f"Trame reçue : {trame}")

                # Décode la trame hexadécimale en texte
                data = bytes.fromhex(trame).decode('utf-8', errors='ignore')

                # Affichage de la trame décodée
                print(f"Texte décodé : {data}")

                if data == latest_text or data in ["", "\r\n", "\n", "\r"]:
                    continue

                latest_text = data
                print(f"Dernière trame : {latest_text}")

                # vérifier si la trame n'est pas un int
                if latest_text.isdigit():
                    print(f"La trame est un entier : {latest_text}")
                    continue

                # verifier sur la trame est bien un texte JSON
                try:
                    tempCapteurInformation = json.loads(latest_text)
                    print(f"Capteur Information : {tempCapteurInformation}")
                except json.JSONDecodeError as e:
                    print(f"Erreur de décodage JSON : {e}")
                    continue

                # Sauvegarde des données dans le dictionnaire des capteurs
                try:
                    save_data(tempCapteurInformation)
                except ValueError as e:
                    print(f"Erreur de sauvegarde des données : {e}")
                    continue

                # Publication immédiate sur le topic MQTT
                send_mqtt(latest_text)

        except serial.SerialException as e:
            print(f"Erreur de lecture sur le port série : {e}")
            ser.close()
            init = True

        try:
            if len(commandeBuffer) > 0:
                commande = commandeBuffer.pop(0)
                ser.write(commande.encode())
                print(f"Commande envoyée : {commande}")
        except serial.SerialException as e:
            print(f"Erreur d'écriture sur le port série : {e}")
            ser.close()
            init = True

        await asyncio.sleep(1)


# ---------------- API FastAPI ----------------

app = FastAPI(
    title="API de récupération de trame",
    description="API pour récupérer la dernière trame reçue depuis le port série.",
    version="1.0.0",
    docs_url="/docs",  # URL pour Swagger UI
    redoc_url="/redoc"  # URL pour ReDoc
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Permettre toutes les origines, à ajuster selon vos besoins
    allow_credentials=True,
    allow_methods=["*"],  # Permettre toutes les méthodes HTTP
    allow_headers=["*"],  # Permettre tous les en-têtes
)


@app.get("/")
async def get_latest_text():
    """
    Hello World
    """
    return "Hello World"


@app.get("/capteur")
async def get_capteur() -> CapteurInformation:
    """
    Renvoie les informations du capteur.
    """
    return capteurInformation


@app.get("/capteur/temperature")
async def get_temperature() -> float:
    """
    Renvoie la température du capteur.
    """
    return capteurInformation.Temperature


@app.get("/capteur/humidity")
async def get_humidity() -> float:
    """
    Renvoie l'humidité du capteur.
    """
    return capteurInformation.Humidity


@app.get("/capteur/rgb")
async def get_rgb() -> LedRGB:
    """
    Renvoie l'état de la LED RGB du capteur.
    """
    return capteurInformation.RGB


@app.get("/capteur/led")
async def get_led() -> bool:
    """
    Renvoie l'état de la LED du capteur.
    """
    return capteurInformation.Led


@app.get("/capteur/buzzer")
async def get_buzzer() -> bool:
    """
    Renvoie l'état du buzzer du capteur.
    """
    return capteurInformation.Buzzer


@app.get("/capteur/button")
async def get_button() -> bool:
    """
    Renvoie l'état du bouton du capteur.
    """
    return capteurInformation.Button


@app.post("/capteur/buzzer")
async def post_buzzer(state: bool) -> dict:
    """
    Active ou désactive le buzzer du capteur.
    """
    global commandeBuffer
    commandeBuffer.append(f"[BUZZER#SWITCH:{state}]\n")
    return {"buzzer": state}


@app.post("/capteur/led")
async def post_led(state: bool) -> dict:
    """
    Active ou désactive la LED du capteur.
    """
    global commandeBuffer
    commandeBuffer.append(f"[LED#SWITCH:{state}]\n")
    return {"led": state}


@app.post("/capteur/rgb/color")
async def post_rgb_color(red: int, green: int, blue: int) -> dict:
    """
    Change la couleur de la LED RGB du capteur.
    """
    global commandeBuffer
    commandeBuffer.append(f"[RGB#COLOR:{red},{green},{blue}]\n")
    return {"red": red, "green": green, "blue": blue}


@app.post("/capteur/rgb/switch")
async def post_rgb_switch(state: bool) -> dict:
    """
    Active ou désactive la LED RGB du capteur.
    """
    global commandeBuffer
    commandeBuffer.append(f"[RGB#SWITCH:{state}]\n")
    return {"state": state}


# ---------------- Exécution de l'API ----------------

loop = asyncio.get_event_loop()
loop.create_task(serial_controller())

if __name__ == "__main__":
    uvicorn.run(app, host=API_HOST, port=API_PORT)
