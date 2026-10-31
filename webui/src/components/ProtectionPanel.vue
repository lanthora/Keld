<script setup lang="ts">
import { computed } from 'vue'
import { useRoute } from 'vue-router'
import { t } from '../lib/i18n'
import { modules, type Module } from '../lib/api'
import { useProtection } from '../lib/protection'
import AppIcon from './AppIcon.vue'
import PoliciesPanel from './PoliciesPanel.vue'
const route = useRoute()
const configuring = computed(() => route.params.module as Module)
const moduleInfo = computed(() => modules.find((m) => m.id === configuring.value)!)
const {
  states,
  core,
  refreshing,
  pending,
  connectionError,
  feedback,
  savedMode,
  savedDefault,
  mode,
  defaultStatus,
  dirty,
  refresh,
  toggle,
  saveProcess,
  resetProcess,
  tileState
} = useProtection()
</script>
<template>
  <RouterLink class="back-link" to="/overview"
    ><AppIcon name="arrow" :size="15" />{{ t('Back to protection home') }}</RouterLink
  >
  <header class="dashboard-heading detail-heading">
    <div class="detail-title">
      <span class="detail-icon"><AppIcon :name="configuring" :size="32" /></span>
      <div>
        <h1>{{ t(moduleInfo.name) }}</h1>
        <p>{{ t('Manage this protection’s switch, mode and policies.') }}</p>
      </div>
    </div>
    <span class="detail-status" :class="tileState(configuring)"
      ><span class="status-dot" :class="{ connected: tileState(configuring) === 'enabled' }"></span
      >{{
        t(
          tileState(configuring) === 'unknown'
            ? 'Status unknown'
            : states[configuring]?.status === 1
            ? 'On'
            : 'Off'
        )
      }}</span
    >
  </header>
  <p v-if="connectionError" role="alert" class="notice error">{{ t(connectionError) }}</p>
  <section
    class="rule-inspector module-configuration"
    :aria-label="t('Configure {module}', { module: t(modules.find((m) => m.id === configuring)?.name) })"
  >
    <div class="configuration-switch-row">
      <div>
        <strong>{{ t('Module switch') }}</strong>
        <p>{{ t('Enable or disable this protection module.') }}</p>
      </div>
      <div class="protection-switch">
        <span>{{
          t(
            states[configuring] ? (states[configuring]?.status === 1 ? 'On' : 'Off') : 'Unknown'
          )
        }}</span
        ><button
          type="button"
          role="switch"
          :aria-checked="states[configuring]?.status === 1"
          :aria-label="
            t('Toggle {module}', { module: t(modules.find((m) => m.id === configuring)?.name) })
          "
          :disabled="refreshing || pending !== null || !states[configuring] || !core?.coreOnline"
          @click="toggle(modules.find((m) => m.id === configuring)!)"
        >
          <span></span>
        </button>
      </div>
    </div>
    <template v-if="configuring === 'process'"
      ><form class="process-properties" @submit.prevent="saveProcess">
        <label
          >{{ t('Operating mode')
          }}<select v-model.number="mode" :disabled="pending !== null || savedMode === undefined">
            <option v-if="savedMode === undefined" disabled :value="undefined">
              {{ t('Waiting for response') }}
            </option>
            <option :value="0">{{ t('No decision') }}</option>
            <option :value="1">{{ t('Audit mode') }}</option>
            <option :value="2">{{ t('Defense mode') }}</option>
          </select></label
        ><label
          >{{ t('Default status for new commands')
          }}<select
            v-model.number="defaultStatus"
            :disabled="pending !== null || savedDefault === undefined"
          >
            <option v-if="savedDefault === undefined" disabled :value="undefined">
              {{ t('Waiting for response') }}
            </option>
            <option :value="0">{{ t('Pending') }}</option>
            <option :value="1">{{ t('Untrusted') }}</option>
            <option :value="2">{{ t('Trusted') }}</option>
          </select></label
        >
        <div class="protection-save-row">
          <button
            class="primary"
            :disabled="!dirty || pending !== null || refreshing || !core?.coreOnline"
          >
            {{ t(pending === 'process' ? 'Applying…' : 'Save settings') }}</button
          ><button type="button" :disabled="!dirty || pending !== null" @click="resetProcess">
            {{ t('Reset') }}</button
          ><small v-if="dirty" class="unsaved-label">{{ t('Unsaved changes') }}</small>
        </div>
      </form>
      <p v-if="states.process?.status === 1 && savedMode === 0" class="inline-warning">
        {{ t('The module is on. Choose audit or defense mode and save.') }}
      </p>
      <RouterLink class="configuration-events" to="/events?module=process"
        >{{ t('Command trust and events') }} →</RouterLink
      ></template
    >
    <PoliciesPanel v-else :module="configuring" embedded @changed="refresh(true)" />
    <p v-if="pending === configuring" class="card-feedback" role="status">{{ t('Applying…') }}</p>
    <p v-else-if="feedback[configuring]?.error" class="card-feedback failure" role="alert">
      {{ t(feedback[configuring]?.error) }}
    </p>
    <p v-else-if="feedback[configuring]?.success" class="card-feedback" role="status">
      {{ t(feedback[configuring]?.success) }}
    </p>
  </section>
</template>
