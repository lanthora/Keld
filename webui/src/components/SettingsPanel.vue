<script setup lang="ts">
import { t } from '../lib/i18n'
import { onMounted, ref } from 'vue'
import { api, errorMessage, type CoreStatus } from '../lib/api'
const core = ref<CoreStatus | null>(null),
  busy = ref(false),
  error = ref(''),
  success = ref('')
async function load() {
  try {
    core.value = await api<CoreStatus>('/ctrl/status')
  } catch (e) {
    core.value = null
    error.value = errorMessage(e)
  }
}
async function apply(path: string, confirmation?: string) {
  if (confirmation && !window.confirm(t(confirmation))) return
  busy.value = true
  error.value = ''
  success.value = ''
  try {
    await api(path)
    success.value = 'Operation successful'
    await load()
  } catch (e) {
    error.value = errorMessage(e)
  } finally {
    busy.value = false
  }
}
async function checkCore() {
  busy.value = true
  error.value = ''
  await load()
  busy.value = false
}
function toggleDebug() {
  return apply(`/ctrl/${core.value?.debugEnabled ? 'disableDebug' : 'enableDebug'}`)
}
function stopCore() {
  return apply('/ctrl/shutdown', 'Stop the protection core? Host protection will be interrupted.')
}
onMounted(checkCore)
</script>
<template>
  <p v-if="error" class="notice error" role="alert">{{ t(error) }}</p>
  <p v-if="success" class="notice success" role="status">{{ t(success) }}</p>
  <div class="settings-shortcut">
    <span>{{ t('Protection switches and operating modes are now in the protection center.') }}</span
    ><RouterLink to="/overview">{{ t('Open protection center') }} →</RouterLink>
  </div>
  <section class="panel settings-section">
    <h2>{{ t('Diagnostics and services') }}</h2>
    <div class="setting-row">
      <div>
        <h3>{{ t('Core communication') }}</h3>
        <p class="muted">{{ t(core ? (core.coreOnline ? 'Connected' : 'Disconnected') : 'Unknown') }}</p>
      </div>
      <button :disabled="busy" @click="checkCore">{{ t('Check again') }}</button>
    </div>
    <div class="setting-row">
      <div>
        <h3>{{ t('Performance diagnostics') }}</h3>
        <p class="muted">
          {{ t(core ? (core.debugEnabled ? 'On' : 'Off') : 'Unknown') }}
          {{ t('· Enable pprof only when troubleshooting.') }}
        </p>
      </div>
      <button :disabled="busy || !core" @click="toggleDebug">
        {{ t(core?.debugEnabled ? 'Disable diagnostics' : 'Enable diagnostics') }}
      </button>
    </div>
    <div class="setting-row">
      <div>
        <h3>{{ t('Stop protection core') }}</h3>
        <p class="muted">{{ t('Stopping the core interrupts protection. systemd may restart it according to its service configuration.') }}</p>
      </div>
      <button class="danger-text" :disabled="busy || !core?.coreOnline" @click="stopCore">
        {{ t('Stop core') }}
      </button>
    </div>
  </section>
</template>
