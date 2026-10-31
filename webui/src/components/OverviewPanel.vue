<script setup lang="ts">
import { t, formatClock } from '../lib/i18n'
import { modules } from '../lib/api'
import { useProtection } from '../lib/protection'
import AppIcon from './AppIcon.vue'
const { states, core, connectionError, updated, tileState } = useProtection()
</script>
<template>
  <h1 class="sr-only">{{ t('Host protection') }}</h1>
  <p v-if="connectionError" role="alert" class="notice error">{{ t(connectionError) }}</p>
  <p v-else-if="core && !core.coreOnline" role="status" class="notice warning">
    {{ t('Check the protection service. Policy execution cannot be confirmed until it reconnects.') }}
  </p>
  <div class="protection-tiles">
    <section
      v-for="m in modules"
      :key="m.id"
      class="protection-tile"
      :class="tileState(m.id)"
      :aria-labelledby="'protection-' + m.id"
    >
      <header class="tile-top">
        <span class="tile-icon"><AppIcon :name="m.id" :size="30" /></span
        ><RouterLink
          class="tile-configure"
          :aria-label="t('Configure {module}', { module: t(m.name) })"
          :to="'/protection/' + m.id"
        >
          <AppIcon name="settings" :size="14" /><span>{{ t('Configure') }}</span>
        </RouterLink>
      </header>
      <h2 :id="'protection-' + m.id">
        <RouterLink class="tile-main-link" :to="'/protection/' + m.id">{{ t(m.name) }}</RouterLink>
      </h2>
      <div class="tile-state">
        <span class="tile-state-label"
          ><span></span
          >{{
            t(
              tileState(m.id) === 'unknown'
                ? 'Status unknown'
                : states[m.id]?.status === 1
                ? 'On'
                : 'Off'
            )
          }}</span
        >
      </div>
      <dl class="tile-metric">
        <dt>{{ t('Current policies') }}</dt>
        <dd>
          {{ states[m.id]?.policyCount ?? '—' }}<span>{{ t('') }}</span>
        </dd>
      </dl>
      <RouterLink
        class="tile-attention"
        :class="{ actionable: (states[m.id]?.unreadEventCount ?? 0) > 0 }"
        :to="{ path: '/events', query: { module: m.id } }"
        :aria-label="t('View pending items for {module}', { module: t(m.name) })"
        ><div>
          <strong>{{ states[m.id]?.unreadEventCount ?? '—' }}</strong
          ><span>{{ t('Needs attention') }}</span>
        </div>
        <span class="tile-attention-action"
          >{{ t((states[m.id]?.unreadEventCount ?? 0) > 0 ? 'Review' : 'View events')
          }}<AppIcon name="arrow" :size="14" /></span
      ></RouterLink>
    </section>
  </div>
  <div class="control-bottom">
    <span>{{ t('Refreshes every 10 seconds ·') }} {{ updated ? formatClock(updated) : t('Waiting for response') }}</span>
  </div>
</template>
