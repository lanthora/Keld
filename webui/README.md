# Keld web interface

A Vue 3 and TypeScript single-page application using same-origin cookie sessions with the Go/Gin service. Production assets are embedded into `keld-web` with `go:embed`; Node.js is needed only for builds.

## Features

- Sign-in, sign-out, session restoration after refresh, and expired-session redirects.
- A protection home with process, file, and network status, policy counts, and pending events. Configuration opens in dedicated routes.
- Process audit/defense modes and command trust management.
- File policies with host filesystem browsing, operation masks, and auditing/blocking.
- IPv4 network rules with protocol, port and address ranges, direction, priority, accept/drop behavior, and logging.
- Event details, filtering, ID cursor pagination, review status, and deletion.
- Account creation, editing, password reset, deletion, and glob-based endpoint permissions.
- Core connection checks, diagnostics controls, and core shutdown.
- English and Simplified Chinese, with browser-language detection and a saved preference.

The interface polls shared status every ten seconds, pauses polling when hidden, and preserves unsaved edits. Settings are confirmed only after successful backend operations. Module status reflects saved configuration; the core protocol does not expose each module's live enabled state. Connectivity alone does not confirm policy enforcement.

## Build and run

Use Node.js 20.19+ (20.x) or 22.12+, npm, and the Go toolchain specified in `apps/go.mod`.

```sh
# From the repository root:
make web
./apps/cmd/web/keld-web -config /path/to/web.yaml
```

`make web` installs locked frontend dependencies, checks types, builds and embeds assets, and compiles the Go web service. It does not build the kernel module. A running Linux Keld core is required for protection operations.

The configuration template is `apps/configs/web.yaml`. The default listener is `127.0.0.1:8080`; the default configuration path is `/etc/keld/web.yaml`. An empty user table bootstraps the first signed-in account as administrator. Complete this in a trusted environment using an 8–72 byte password. Legacy SHA-256 passwords are upgraded to bcrypt after successful authentication. Sessions are held in memory and expire when the service restarts.

## Development

```sh
cd webui
npm ci
npm run dev
```

Vite proxies `/auth`, `/admin`, `/ctrl`, `/process`, `/file`, and `/net` to `http://127.0.0.1:8080`. Override the target with `KELD_API=http://127.0.0.1:<port> npm run dev`. Access the application through Vite to keep sessions same-origin.

```sh
npm run build        # Type checking and dist output
npm run build:embed  # Build and prepare Go embedded assets
npx eslint src --ext .vue,.ts
npm test
```

After frontend changes, regenerate embedded assets and rebuild Go. Build artifacts are excluded from Git.

## API and permissions

The HTTP API uses POST JSON and returns `{ status, message, data }`. Status `0` means success, `100` means unauthenticated, and `101` means permission denied. List `offset` values are the last record ID, rather than a row offset; `limit` accepts 1–200. The interface displays 25 records per page.

- `POST /file/browse`: `{ path, after?, search? }`; returns canonical path, parent, directory entries, and a continuation cursor. Results include metadata only, with at most 100 entries sorted by name. Symlinks follow target types; broken links and special files cannot be selected. This endpoint requires its own permission; `/file/list*` does not grant access.
- `POST /ctrl/status`: returns `{ coreOnline, debugEnabled }` after a core echo check.
- `POST /process/deleteEvents`: `{ id }`; trusted records must be untrusted before deletion.
- `POST /file/clearPolicies`: clears file policies from the core and database.
- `POST /ctrl/enableDebug` and `/ctrl/disableDebug`: control diagnostics. pprof starts disabled and still requires authentication and endpoint permission when enabled.

Authenticated accounts can always retrieve their own session information and sign out. Other operations require matching saved glob permissions. Account changes revoke prior sessions; editing the current account preserves only the current session.

All permissions: `{*}`. Read-only example:

```text
{/process/listEvents,/process/show*,/file/list*,/file/show*,/net/list*,/net/show*,/ctrl/status}
```

The API validates same-origin Origin/Fetch Metadata, uses HttpOnly/SameSite cookies, and limits request bodies. Remote deployment requires an explicit listener and appropriate HTTPS/access controls. A TLS reverse proxy must forward the original Host correctly.

## Localization

Application source uses English message keys. Catalogs live in `src/lib/locales/en.json` and `zh-CN.json`. The language selector and localized icons use their respective language names and artwork. Product names, page titles, time formatting, UI messages, and API error presentation follow the selected language.

## Tests

```sh
cd apps
go test ./...
go test -race ./internal/web ./internal/worker
```

Go regression tests use in-memory SQLite and isolated Unix sockets to cover authentication, permissions, pagination, policy operations, core failure handling, filesystem browsing, and static routing. `KELD_SOCKET` overrides `/tmp/keld.sock` for isolated clients. Real enforcement requires Linux integration testing; see the root README.

Frontend tests cover language selection, branding, interpolation, translation completeness, API error messages, server-confirmed protection controls, failed operations, edit preservation, partial-save retries, and canceled dangerous actions.

## Workspace layout

The home screen presents three protection cards. Event and policy routes are reached from those cards; a bottom-right tools menu provides settings, accounts, language selection, and sign-out. Other pages link back home. Content remains centered when space permits and scrolls naturally when tall. The lower-left connection status is persistent. Enabled cards use blue borders and status indicators; disabled cards use gray. Views share typography, spacing, form controls, and a single status poll.
