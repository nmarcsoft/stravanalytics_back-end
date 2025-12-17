
<script setup>
import { onMounted } from "vue"
import Chart from "chart.js/auto"

onMounted(async () => {
  // 1️⃣ Récupération des données
  const res = await fetch("http://localhost:8080/api/activities")
  let activities = await res.json()
  console.log("On va afficher le graphique")

  // 2️⃣ Trier par date croissante
  activities.sort((a, b) =>
    new Date(a.start_date) - new Date(b.start_date)
  )

  // 3️⃣ Construire les axes
  const labels = activities.map(a =>
    new Date(a.start_date).toLocaleDateString()
  )

  const speeds = activities.map(a => a.average_speed)

  // 4️⃣ Créer le graphique
  const ctx = document.getElementById("speedChart")

  new Chart(ctx, {
    type: "line",
    data: {
      labels,
      datasets: [{
        label: "Average speed (m/s)",
        data: speeds,
        borderColor: "#fc4c02",
        backgroundColor: "rgba(252,76,2,0.2)",
        tension: 0.3,
        pointRadius: 4
      }]
    },
    options: {
      responsive: true,
      scales: {
        y: {
          title: {
            display: true,
            text: "Average speed (m/s)"
          }
        },
        x: {
          title: {
            display: true,
            text: "Date"
          }
        }
      }
    }
  })
})
</script>

<template>
  <div>
    <h1>Évolution de la vitesse moyenne</h1>
    <canvas id="speedChart" width="900" height="400"></canvas>
  </div>
</template>
