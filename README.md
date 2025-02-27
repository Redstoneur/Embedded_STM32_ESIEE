# Embedded_STM32_ESIEE

## Description du projet

Ce projet est un système embarqué qui permet de contrôler des actionneurs (LED RGB, relais, buzzer actif) et de lire des
capteurs (température/humidité) à distance via une interface web. Le système embarqué est composé d'une carte de
développement STM32 (F446RE NUCLEO) et d'une Raspberry Pi 4 qui sert de serveur pour l'interface web. Le serveur est
exécuté dans un conteneur Docker sur la Raspberry Pi 4.

## Matériel nécessaire

- Serveur et interface vers la carte de développement : Raspberry Pi 4
- Carte de développement utilisée : STM32 - F446RE NUCLEO
- LED RGB
- Capteur de température/humidité
- Relais (identifié par le nom LED dans le code)
- Buzzer actif
- Câbles de connexion (USB, GPIO)
- Ordinateur avec un environnement de développement intégré (IDE) compatible (par exemple, PyCharm, STM32CubeIDE)

## Prérequis

- Installer Docker et Docker Compose sur la Raspberry Pi 4 pour exécuter le serveur (
  voir [INSTALLATION_DOCKER_RPI.md](INSTALLATION_DOCKER_RPI.md) pour les instructions)
- Installer les outils de développement STM32 (STM32CubeMX, STM32CubeIDE) sur un ordinateur pour configurer et générer
  le code de base pour la carte STM32

## Utilisation du projet

1. Cloner le dépôt du projet sur l'ordinateur avec les outils de développement STM32 et le serveur Raspberry Pi 4.

```bash
git clone https://github.com/Redstoneur/Embedded_STM32_ESIEE.git
cd Embedded_STM32_ESIEE
```

2. Configurer le projet du Dossier STM32CubeMX pour générer le code de base pour la carte STM32.

4. Compiler et flasher le code sur la carte STM32.

5. Connecter les capteurs et actionneurs (LED RGB, capteur de température/humidité, relais, buzzer actif) à la carte
   STM32 et au Raspberry Pi 4.

6. Une fois le code flashé et les capteurs/actionneurs connectés, et la comunications Rx/Tx configurées, exécuter le
   serveur sur la Raspberry Pi 4 via Docker Compose.

## Licence

Ce projet est sous licence MIT. Voir le fichier `LICENSE` pour plus de détails.
