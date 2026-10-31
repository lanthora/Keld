<script setup lang="ts">
import { t } from '../lib/i18n'

import FilePicker from './FilePicker.vue'
import { useDialog } from '../lib/dialog'
import { computed, ref, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import {
  api,
  errorMessage,
  modules,
  permissionBits,
  permissionsText,
  type Module,
  type FilePolicy,
  type NetPolicy
} from '../lib/api'
const props = defineProps<{ module?: Module; embedded?: boolean }>()
const emit = defineEmits<{ (event: 'changed'): void }>()
const route = useRoute(),
  router = useRouter()
const module = computed<Module>(
  () =>
    props.module ||
    (modules.some((m) => m.id === route.query.module) ? (route.query.module as Module) : 'file')
)
const fileRows = ref<FilePolicy[]>([]),
  netRows = ref<NetPolicy[]>([]),
  cursors = ref([0]),
  hasNext = ref(false)
const error = ref(''),
  success = ref(''),
  busy = ref(false),
  showForm = ref(false),
  editing = ref<number | null>(null)
const path = ref(''),
  selectedBits = ref<number[]>([1, 2, 3])
function newNet(): NetPolicy {
  return {
    priority: 0,
    addr: {
      src: { begin: '0.0.0.0', end: '255.255.255.255' },
      dst: { begin: '0.0.0.0', end: '255.255.255.255' }
    },
    protocol: { begin: 0, end: 255 },
    port: { src: { begin: 0, end: 65535 }, dst: { begin: 0, end: 65535 } },
    flags: 3,
    response: 3
  }
}
const net = ref<NetPolicy>(newNet())
let generation = 0
async function load() {
  if (module.value === 'process') return
  const ticket = ++generation
  busy.value = true
  error.value = ''
  try {
    const data = await api<(FilePolicy | NetPolicy)[] | null>(`/${module.value}/listPolicies`, {
      limit: 26,
      offset: cursors.value[cursors.value.length - 1]
    })
    if (ticket !== generation) return
    hasNext.value = (data || []).length > 25
    if (module.value === 'file') fileRows.value = (data || []).slice(0, 25) as FilePolicy[]
    else netRows.value = (data || []).slice(0, 25) as NetPolicy[]
  } catch (e) {
    if (ticket === generation) {
      error.value = errorMessage(e)
      fileRows.value = []
      netRows.value = []
    }
  } finally {
    if (ticket === generation) busy.value = false
  }
}
watch(
  module,
  () => {
    generation++
    showForm.value = false
    cursors.value = [0]
    success.value = ''
    load()
  },
  { immediate: true }
)
function openForm(policy?: FilePolicy) {
  error.value = ''
  editing.value = policy?.id ?? null
  path.value = policy?.path || ''
  selectedBits.value = policy
    ? permissionBits.map((_, i) => i).filter((i) => policy.perm & (1 << i))
    : [1, 2, 3]
  net.value = newNet()
  showForm.value = true
}
async function save() {
  busy.value = true
  error.value = ''
  success.value = ''
  try {
    if (module.value === 'file') {
      const perm = selectedBits.value.reduce((mask, bit) => mask | (1 << bit), 0)
      if (!path.value) throw new Error('Choose a file or directory first')
      if (!perm) throw new Error('Select at least one protection or audit operation')
      await api(
        `/file/${editing.value ? 'updatePolicy' : 'addPolicy'}`,
        editing.value ? { id: editing.value, perm } : { path: path.value, perm }
      )
    } else {
      if (!window.confirm(t(t('Apply this network rule? Make sure it will not block your management connection.')))) return
      await api('/net/addPolicy', net.value)
    }
    showForm.value = false
    success.value = 'Policy applied and saved'
    emit('changed')
    await load()
  } catch (e) {
    error.value = errorMessage(e)
  } finally {
    busy.value = false
  }
}
async function remove(id: number) {
  if (!window.confirm(t(t('Remove this protection policy?')))) return
  busy.value = true
  error.value = ''
  success.value = ''
  try {
    await api(`/${module.value}/deletePolicy`, { id })
    success.value = 'Policy removed'
    emit('changed')
    await load()
  } catch (e) {
    error.value = errorMessage(e)
  } finally {
    busy.value = false
  }
}
async function clearFiles() {
  if (!window.confirm(t(t('Clear all file protection policies? All file protection rules will be removed.')))) return
  busy.value = true
  error.value = ''
  try {
    await api('/file/clearPolicies')
    cursors.value = [0]
    success.value = 'File policies cleared'
    emit('changed')
    await load()
  } catch (e) {
    error.value = errorMessage(e)
  } finally {
    busy.value = false
  }
}
function next() {
  const list = module.value === 'file' ? fileRows.value : netRows.value
  if (list.length) cursors.value.push(list[list.length - 1].id!)
  load()
}
function previous() {
  cursors.value.pop()
  load()
}
const dialog = useDialog(
  () => showForm.value,
  () => {
    if (!busy.value) showForm.value = false
  }
)
defineExpose({ openForm })
</script>
<template>
  <div v-if="!embedded" class="tabs">
    <button
      v-for="m in modules"
      :key="m.id"
      :class="{ active: module === m.id }"
      :disabled="busy"
      @click="router.replace({ query: { module: m.id } })"
    >
      {{ t(m.name) }}
    </button>
  </div>
  <section v-if="module === 'process'" class="panel explanation">
    <h2>{{ t('Command trust policies') }}</h2>
    <p class="muted">
      {{
        t(
          'Process policies match working directory, executable and arguments. Manage command trust in the event list and operating modes in the protection center.'
        )
      }}
    </p>
    <RouterLink class="button primary" to="/events?module=process">{{
      t('Manage command trust →')
    }}</RouterLink>
  </section>
  <template v-else
    ><div class="toolbar">
      <p class="muted">
        {{
          module === 'file'
            ? t('Choose a file or directory on the host and configure protection and auditing.')
            : t('IPv4 range matching. Lower priority values are matched first.')
        }}
      </p>
      <div class="inline">
        <button :disabled="busy" @click="load">{{ t('Refresh') }}</button
        ><button v-if="module === 'file'" :disabled="busy" @click="clearFiles">
          {{ t('Clear policies') }}</button
        ><button class="primary" :disabled="busy" @click="openForm()">
          {{ t('＋ Add policy') }}
        </button>
      </div>
    </div>
    <p v-if="error && !showForm" class="notice error" role="alert">{{ t(error) }}</p>
    <p v-if="success" class="notice success" role="status">{{ t(success) }}</p>
    <section class="panel table-panel">
      <div class="table-scroll">
        <table v-if="module === 'file'">
          <thead>
            <tr>
              <th>ID</th>
              <th>{{ t('File path') }}</th>
              <th>{{ t('Protection and auditing') }}</th>
              <th>{{ t('Status') }}</th>
              <th>{{ t('Actions') }}</th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="row in fileRows" :key="row.id">
              <td>#{{ row.id }}</td>
              <td class="mono wrap">{{ row.path }}</td>
              <td>{{ permissionsText(row.perm) }}</td>
              <td>
                <span class="pill" :class="row.status === 0 ? 'success' : 'warning'">{{
                  [t('Normal'), t('Unknown'), t('Conflict'), t('File not found')][row.status] || t('Unknown')
                }}</span>
              </td>
              <td>
                <div class="row-actions">
                  <button :disabled="busy" @click="openForm(row)">{{ t('Edit') }}</button
                  ><button class="danger-text" :disabled="busy" @click="remove(row.id)">
                    {{ t('Delete') }}
                  </button>
                </div>
              </td>
            </tr>
            <tr v-if="!fileRows.length">
              <td colspan="5" class="empty">
                {{ busy ? t('Loading…') : error ? t('Could not load data. Please retry.') : t('No file policies') }}
              </td>
            </tr>
          </tbody>
        </table>
        <table v-else>
          <thead>
            <tr>
              <th>{{ t('ID / Priority') }}</th>
              <th>{{ t('Address ranges') }}</th>
              <th>{{ t('Protocol / Ports') }}</th>
              <th>{{ t('Match / Action') }}</th>
              <th>{{ t('Actions') }}</th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="row in netRows" :key="row.id">
              <td>
                #{{ row.id }}<br /><span class="muted">{{ t('Priority') }} {{ row.priority }}</span>
              </td>
              <td class="mono">
                {{ t('Source') }} {{ row.addr.src.begin }} – {{ row.addr.src.end }}<br />{{
                  t('Destination')
                }}
                {{ row.addr.dst.begin }} – {{ row.addr.dst.end }}
              </td>
              <td>
                {{ t('Protocol') }} {{ row.protocol.begin }} – {{ row.protocol.end }}<br /><span
                  class="muted"
                  >{{ t('Source port') }} {{ row.port.src.begin }}–{{ row.port.src.end }}<br />{{
                    t('Destination port')
                  }}
                  {{ row.port.dst.begin }}–{{ row.port.dst.end }}</span
                >
              </td>
              <td>
                {{ row.flags & 1 ? t('Inbound ') : '' }}{{ row.flags & 2 ? t('Outbound ') : ''
                }}{{ row.flags & 4 ? t('Handshake ') : '' }}{{ row.flags & 8 ? t('No data') : ''
                }}<br /><span class="pill" :class="row.response & 1 ? 'success' : 'warning'"
                  >{{ row.response & 1 ? t('Allow') : t('Block')
                  }}{{ row.response & 2 ? t(' · Log') : '' }}</span
                >
              </td>
              <td>
                <button class="danger-text" :disabled="busy" @click="remove(row.id!)">
                  {{ t('Delete') }}
                </button>
              </td>
            </tr>
            <tr v-if="!netRows.length">
              <td colspan="5" class="empty">
                {{ busy ? t('Loading…') : error ? t('Could not load data. Please retry.') : t('No network policies') }}
              </td>
            </tr>
          </tbody>
        </table>
      </div>
      <div class="pagination">
        <span>{{ t('Page {page} · Up to 25 records per page', { page: cursors.length }) }}</span>
        <div>
          <button :disabled="busy || cursors.length === 1" @click="previous">
            {{ t('Previous') }}</button
          ><button :disabled="busy || !hasNext" @click="next">{{ t('Next') }}</button>
        </div>
      </div>
    </section>
  </template>
  <div v-if="showForm" class="modal-backdrop" @click.self="!busy && (showForm = false)">
    <section
      class="modal"
      ref="dialog"
      role="dialog"
      aria-modal="true"
      aria-labelledby="policy-title"
    >
      <div class="section-heading">
        <h2 id="policy-title">
          {{ t(module === 'file' ? (editing ? 'Edit file policy' : 'Add file policy') : 'Add network policy') }}
        </h2>
        <button :disabled="busy" @click="showForm = false">{{ t('Close') }}</button>
      </div>
      <form @submit.prevent="save">
        <template v-if="module === 'file'">
          <div v-if="editing !== null" class="file-picker-selection">
            <span class="muted">{{ t('Protection target') }}</span
            ><strong class="mono wrap">{{ path }}</strong>
          </div>
          <FilePicker v-else v-model="path" :disabled="busy" />
          <fieldset>
            <legend>{{ t('Protection and audit operations') }}</legend>
            <div class="checkbox-grid">
              <label v-for="(label, bit) in permissionBits" :key="bit" class="check"
                ><input v-model="selectedBits" type="checkbox" :value="bit" />{{ t(label) }}</label
              >
            </div>
          </fieldset>
          <p class="muted">{{ t('Blocking operations may affect system services. Choose carefully.') }}</p></template
        >
        <template v-else
          ><div class="form-grid">
            <label
              >{{ t('Priority')
              }}<input
                v-model.number="net.priority"
                type="number"
                min="-128"
                max="127"
                required /></label
            ><label
              >{{ t('Traffic direction')
              }}<select v-model.number="net.flags">
                <option :value="1">{{ t('Inbound') }}</option>
                <option :value="2">{{ t('Outbound') }}</option>
                <option :value="3">{{ t('Inbound and outbound') }}</option>
                <option :value="5">{{ t('Inbound TCP handshake') }}</option>
                <option :value="6">{{ t('Outbound TCP handshake') }}</option>
                <option :value="7">{{ t('TCP handshake in both directions') }}</option>
                <option :value="9">{{ t('Inbound TCP without data') }}</option>
                <option :value="10">{{ t('Outbound TCP without data') }}</option>
                <option :value="11">{{ t('TCP without data in both directions') }}</option>
              </select></label
            >
          </div>
          <fieldset v-for="side in (['src', 'dst'] as const)" :key="side">
            <legend>{{ t(side === 'src' ? 'Source addresses and ports' : 'Destination addresses and ports') }}</legend>
            <div class="form-grid">
              <label
                >{{ t('First IPv4 address') }}<input v-model.trim="net.addr[side].begin" required /></label
              ><label
                >{{ t('Last IPv4 address') }}<input v-model.trim="net.addr[side].end" required /></label
              ><label
                >{{ t('First port')
                }}<input
                  v-model.number="net.port[side].begin"
                  type="number"
                  min="0"
                  max="65535"
                  required /></label
              ><label
                >{{ t('Last port')
                }}<input
                  v-model.number="net.port[side].end"
                  type="number"
                  :min="net.port[side].begin"
                  max="65535"
                  required
              /></label>
            </div>
          </fieldset>
          <div class="form-grid">
            <label
              >{{ t('First protocol number')
              }}<input
                v-model.number="net.protocol.begin"
                type="number"
                min="0"
                max="255"
                required /></label
            ><label
              >{{ t('Last protocol number')
              }}<input
                v-model.number="net.protocol.end"
                type="number"
                :min="net.protocol.begin"
                max="255"
                required /></label
            ><label
              >{{ t('Action on match')
              }}<select v-model.number="net.response">
                <option :value="3">{{ t('Allow and log') }}</option>
                <option :value="2">{{ t('Block and log') }}</option>
                <option :value="1">{{ t('Allow') }}</option>
                <option :value="0">{{ t('Block') }}</option>
              </select></label
            >
          </div>
          <p class="muted">
            {{ t('Protocol numbers: ICMP = 1, TCP = 6, UDP = 17. The range 0–255 matches all protocols. Both endpoints are included.') }}
          </p></template
        >
        <p v-if="error" class="notice error" role="alert">{{ t(error) }}</p>
        <div class="form-actions">
          <button type="button" :disabled="busy" @click="showForm = false">{{ t('Cancel') }}</button
          ><button class="primary" :disabled="busy || (module === 'file' && !path)">
            {{ busy ? t('Applying…') : t('Apply policy') }}
          </button>
        </div>
      </form>
    </section>
  </div>
</template>
