<script setup>
const loginWithStrava = () => {
  window.location.href = "http://localhost:8080/auth/login";
};
</script>

<template>
  <div class="login">
    <button class="strava-btn" @click="loginWithStrava">
      Se connecter avec Strava
    </button>
  </div>
</template>

<style scoped>
.strava-btn {
  background-color: #fc4c02;
  color: white;
  border: none;
  padding: 12px 20px;
  font-size: 16px;
  border-radius: 6px;
  cursor: pointer;
}

.strava-btn:hover {
  opacity: 0.9;
}
</style>
