import os
import sys
import threading
import time

import paho.mqtt.client as mqtt
import serial
import uvicorn
from fastapi import FastAPI

# ---------------- Configuration ----------------

# Configuration du port série (adapté aux broches Rx/Tx)
SERIAL_PORT = os.getenv("SERIAL_PORT", "/dev/serial0")
BAUD_RATE = int(os.getenv("BAUD_RATE", 115200))

# Configuration MQTT
MQTT_BROKER = os.getenv("MQTT_BROKER", "test.mosquitto.org")  # adresse du serveur MQTT
MQTT_PORT = int(os.getenv("MQTT_PORT", 1883))
MQTT_TOPIC = os.getenv("MQTT_TOPIC", "rfid/text")  # topic sur lequel publier la trame

# Configuration de l'API FastAPI
API_HOST = os.getenv("API_HOST", "0.0.0.0")
API_PORT = int(os.getenv("API_PORT", 8000))

# Variable globale pour stocker la dernière trame reçue
latest_text = ""

# ---------------- Initialisation MQTT ----------------

# Création du client MQTT
mqtt_client = mqtt.Client()


def connect_mqtt():
    try:
        mqtt_client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
        mqtt_client.loop_start()  # Démarrer la boucle MQTT en tâche de fond
        return True
    except Exception as e:
        print(f"Erreur de connexion au serveur MQTT : {e}")
        return False


mqtt_connected = connect_mqtt()


# ---------------- Fonction de lecture sur le port série ----------------

def read_serial():
    """
    Lit en continu une trame sur le port série et la convertit en texte hexadécimal.
    Met à jour la variable globale 'latest_text' et publie la trame sur MQTT.
    """
    global latest_text
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

    while True:
        try:
            ser.flush()  # Nettoyer le buffer
            line = ser.readline()  # Lire une ligne sur le port série
            if line and line != "0A":
                # Conversion de la trame en chaîne hexadécimale en majuscules
                # On peut utiliser la méthode .hex() pour simplifier
                data = line.hex().upper()

                # sauvegarde de la trame hexadécimale décodée en texte
                latest_text = bytes.fromhex(data).decode('utf-8')

                # Affichage de la trame reçue et du texte décodé
                print(f"Trame reçue : {data}")
                print(f"Texte décodé : {latest_text}")

                # Publication immédiate sur le topic MQTT
                if mqtt_connected:
                    try:
                        mqtt_client.publish(MQTT_TOPIC, payload=data)
                    except Exception as e:
                        print(f"Erreur de publication sur MQTT : {e}")

        except serial.SerialException as e:
            print(f"Erreur de lecture sur le port série : {e}")
            continue

        time.sleep(0.1)


# Démarrer la lecture du port série dans un thread séparé
serial_thread = threading.Thread(target=read_serial, daemon=True)
serial_thread.start()

# ---------------- API FastAPI ----------------

app = FastAPI(title="API de récupération de trame")


@app.get("/latest")
def get_latest_text():
    """
    Renvoie la dernière trame textuelle reçue depuis le port série.
    """
    return {"latest_text": latest_text}


# ---------------- Exécution de l'API ----------------

if __name__ == "__main__":
    uvicorn.run(app, host=API_HOST, port=API_PORT)
