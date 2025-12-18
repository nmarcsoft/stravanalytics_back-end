<script setup>
import { onMounted } from "vue"
import Chart from "chart.js/auto"

/* ===========================
   Classification des séances
   =========================== */
const isEF = name => {
  const n = name.toLowerCase()
  return n.includes("ef") || n.includes("endurance fondamentale")
}

const isVMA = name =>
  name.toLowerCase().includes("vma")

const isSeuil = name =>
  name.toLowerCase().includes("seuil")

/* ===========================
   Pondération FC
   =========================== */
const scaleHR = (hr, min = 110, max = 175) => {
  const clamped = Math.max(min, Math.min(max, hr))
  return 3 + (clamped - min) * (10 - 3) / (max - min)
}

const hrColor = (hr, min = 110, max = 175) => {
  const ratio = Math.max(0, Math.min(1, (hr - min) / (max - min)))
  const r = Math.floor(255 * ratio)
  const g = Math.floor(200 * (1 - ratio))
  return `rgb(${r}, ${g}, 50)`
}

/* ===========================
   Création d’un graphique
   =========================== */
function createChart(canvasId, label, data, color) {
  new Chart(document.getElementById(canvasId), {
    type: "line",
    data: {
      labels: data.map(d => d.label),
      datasets: [{
        label,
        data: data.map(d => ({
          x: d.label,
          y: d.y,
          r: d.r,
          hr: d.hr   // 👈 transmis au tooltip
        })),
        borderColor: color,
        backgroundColor: color.replace("1)", "0.2)"),
        tension: 0.3,
        pointRadius: data.map(d => d.r),
        pointBackgroundColor: data.map(d => d.c),
        pointBorderColor: "#000"
      }]
    },
    options: {
      responsive: true,
      plugins: {
        tooltip: {
          callbacks: {
            label: ctx => {
              const speed = ctx.parsed.y.toFixed(2)
              const hr = ctx.raw.hr

              return [
                `Allure : ${speed} m/s`,
                `FC moyenne : ${hr} bpm`
              ]
            }
          }
        }
      },
      scales: {
        y: {
          reverse: true
        }
      }
    }
  })
}

/* ===========================
   Lifecycle
   =========================== */
onMounted(async () => {
  const res = await fetch("http://localhost:8080/api/activities")
  let activities = await res.json()

  activities.sort((a, b) =>
    new Date(a.start_date) - new Date(b.start_date)
  )

  const mapData = list =>
    list.map(a => ({
      label: new Date(a.start_date).toLocaleDateString(),
      y: a.average_speed,
      r: scaleHR(a.average_heartrate),
      c: hrColor(a.average_heartrate),
      hr: a.average_heartrate
    }))

  createChart(
    "efChart",
    "EF – Allure pondérée FC",
    mapData(activities.filter(a => isEF(a.name))),
    "rgba(76,175,80,1)",
  )

  createChart(
    "vmaChart",
    "VMA – Allure pondérée FC",
    mapData(activities.filter(a => isVMA(a.name))),
    "rgba(229,57,53,1)"
  )

  createChart(
    "seuilChart",
    "SEUIL – Allure pondérée FC",
    mapData(activities.filter(a => isSeuil(a.name))),
    "rgba(50,56,168,1)"
  )
})
</script>

<template>
  <div>
    <h1>Analyse des allures pondérées</h1>

    <h2>Endurance fondamentale</h2>
    <canvas id="efChart" width="900" height="350"></canvas>

    <h2>VMA</h2>
    <canvas id="vmaChart" width="900" height="350"></canvas>

    <h2>Seuil</h2>
    <canvas id="seuilChart" width="900" height="350"></canvas>
  </div>
</template>
