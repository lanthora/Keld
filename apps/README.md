# Keld applications

## Build

Build and embed the frontend before compiling the Go applications:

```sh
make web
cd apps
CGO_ENABLED=0 make
```

The build produces `cmd/<name>/keld-<name>` for each application under `cmd`: `sample`, `telegram`, `web`, and `notify`. Builds also work in an initialized repository without a first commit.

SQLite uses the CGO-free `modernc.org/sqlite` driver. Existing SQLite 3 database files can be reused without conversion. The shared connection configuration enables WAL with `_pragma=journal_mode(WAL)`. To verify the Go applications without CGO, run `CGO_ENABLED=0 go test ./...` and `CGO_ENABLED=0 go build ./...` from this directory. The native core still needs its C/C++ toolchain.

## Run

```sh
# Display process audit events from the running core.
./cmd/sample/keld-sample

# Start the web service with an explicit configuration.
./cmd/web/keld-web -config configs/web.yaml
```

Default system configuration paths use `/etc/keld/`; see the templates in `configs/`. Telegram requires a configured bot token. Desktop notifications require a supported graphical user session. See [the web interface guide](../webui/README.md) for authentication, permissions, and frontend development.
