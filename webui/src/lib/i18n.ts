import { ref, watchEffect } from 'vue'
import english from './locales/en.json'
import chinese from './locales/zh-CN.json'

export type Locale = 'zh-CN' | 'en'
const storageKey = 'keld.locale'
function initialLocale(): Locale {
  try {
    const saved = localStorage.getItem(storageKey)
    if (saved === 'zh-CN' || saved === 'en') return saved
  } catch {
    /* Language selection also works when browser storage is unavailable. */
  }
  return navigator.language.toLowerCase().startsWith('zh') ? 'zh-CN' : 'en'
}
export const locale = ref<Locale>(initialLocale())
const translations: Record<string, string> = english
function lookup(messages: Record<string, string>, key: string): string | undefined {
  return Object.prototype.hasOwnProperty.call(messages, key) ? messages[key] : undefined
}
export function t(source: unknown, parameters: Record<string, unknown> = {}): string {
  const key = String(source ?? '')
  const message =
    locale.value === 'en'
      ? lookup(translations, key) ?? key
      : lookup(chinese, key) ?? lookup(translations, key) ?? key
  return message.replace(/\{(\w+)\}/g, (placeholder, name: string) =>
    Object.prototype.hasOwnProperty.call(parameters, name)
      ? String(parameters[name] ?? '')
      : placeholder
  )
}
export function setLocale(value: string) {
  if (value !== 'zh-CN' && value !== 'en') return
  locale.value = value
  try {
    localStorage.setItem(storageKey, value)
  } catch {
    /* Keep the in-memory preference. */
  }
}
export const formatClock = (time: number) =>
  new Date(time).toLocaleTimeString(locale.value, { hour12: false })
watchEffect(() => {
  document.documentElement.lang = locale.value
  document.title = t('product.title')
  document
    .querySelector<HTMLLinkElement>('link[rel="icon"]')
    ?.setAttribute('href', locale.value === 'en' ? '/favicon-en.svg' : '/favicon.svg')
})
