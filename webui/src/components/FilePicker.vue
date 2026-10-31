<script setup lang="ts">
import { t } from '../lib/i18n'

import { onMounted, onUnmounted, ref } from 'vue'
import { api, errorMessage } from '../lib/api'
interface Entry {
  name: string
  path: string
  directory: boolean
  symlink: boolean
  selectable: boolean
}
interface Directory {
  path: string
  parent: string
  entries: Entry[]
  nextCursor: string
}
defineProps<{ modelValue: string; disabled?: boolean }>()
const emit = defineEmits<{ (event: 'update:modelValue', value: string): void }>()
const directory = ref<Directory | null>(null),
  busy = ref(false),
  error = ref(''),
  search = ref('')
const cursors = ref([''])
let generation = 0
async function load(path: string, after = '') {
  const ticket = ++generation
  busy.value = true
  error.value = ''
  try {
    const result = await api<Directory>('/file/browse', { path, after, search: search.value })
    if (ticket === generation) directory.value = result
  } catch (e) {
    if (ticket === generation) error.value = errorMessage(e)
  } finally {
    if (ticket === generation) busy.value = false
  }
}
function navigate(path: string) {
  cursors.value = ['']
  search.value = ''
  load(path)
}
function filter() {
  cursors.value = ['']
  load(directory.value?.path || '/')
}
function next() {
  if (!directory.value?.nextCursor) return
  cursors.value.push(directory.value.nextCursor)
  load(directory.value.path, directory.value.nextCursor)
}
function previous() {
  cursors.value.pop()
  load(directory.value?.path || '/', cursors.value[cursors.value.length - 1])
}
function choose(path: string) {
  emit('update:modelValue', path)
}
onMounted(() => {
  navigate('/')
})
onUnmounted(() => {
  generation++
})
</script>
<template>
  <section class="file-picker" :aria-label="t('Choose a file or directory on the managed host')">
    <p class="muted">{{ t('Choose a file or directory from the managed host’s filesystem.') }}</p>
    <div class="file-picker-location">
      <span class="mono wrap">{{ directory?.path || '/' }}</span
      ><button
        type="button"
        :disabled="busy || disabled || !directory || directory.path === directory.parent"
        @click="navigate(directory!.parent)"
      >
        {{ t('Parent directory') }}
      </button>
    </div>
    <div class="file-picker-search">
      <input
        v-model="search"
        type="search"
        :aria-label="t('Filter names in this directory')"
        :placeholder="t('Filter this directory by name')"
        :disabled="busy || disabled"
        @keydown.enter.prevent="filter"
      /><button type="button" :disabled="busy || disabled" @click="filter">{{ t('Search') }}</button
      ><button
        type="button"
        :disabled="busy || disabled || !directory"
        @click="choose(directory!.path)"
      >
        {{ t('Choose current directory') }}
      </button>
    </div>
    <p v-if="error" class="notice error" role="alert">
      {{ t(error) }}
      <button type="button" :disabled="busy || disabled" @click="load(directory?.path || '/')">
        {{ t('Retry') }}
      </button>
    </p>
    <div class="file-picker-entries" :aria-busy="busy">
      <p v-if="busy" class="empty">{{ t('Reading directory…') }}</p>
      <template v-else-if="directory && !error"
        ><div
          v-for="entry in directory.entries"
          :key="entry.name"
          class="file-picker-entry"
          :class="{ selected: modelValue === entry.path }"
        >
          <div class="file-picker-name">
            <button
              v-if="entry.directory"
              type="button"
              class="text-button"
              :disabled="disabled"
              @click="navigate(entry.path)"
            >
              {{ entry.name }}/</button
            ><span v-else class="mono wrap">{{ entry.name }}</span
            ><small class="muted">{{
              entry.symlink
                ? t('Symbolic link')
                : entry.directory
                ? t('Directory')
                : entry.selectable
                ? t('File')
                : t('Unavailable')
            }}</small>
          </div>
          <div class="inline">
            <button
              v-if="entry.directory"
              type="button"
              :disabled="disabled"
              @click="navigate(entry.path)"
            >
              {{ t('Open') }}</button
            ><button
              type="button"
              :disabled="disabled || !entry.selectable"
              :aria-label="t('Choose ') + entry.name"
              @click="choose(entry.path)"
            >
              {{ t('Choose') }}
            </button>
          </div>
        </div>
        <p v-if="!directory.entries.length" class="empty">
          {{ search ? t('No matching files or directories') : t('This directory is empty') }}
        </p></template
      >
    </div>
    <div class="pagination">
      <span>{{ t('Page {page}', { page: cursors.length }) }}</span>
      <div>
        <button
          type="button"
          :disabled="busy || disabled || cursors.length === 1"
          @click="previous"
        >
          {{ t('Previous') }}</button
        ><button type="button" :disabled="busy || disabled || !directory?.nextCursor" @click="next">
          {{ t('Next') }}
        </button>
      </div>
    </div>
    <div class="file-picker-selection">
      <span class="muted">{{ t('Selected') }}</span
      ><strong class="mono wrap">{{ modelValue || t('No file or directory selected') }}</strong>
    </div>
  </section>
</template>
