<script setup lang="ts">
import { onMounted, ref } from "vue";
import off from "./assets/Switch_OFF.webp";
import on from "./assets/Switch_ON.webp";

const apiUrl = "http://192.168.170.90:8000/capteur";

interface typeCapteur {
  Humidity: number;
  Temperature: number;
  Buzzer: boolean;
  RGB: {
    Blue: number;
    Green: number;
    Red: number;
  };
}

const capteurs = ref<typeCapteur | null>(null);

const fetchData = async (): Promise<void> => {
  try {
    const response = await fetch(apiUrl);
    if (!response.ok)
      throw new Error("Erreur lors de la récupération des données");
    capteurs.value = await response.json();
  } catch (err) {
    console.error("Erreur:", err);
  }
};

onMounted(fetchData);
</script>

<template>
  <div>
    <h1>Projet Système Embarqué</h1>

    <div v-if="capteurs" class="content">
      <!-- Buzzer -->
      <div class="buzzer-container">
        <img src="./assets/Buzzer.webp" />
        <button
          :class="{ buzzeColor: capteurs.Buzzer, dark: !capteurs.Buzzer }"
        ></button>
        <img :src="capteurs.Buzzer ? on : off" />
      </div>

      <!-- Température -->
      <div>
        Température: <button>{{ capteurs.Temperature }}°C</button>
      </div>

      <!-- Humidité -->
      <div>
        <img src="./assets/Humidity_icon.webp" />
        <button>{{ capteurs.Humidity }}%</button>
      </div>

      <!-- RGB -->
      <div class="rgb-content">
        <span>RGB</span>
        <div>
          <div class="rgb-item">
            <img src="./assets/LED_rouge.webp" />
            <input type="number" min="1" max="255" v-model="capteurs.RGB.Red" />
          </div>

          <div class="rgb-item">
            <img src="./assets/LED_verte.webp" />
            <input
              type="number"
              min="1"
              max="255"
              v-model="capteurs.RGB.Green"
            />
          </div>

          <div class="rgb-item">
            <img src="./assets/LED_bleue.webp" />
            <input
              type="number"
              min="1"
              max="255"
              v-model="capteurs.RGB.Blue"
            />
          </div>
        </div>
      </div>
      <img
        class="image-rbd"
        :src="
          capteurs?.RGB?.Blue < 1 &&
          capteurs?.RGB.Green < 1 &&
          capteurs?.RGB.Red < 1
            ? off
            : on
        "
      />
    </div>
  </div>
</template>

<style scoped>
.content {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 20px;
}

h1 {
  text-align: center;
  font-size: 24px;
  color: #333;
}

button {
  padding: 10px 20px;
  border: none;
  border-radius: 8px;
  font-size: 16px;
  cursor: pointer;
}

.buzzeColor {
  background: darkorange;
  color: white;
}

.dark {
  background: black;
  color: white;
}

img {
  width: 50px;
  height: 50px;
  object-fit: contain;
}

/* Correction de l'affichage des LED RGB */
.rgb-content {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 10px;
}

.rgb-content div {
  display: flex;
  justify-content: center;
  gap: 20px;
}

.rgb-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 5px;
}

.rgb-item input {
  width: 60px;
  text-align: center;
  font-size: 16px;
  padding: 5px;
  border: 1px solid #ccc;
  border-radius: 5px;
}
.buzzer-container {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 15px;
}

.buzzer-container img {
  width: 90px;
  height: 90px;
  object-fit: contain;
}

.buzzer-container button {
  width: 50px;
  height: 50px;
  border: none;
  border-radius: 50%;
  cursor: pointer;
}
.image-rbd {
  width: 90px;
  height: 90px;
  object-fit: contain;
}
</style>
