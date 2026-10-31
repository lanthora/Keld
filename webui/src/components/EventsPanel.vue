<script setup lang="ts">
import { t } from '../lib/i18n'

import { useDialog } from '../lib/dialog'
import { ref, watch, computed } from 'vue'
import { useRoute } from 'vue-router'
import {
  api,
  errorMessage,
  modules,
  permissionsText,
  formatTime,
  type Module,
  type EventRecord
} from '../lib/api'
const route = useRoute()
const module = computed<Module>(() =>
  modules.some((m) => m.id === route.query.module) ? (route.query.module as Module) : 'process'
)
const rows = ref<EventRecord[]>([]),
  cursors = ref([0]),
  busy = ref(false),
  error = ref(''),
  success = ref(''),
  detail = ref<EventRecord | null>(null)
const hasNext = ref(false),
  search = ref(''),
  unread = ref(false)
const visibleRows = computed(() =>
  rows.value.filter(
    (row) =>
      (!unread.value || row.status === 0) &&
      JSON.stringify(row).toLowerCase().includes(search.value.toLowerCase())
  )
)
let generation = 0
async function load() {
  const ticket = ++generation
  busy.value = true
  error.value = ''
  try {
    const data = await api<EventRecord[] | null>(`/${module.value}/listEvents`, {
      limit: 26,
      offset: cursors.value[cursors.value.length - 1]
    })
    if (ticket !== generation) return
    rows.value = (data || []).slice(0, 25)
    hasNext.value = (data || []).length > 25
  } catch (e) {
    if (ticket === generation) {
      error.value = errorMessage(e)
      rows.value = []
    }
  } finally {
    if (ticket === generation) busy.value = false
  }
}
watch(
  module,
  () => {
    rows.value = []
    cursors.value = [0]
    success.value = ''
    detail.value = null
    search.value = ''
    unread.value = false
    load()
  },
  { immediate: true }
)
async function update(row: EventRecord, status: number) {
  if (
    module.value === 'process' &&
    status === 1 &&
    !window.confirm(t(t('Mark this command as untrusted? Defense mode may prevent it from running.')))
  )
    return
  busy.value = true
  success.value = ''
  error.value = ''
  try {
    await api(`/${module.value}/updateEventStatus`, { id: row.id, status })
    success.value = module.value === 'process' ? 'Command trust policy updated' : 'Event status updated'
    await load()
  } catch (e) {
    error.value = errorMessage(e)
  } finally {
    busy.value = false
  }
}
async function remove(row: EventRecord) {
  if (!window.confirm(t(t('Delete this event permanently?')))) return
  busy.value = true
  error.value = ''
  success.value = ''
  try {
    await api(`/${module.value}/${module.value === 'process' ? 'deleteEvents' : 'deleteEvent'}`, {
      id: row.id
    })
    success.value = 'Event deleted'
    await load()
  } catch (e) {
    error.value = errorMessage(e)
  } finally {
    busy.value = false
  }
}
function next() {
  if (rows.value.length) cursors.value.push(rows.value[rows.value.length - 1].id)
  load()
}
function previous() {
  cursors.value.pop()
  load()
}
const detailFields: Record<string, string> = {
  id: 'Event ID',
  status: 'Review status',
  workdir: 'Working directory',
  binary: 'Executable',
  argv: 'Arguments',
  count: 'Execution count',
  judge: 'Decision mode',
  path: 'File path',
  perm: 'Protection and auditing',
  timestamp: 'Time',
  saddr: 'Source address',
  daddr: 'Destination address',
  sport: 'Source port',
  dport: 'Destination port',
  protocol: 'Protocol number',
  policy: 'Matched policy ID'
}
function detailValue(key: string, value: unknown) {
  if (key === 'timestamp') return formatTime(Number(value))
  if (key === 'perm') return permissionsText(Number(value))
  if (key === 'status')
    return t(
      (module.value === 'process' ? ['Pending', 'Untrusted', 'Trusted'] : ['Unread', 'Read'])[
        Number(value)
      ] || 'Unknown'
    )
  if (key === 'judge') return t(['No decision', 'Audit', 'Defense'][Number(value)] || 'Unknown')
  return value
}
const dialog = useDialog(
  () => !!detail.value,
  () => {
    detail.value = null
  }
)
</script>
<template>
  <div class="toolbar">
    <div class="inline">
      <input
        v-model="search"
        type="search"
        :placeholder="t('Filter records on this page…')"
        :aria-label="t('Filter records on this page')"
      /><label class="check"><input v-model="unread" type="checkbox" />{{ t('Pending only') }}</label>
    </div>
    <button :disabled="busy" @click="load">{{ t('Refresh events') }}</button>
  </div>
  <p v-if="error" class="notice error" role="alert">{{ t(error) }}</p>
  <p v-if="success" class="notice success" role="status">{{ t(success) }}</p>
  <section class="panel table-panel">
    <div class="table-scroll">
      <table>
        <thead>
          <tr>
            <th>{{ t('ID / Status') }}</th>
            <th>
              {{ module === 'process' ? t('Command') : module === 'file' ? t('File') : t('Network connection') }}
            </th>
            <th>
              {{ module === 'process' ? t('Execution details') : t('Event details') }}
            </th>
            <th>{{ t('Actions') }}</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="row in visibleRows" :key="row.id">
            <td>
              <span class="mono">#{{ row.id }}</span
              ><br /><span class="pill" :class="row.status === 0 ? 'warning' : 'neutral'">{{
                module === 'process'
                  ? [t('Pending'), t('Untrusted'), t('Trusted')][row.status]
                  : [t('Unread'), t('Read')][row.status]
              }}</span>
            </td>
            <td class="wide-cell">
              <strong>{{
                row.binary || row.path || `${row.saddr}:${row.sport} → ${row.daddr}:${row.dport}`
              }}</strong>
              <div class="muted mono wrap">
                {{
                  module === 'process'
                    ? row.argv
                    : module === 'file'
                    ? permissionsText(row.perm || 0)
                    : t('Protocol {p0} · Policy #{p1}', {
                        p0: row.protocol,
                        p1: row.policy
                      })
                }}
              </div>
            </td>
            <td class="muted">
              {{
                module === 'process'
                  ? t('Executions: {p0} · {p1}', {
                      p0: row.count,
                      p1: [t('No decision'), t('Audit'), t('Defense')][row.judge || 0]
                    })
                  : formatTime(row.timestamp)
              }}
            </td>
            <td>
              <div class="row-actions">
                <button @click="detail = row">{{ t('Details') }}</button
                ><template v-if="module === 'process'"
                  ><button :disabled="busy || row.status === 2" @click="update(row, 2)">
                    {{ t('Trusted') }}</button
                  ><button :disabled="busy || row.status === 1" @click="update(row, 1)">
                    {{ t('Untrusted') }}
                  </button></template
                ><button v-else :disabled="busy" @click="update(row, row.status === 0 ? 1 : 0)">
                  {{ row.status === 0 ? t('Mark as read') : t('Mark as unread') }}</button
                ><button
                  class="danger-text"
                  :disabled="busy || (module === 'process' && row.status === 2)"
                  :title="
                    module === 'process' && row.status === 2
                      ? t('Remove trust before deleting an active policy')
                      : ''
                  "
                  @click="remove(row)"
                >
                  {{ t('Delete') }}
                </button>
              </div>
            </td>
          </tr>
          <tr v-if="!visibleRows.length">
            <td colspan="4" class="empty">
              {{
                busy
                  ? t('Loading events…')
                  : error
                  ? t('Could not load data. Please retry.')
                  : t('No matching events')
              }}
            </td>
          </tr>
        </tbody>
      </table>
    </div>
    <div class="pagination">
      <span>{{
        t('Page {page} · Records: {count} · ascending ID', {
          page: cursors.length,
          count: rows.length
        })
      }}</span>
      <div>
        <button :disabled="busy || cursors.length === 1" @click="previous">{{ t('Previous') }}</button
        ><button :disabled="busy || !hasNext" @click="next">{{ t('Next') }}</button>
      </div>
    </div>
  </section>
  <div
    v-if="detail"
    class="modal-backdrop"
    @click.self="detail = null"
    @keydown.esc="detail = null"
  >
    <section
      class="modal"
      ref="dialog"
      role="dialog"
      aria-modal="true"
      aria-labelledby="event-detail"
    >
      <div class="section-heading">
        <h2 id="event-detail">{{ t('Event #{id}', { id: detail.id }) }}</h2>
        <button autofocus @click="detail = null">{{ t('Close') }}</button>
      </div>
      <dl class="detail-list">
        <template v-for="(value, key) in detail" :key="key"
          ><dt>{{ t(detailFields[key] || key) }}</dt>
          <dd>{{ detailValue(key, value) }}</dd></template
        >
      </dl>
    </section>
  </div>
</template>
