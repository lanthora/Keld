<script setup lang="ts">
import { t } from '../lib/i18n'

import { useDialog } from '../lib/dialog'
import { onMounted, ref } from 'vue'
import { api, errorMessage, type User } from '../lib/api'
import { currentUser, loadSession } from '../lib/session'
const rows = ref<User[]>([]),
  busy = ref(false),
  error = ref(''),
  success = ref(''),
  showForm = ref(false)
const form = ref({ userID: 0, username: '', aliasName: '', password: '', permissions: '{*}' })
async function load() {
  busy.value = true
  error.value = ''
  try {
    rows.value = (await api<User[] | null>('/admin/listAllUsers')) || []
  } catch (e) {
    error.value = errorMessage(e)
    rows.value = []
  } finally {
    busy.value = false
  }
}
function open(user?: User) {
  form.value = {
    userID: user?.userID || 0,
    username: user?.username || '',
    aliasName: user?.aliasName || '',
    password: '',
    permissions: user?.permissions || '{*}'
  }
  error.value = ''
  showForm.value = true
}
async function save() {
  busy.value = true
  error.value = ''
  success.value = ''
  try {
    await api(`/admin/${form.value.userID ? 'updateUserInfo' : 'addUser'}`, form.value)
    showForm.value = false
    form.value.password = ''
    success.value = 'Account saved'
    await loadSession()
    await load()
  } catch (e) {
    error.value = errorMessage(e)
  } finally {
    busy.value = false
  }
}
async function remove(user: User) {
  if (!window.confirm(t(t('Delete account {p0}?', { p0: user.username })))) return
  busy.value = true
  error.value = ''
  success.value = ''
  try {
    await api('/admin/deleteUser', { userID: user.userID })
    success.value = 'Account deleted'
    await load()
  } catch (e) {
    error.value = errorMessage(e)
  } finally {
    busy.value = false
  }
}
onMounted(load)
const dialog = useDialog(
  () => showForm.value,
  () => {
    if (!busy.value) showForm.value = false
  }
)
</script>
<template>
  <div class="toolbar">
    <p class="muted">{{ t('Manage console accounts and API access permissions.') }}</p>
    <div class="inline">
      <button :disabled="busy" @click="load">{{ t('Refresh') }}</button
      ><button class="primary" :disabled="busy" @click="open()">{{ t('＋ Create account') }}</button>
    </div>
  </div>
  <p v-if="error && !showForm" class="notice error" role="alert">{{ t(error) }}</p>
  <p v-if="success" class="notice success" role="status">{{ t(success) }}</p>
  <section class="panel table-panel">
    <div class="table-scroll">
      <table>
        <thead>
          <tr>
            <th>{{ t('Username') }}</th>
            <th>{{ t('Display name') }}</th>
            <th>{{ t('Permission expression') }}</th>
            <th>{{ t('Actions') }}</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="user in rows" :key="user.userID">
            <td>
              <strong>{{ user.username }}</strong
              ><span v-if="user.userID === currentUser?.userID" class="pill neutral">{{
                t('Current account')
              }}</span>
            </td>
            <td>{{ user.aliasName }}</td>
            <td class="mono wrap">{{ user.permissions }}</td>
            <td>
              <div class="row-actions">
                <button :disabled="busy" @click="open(user)">{{ t('Edit') }}</button
                ><button
                  class="danger-text"
                  :disabled="busy || user.userID === currentUser?.userID"
                  @click="remove(user)"
                >
                  {{ t('Delete') }}
                </button>
              </div>
            </td>
          </tr>
          <tr v-if="!rows.length">
            <td colspan="4" class="empty">
              {{ busy ? t('Loading accounts…') : error ? t('Could not load accounts') : t('No accounts') }}
            </td>
          </tr>
        </tbody>
      </table>
    </div>
  </section>
  <div v-if="showForm" class="modal-backdrop" @click.self="!busy && (showForm = false)">
    <section
      class="modal"
      ref="dialog"
      role="dialog"
      aria-modal="true"
      aria-labelledby="user-title"
    >
      <div class="section-heading">
        <h2 id="user-title">
          {{ form.userID ? t('Edit account') : t('Create account') }}
        </h2>
        <button :disabled="busy" @click="showForm = false">{{ t('Close') }}</button>
      </div>
      <form @submit.prevent="save">
        <label
          >{{ t('Username')
          }}<input
            v-model.trim="form.username"
            autocomplete="off"
            required
            maxlength="128" /></label
        ><label
          >{{ t('Display name')
          }}<input v-model.trim="form.aliasName" required maxlength="128" /></label
        ><label
          >{{ form.userID ? t('New password (required when editing)') : t('Password')
          }}<input
            v-model="form.password"
            type="password"
            autocomplete="new-password"
            required
            minlength="8"
            maxlength="72" /></label
        ><label
          >{{ t('Permission expression') }}<input v-model.trim="form.permissions" required placeholder="{*}"
        /></label>
        <p class="muted">
          {{
            t(
              'Match API paths with glob expressions. Full access: {*}; read-only example: {/process/listEvents,/process/show*,/file/list*,/file/show*,/net/list*,/net/show*,/ctrl/status}. Session information and sign-out are always available.'
            )
          }}
        </p>
        <p v-if="error" class="notice error" role="alert">{{ t(error) }}</p>
        <div class="form-actions">
          <button type="button" :disabled="busy" @click="showForm = false">{{ t('Cancel') }}</button
          ><button class="primary" :disabled="busy">
            {{ busy ? t('Saving…') : t('Save account') }}
          </button>
        </div>
      </form>
    </section>
  </div>
</template>
