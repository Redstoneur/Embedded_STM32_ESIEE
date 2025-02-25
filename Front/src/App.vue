<script setup lang="ts">
import { onMounted, ref } from "vue";
import off from "./assets/Switch_OFF.webp";
import on from "./assets/Switch_ON.webp";

const apiUrl = "http://192.168.170.90:8000/capteur";

const redValue = ref<number>(0);
const greenValue = ref<number>(0);
const blueValue = ref<number>(0);

interface typeCapteur {
  Humidity: number;
  Temperature: number;
  Buzzer: boolean;
  Led: boolean;
  RGB: {
    Blue: number;
    Green: number;
    Red: number;
    State: boolean;
  };
}

const capteurs = ref<typeCapteur | null>(null);

const fetchData = async (): Promise<void> => {
  try {
    const response = await fetch(apiUrl);
    if (!response.ok)
      throw new Error("Erreur lors de la récupération des données");
    capteurs.value = await response.json();

    // Initialiser les valeurs locales avec celles récupérées de l'API
    if (capteurs.value) {
      redValue.value = capteurs.value.RGB.Red;
      greenValue.value = capteurs.value.RGB.Green;
      blueValue.value = capteurs.value.RGB.Blue;
    }
  } catch (err) {
    console.error("Erreur:", err);
  }
};

const toggleBuzzer = async (): Promise<void> => {
  if (!capteurs.value) return;

  const newState = !capteurs.value.Buzzer; // Inverser l'état actuel

  try {
    const response = await fetch(`${apiUrl}/buzzer?state=${newState}`, {
      method: "PUT",
      headers: {
        Accept: "application/json",
      },
    });

    if (!response.ok) {
      const errorMessage = await response.text(); // Récupérer le message d'erreur de l'API
      throw new Error(`Erreur API: ${errorMessage}`);
    }
  } catch (err) {
    console.error("Erreur:", err);
  }
};
// 'http://192.168.170.90:8000/capteur/led?state=true' \

const toggleRadiateur = async (): Promise<void> => {
  if (!capteurs.value) return;

  const newState = !capteurs.value.Led; // Inverser l'état actuel

  try {
    const response = await fetch(`${apiUrl}/led?state=${newState}`, {
      method: "PUT",
      headers: {
        Accept: "application/json",
      },
    });

    if (!response.ok) {
      const errorMessage = await response.text(); // Récupérer le message d'erreur de l'API
      throw new Error(`Erreur API: ${errorMessage}`);
    }
  } catch (err) {
    console.error("Erreur:", err);
  }
};
const toggleSwitch = async (): Promise<void> => {
  if (!capteurs.value) return;

  const newState = !capteurs.value.RGB.State; // Inverser l'état actuel

  try {
    const response = await fetch(`${apiUrl}/rgb/switch?state=${newState}`, {
      method: "PUT",
      headers: {
        Accept: "application/json",
      },
    });

    if (!response.ok) {
      const errorMessage = await response.text(); // Récupérer le message d'erreur de l'API
      throw new Error(`Erreur API: ${errorMessage}`);
    }
  } catch (err) {
    console.error("Erreur:", err);
  }
};

const sendCapteurValue = async (): Promise<void> => {
  if (!capteurs.value?.RGB) return;

  try {
    const response = await fetch(
      `${apiUrl}/rgb/color?red=${redValue.value}&green=${greenValue.value}&blue=${blueValue.value}`,
      {
        method: "PUT",
        headers: {
          Accept: "application/json",
        },
      }
    );

    if (!response.ok) {
      const errorMessage = await response.text(); // Récupérer le message d'erreur de l'API
      throw new Error(`Erreur API: ${errorMessage}`);
    }
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
        <img :src="capteurs.Buzzer ? on : off" />
        <button class="buttton-buzzer" @click="toggleBuzzer">
          {{ capteurs.Buzzer ? "Éteindre" : "Allumer" }}
        </button>
      </div>

      <!-- radiateur -->
      <div class="buzzer-container">
        <img src="./assets/Radiateur.webp" />
        <img :src="capteurs.Led ? on : off" />
        <button class="buttton-buzzer" @click="toggleRadiateur">
          {{ capteurs.Led ? "Éteindre" : "Allumer" }}
        </button>
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
            <input type="number" min="0" max="255" v-model="redValue" />
          </div>

          <div class="rgb-item">
            <img src="./assets/LED_verte.webp" />
            <input type="number" min="0" max="255" v-model="greenValue" />
          </div>

          <div class="rgb-item">
            <img src="./assets/LED_bleue.webp" />
            <input type="number" min="0" max="255" v-model="blueValue" />
          </div>
          <button @click="sendCapteurValue">↻</button>
        </div>
      </div>
      <img class="image-rbd" :src="capteurs?.RGB?.State ? on : off" />
      <button @click="toggleSwitch">
        {{ capteurs.RGB.State ? "Éteindre" : "Allumer" }}
      </button>
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
.buzzer-container .buttton-buzzer {
  border: none;
  font-size: 16px;
  cursor: pointer;
  width: 100%;
  background: gray;
}
.image-rbd {
  width: 90px;
  height: 90px;
  object-fit: contain;
}
</style>
