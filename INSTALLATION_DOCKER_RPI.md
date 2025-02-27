# 🐳 **Installation de Docker sur Raspberry Pi 4**  

## **1️⃣ Mise à jour du système**  
Avant toute installation, commence par mettre à jour les paquets existants :  
```bash
sudo apt update && sudo apt upgrade -y
```

## **2️⃣ Installation des dépendances nécessaires**  
Docker nécessite quelques paquets pour fonctionner correctement :  
```bash
sudo apt install apt-transport-https ca-certificates curl software-properties-common -y
```

## **3️⃣ Ajout de la clé GPG et du dépôt officiel Docker**  
Ajoute la clé GPG de Docker pour vérifier l’intégrité des paquets :  
```bash
curl -fsSL https://download.docker.com/linux/debian/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
```
Puis ajoute le dépôt Docker correspondant à ton architecture :  
```bash
echo "deb [arch=arm64 signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/debian $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
```

## **4️⃣ Mise à jour des sources et installation de Docker**  
Recharge la liste des paquets et utilise le script officiel pour installer Docker :  
```bash
sudo apt update
curl -fsSL https://get.docker.com | sudo sh
```

## **5️⃣ Ajout de l’utilisateur au groupe Docker**  
Cela permet d’utiliser Docker sans sudo :  
```bash
sudo usermod -aG docker $USER
newgrp docker
```

## **6️⃣ Vérification de l’installation**  
Assure-toi que Docker et Docker Compose sont bien installés :  
```bash
docker --version
docker compose version
```

---

## ✅ **Docker est maintenant prêt à l’emploi sur ton Raspberry Pi 4 !**  

Tu peux maintenant utiliser Docker et Docker Compose pour gérer tes conteneurs.