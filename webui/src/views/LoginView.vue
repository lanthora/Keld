<script setup lang="ts">
import { t } from '../lib/i18n'

import AppIcon from '../components/AppIcon.vue'
import LanguageSwitcher from '../components/LanguageSwitcher.vue'
import { ref } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { api, errorMessage } from '../lib/api'
import { loadSession } from '../lib/session'
const route = useRoute(),
  router = useRouter()
const username = ref(''),
  password = ref(''),
  busy = ref(false)
const error = ref(route.query.unavailable ? 'Cannot connect to the management service. Check it and retry.' : '')
async function login() {
  busy.value = true
  error.value = ''
  try {
    await api('/auth/login', { username: username.value, password: password.value })
    await loadSession()
    const target = String(route.query.redirect || '/overview')
    await router.replace(
      /^\/(overview|events|policies|settings|users|protection\/(process|file|net))([?#]|$)/.test(
        target
      )
        ? target
        : '/overview'
    )
  } catch (e) {
    error.value = errorMessage(e)
  } finally {
    busy.value = false
  }
}
</script>
<template>
  <main class="login-page">
    <section class="login-window" aria-labelledby="login-heading">
      <header class="login-window-bar">
        <div>
          <AppIcon name="shield" :size="19" /><strong>{{ t('product.name') }}</strong>
        </div>
        <LanguageSwitcher />
      </header>
      <div class="login-window-body">
        <h2 id="login-heading">{{ t('Sign in to the console') }}</h2>
        <p class="muted">{{ t('Use a host management account to continue.') }}</p>
        <form @submit.prevent="login">
          <label
            >{{ t('Username')
            }}<input
              v-model.trim="username"
              autocomplete="username"
              required
              maxlength="128"
              autofocus /></label
          ><label
            >{{ t('Password')
            }}<input
              v-model="password"
              type="password"
              autocomplete="current-password"
              required
              maxlength="72"
          /></label>
          <p v-if="error" class="notice error" role="alert">{{ t(error) }}</p>
          <div class="login-actions">
            <span class="muted">{{ t('Linux host management') }}</span
            ><button class="primary" :disabled="busy">{{ t(busy ? 'Signing in…' : 'Sign in') }}</button>
          </div>
        </form>
        <details class="setup-note">
          <summary>{{ t('First use and account setup') }}</summary>
          <p>
            {{
              t(
                'On first use, when no accounts exist, these credentials create an administrator. The password must be 8–72 bytes. Complete setup in a trusted local environment.'
              )
            }}
          </p>
        </details>
      </div>
    </section>
  </main>
</template>
