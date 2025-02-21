# Utiliser une image de base Node.js
FROM node:22

# Définir le répertoire de travail
WORKDIR /app

# Copier les fichiers de l'application
COPY . /app

# Installer les dépendances
RUN npm install

# Construire l'application
RUN npm run build

# Installer un serveur HTTP pour servir les fichiers statiques
RUN npm install -g serve

# Exposer le port pour l'application Vue.js
EXPOSE 8080

# Commande pour démarrer le serveur HTTP
CMD ["serve", "-s", "dist"]