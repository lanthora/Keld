import { t, locale } from './i18n'
export class ApiError extends Error {
  constructor(public status: number, message: string) {
    super(message)
  }
}
export async function api<T = void>(path: string, body: unknown = {}): Promise<T> {
  const controller = new AbortController()
  const timeout = window.setTimeout(() => controller.abort(), 15000)
  try {
    const response = await fetch(path, {
      method: 'POST',
      credentials: 'same-origin',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(body),
      signal: controller.signal
    })
    if (!response.ok) throw new Error(t('Service request failed (HTTP {p0})', { p0: response.status }))
    const result = await response.json()
    if (result.status !== 0) {
      if (result.status === 100) window.dispatchEvent(new Event('session-expired'))
      throw new ApiError(result.status, result.message || 'Operation failed')
    }
    return result.data as T
  } catch (error) {
    if (error instanceof DOMException && error.name === 'AbortError')
      throw new Error('Request timed out. Check the service connection and retry.')
    if (error instanceof TypeError) throw new Error('Cannot connect to the management service. Check the network or service status.')
    throw error
  } finally {
    window.clearTimeout(timeout)
  }
}
export const errorMessage = (error: unknown) =>
  error instanceof Error ? error.message : 'Operation failed'
export type Module = 'process' | 'file' | 'net'
export const modules: { id: Module; name: string; description: string }[] = [
  { id: 'process', name: 'Process protection', description: 'Manage command trust and execution modes' },
  { id: 'file', name: 'File protection', description: 'Protect important files and directories' },
  { id: 'net', name: 'Network protection', description: 'Manage host network access rules' }
]
export interface User {
  userID: number
  username: string
  aliasName: string
  permissions: string
}
export interface ModuleStatus {
  status: number
  policyCount: number
  unreadEventCount: number
}
export interface CoreStatus {
  coreOnline: boolean
  debugEnabled: boolean
}
export interface EventRecord {
  id: number
  status: number
  workdir?: string
  binary?: string
  argv?: string
  count?: number
  judge?: number
  path?: string
  perm?: number
  timestamp?: number
  saddr?: string
  daddr?: string
  sport?: number
  dport?: number
  protocol?: number
  policy?: number
}
export interface FilePolicy {
  id: number
  path: string
  perm: number
  status: number
  timestamp: number
}
export interface Range<T> {
  begin: T
  end: T
}
export interface NetPolicy {
  id?: number
  priority: number
  addr: { src: Range<string>; dst: Range<string> }
  protocol: Range<number>
  port: { src: Range<number>; dst: Range<number> }
  flags: number
  response: number
}
export const permissionBits = [
  'Block reads',
  'Block writes',
  'Block deletion',
  'Block renaming',
  'Audit reads',
  'Audit writes',
  'Audit deletion',
  'Audit renaming'
]
export const permissionsText = (perm: number) =>
  permissionBits
    .filter((_, bit) => perm & (1 << bit))
    .map((label) => t(label))
    .join(locale.value === 'en' ? ', ' : '、') || t('None')
export const formatTime = (timestamp?: number) =>
  timestamp ? new Date(timestamp * 1000).toLocaleString(locale.value, { hour12: false }) : '—'
