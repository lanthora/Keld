# Keld

Keld is a Linux host intrusion detection and prevention system. A kernel module observes and controls process execution, file access, and network traffic. A C++ daemon connects the kernel to Go applications, including a web service with an embedded Vue management interface. The interface supports English and Simplified Chinese.

This document is a deployment runbook for operators and AI agents. Given this README and authorized access to a target host, an agent should be able to install Keld, initialize its administrator, verify the deployment, and hand over a working service.

## Instructions for deployment agents

Carry out the deployment on the user's target host. Inspect the environment before choosing commands, read this entire runbook, and use the repository's build and installation scripts. Run privileged commands as root, or through an equivalent authorized sudo session.

- Use a Linux host with systemd and permission to load an out-of-tree kernel module. An ordinary application container is not a suitable target.
- Preserve existing Keld configuration, databases, credentials, and unrelated services. The fresh-install procedure below assumes Keld is not already installed. Follow the maintenance section for an existing installation.
- Keep the web listener on loopback during administrator initialization. Provide remote access through an SSH tunnel or an explicitly configured HTTPS reverse proxy.
- Deploy the core and web service. Build the Telegram, desktop notification, and sample applications as well; activate optional integrations only when their configuration is available.
- Leave enforcement disabled until the operator supplies an intended policy. A running service with no policy is a management deployment, not active protection.
- Do not declare success based only on HTTP availability or systemd status. Complete the authenticated core check and applicable acceptance tests below.
- If a host restriction prevents module loading, report the concrete blocker and the commands/results that establish it. Do not silently substitute a mock core or present a web-only installation as a complete deployment.

## 1. Inspect the host

Run these commands on the target host:

```sh
id
cat /etc/os-release
uname -a
uname -m
ps -p 1 -o comm=
df -h / /tmp
free -h
test -d "/lib/modules/$(uname -r)/build"
systemctl list-unit-files 'keld*'
```

Requirements:

| Component | Requirement |
| --- | --- |
| Operating system | Linux with systemd, root access, and loadable module support |
| Kernel | Linux 5.10 or later; matching headers and a usable kernel build tree for the **running** kernel |
| Architecture | x86-64, ARM, or ARM64 code paths; the deployment validation currently covers x86-64 |
| Native build | make, CMake 3.18.4+, a C++20-capable compiler, nlohmann-json, and libnl3 development files |
| Go | At least the version declared in `apps/go.mod`; CGO and a C compiler are required for SQLite |
| Frontend build | Node.js 22.12+ and npm; see `webui/package.json` for the complete engine constraint |
| Utilities | Git, curl, Python 3, kmod, binutils, and standard GNU shell tools |
| Network | Access to this repository and its Go/npm dependency registries during the build |

On x86-64, newer kernels use the ftrace syscall backend. Kernel tracing/kprobe support, exported facilities, distribution backports, Secure Boot, and lockdown settings can affect compatibility. Check `/proc/config.gz` or `/boot/config-$(uname -r)` where available, `mokutil --sb-state` if installed, and kernel logs when loading fails. If module signatures are enforced, use the host's approved signing/enrollment process. Do not disable host security settings as an implicit workaround.

If the running kernel's build tree is missing, install its matching headers. When the distribution only supplies headers for a newer installed kernel, boot that kernel during an appropriate maintenance window and repeat preflight. Do not build for one kernel and attempt to load into another.

## 2. Install build dependencies

Use the host's package manager. These examples cover the native dependencies; install or update Go and Node.js separately if the distribution packages do not meet the repository's requirements.

Debian / Ubuntu:

```sh
apt-get update
apt-get install -y git curl ca-certificates build-essential cmake \
  nlohmann-json3-dev libnl-3-dev libnl-genl-3-dev \
  "linux-headers-$(uname -r)" kmod binutils python3
```

Arch Linux, using the standard `linux` kernel:

```sh
pacman -S --needed git curl ca-certificates base-devel cmake nlohmann-json \
  libnl linux-headers go nodejs npm python kmod binutils
```

For `linux-lts`, `linux-zen`, or another kernel, use the corresponding header package. If Arch requires a system upgrade, perform a consistent system update and account for any required reboot before continuing.

For other distributions, install equivalent packages. Use official toolchain distributions or the host's established toolchain management method when newer Go or Node.js is required. Verify the actual executables in `PATH`, including inside the privileged build session:

```sh
go version
go env CGO_ENABLED
node --version
npm --version
cmake --version
c++ --version
test -f /usr/include/nlohmann/json.hpp
test -f /usr/include/libnl3/netlink/netlink.h
test -f "/lib/modules/$(uname -r)/build/Makefile"
```

`CGO_ENABLED` must be `1`. Installing Node.js on an administrator's workstation does not satisfy the target host's build requirement.

## 3. Obtain and build the source

For a fresh host, use a dedicated checkout:

```sh
install -d /opt
git clone https://github.com/lanthora/Keld.git /opt/keld
cd /opt/keld
git rev-parse HEAD
cat apps/go.mod | head -n 5
cat webui/package.json
make
```

If `/opt/keld` already exists, inspect its origin, revision, and working tree before reusing it; do not overwrite local changes. If the operator specifies a release or commit, check out that revision before building. Record the final source commit for the handover.

The build compiles the module and daemon, installs frontend dependencies from the lockfile, checks frontend types, embeds the frontend, and builds all Go applications. It must finish successfully and produce:

```text
core/kernel-space/keld.ko
core/user-space/build/keld
apps/cmd/web/keld-web
apps/cmd/telegram/keld-telegram
apps/cmd/notify/keld-notify
apps/cmd/sample/keld-sample
```

Verify the outputs and module compatibility:

```sh
test -s core/kernel-space/keld.ko
test -x core/user-space/build/keld
for app in web telegram notify sample; do
  test -x "apps/cmd/$app/keld-$app"
done
modinfo core/kernel-space/keld.ko
uname -r
```

The module's `vermagic` must match the running kernel's release and build requirements. `make web` builds only the frontend and web binary; it is insufficient for a complete deployment.

## 4. Install and start services

The installation script copies configuration templates into `/etc/keld/`. On an existing installation, back them up and stop the services before installing; restore the intended configuration afterward.

For a fresh installation, from the checkout:

```sh
make install
install -d -m 0700 /var/lib/keld
chmod 0700 /etc/keld
chmod 0600 /etc/keld/*.yaml
cat /etc/keld/web.yaml
```

The initial web configuration should be:

```yaml
db: "/var/lib/keld/web.db"
listen: "127.0.0.1:8080"
```

Check that port 8080 is available. If it is occupied, choose an available loopback port, edit `web.yaml`, and use that port consistently in subsequent commands.

```sh
ss -ltnp
modprobe keld
systemctl enable --now keld.service keld-web.service
systemctl is-enabled keld.service keld-web.service
systemctl is-active keld.service keld-web.service
ready=0
for attempt in $(seq 1 30); do
  if test -S /tmp/keld.sock && \
    curl --fail --silent http://127.0.0.1:8080/login >/dev/null; then
    ready=1
    break
  fi
  sleep 1
done
if test "$ready" -ne 1; then
  journalctl -u keld.service -u keld-web.service -n 100 --no-pager
  exit 1
fi
```

The core service loads the module before starting. `/etc/modules-load.d/keld.conf` also requests loading at boot. The web service requires the core service. Both installed system services run as root because they manage host protection and host files.

Installation paths:

| Purpose | Location |
| --- | --- |
| Module | `/lib/modules/$(uname -r)/keld.ko` |
| Executables | `/usr/bin/keld` and `/usr/bin/keld-{web,telegram,notify,sample}` |
| System services | `/usr/lib/systemd/system/keld{,-web,-telegram}.service` |
| Desktop user service | `/usr/lib/systemd/user/keld-notify.service` |
| Configuration | `/etc/keld/` |
| Core IPC socket | `/tmp/keld.sock` |
| Web database | `/var/lib/keld/web.db` |

## 5. Initialize the administrator and verify core connectivity

On an empty database, the first successful `/auth/login` request creates the administrator with full permissions. Initialize it locally before exposing the console. Passwords must contain 8–72 bytes. There is no preconfigured default password.

The following script is for a fresh installation. It generates a password, authenticates, checks the real core, signs out, and stores credentials in a root-only file without printing the password. If that file already exists, it reuses it. If an existing database has users but the credential file is missing, stop and obtain the existing administrator's credentials instead of attempting another bootstrap.

Run on the target host as root:

```sh
python3 - <<'PY'
import http.cookiejar
import json
import os
from pathlib import Path
import secrets
import sqlite3
import urllib.request

base = "http://127.0.0.1:8080"
credential_path = Path("/root/keld-admin-credentials.json")
database = Path("/var/lib/keld/web.db")
if credential_path.exists():
    credentials = json.loads(credential_path.read_text())
else:
    if database.exists():
        with sqlite3.connect(database) as db:
            exists = db.execute(
                "SELECT 1 FROM sqlite_master WHERE type='table' AND name='user'"
            ).fetchone()
            if exists and db.execute('SELECT count(*) FROM user').fetchone()[0]:
                raise SystemExit("Existing accounts found: obtain the administrator credentials.")
    credentials = {"username": "admin", "password": secrets.token_urlsafe(24)}
    fd = os.open(credential_path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
    with os.fdopen(fd, "w") as output:
        json.dump(credentials, output)
os.chmod(credential_path, 0o600)
client = urllib.request.build_opener(
    urllib.request.HTTPCookieProcessor(http.cookiejar.CookieJar())
)
def api(path, body=None):
    request = urllib.request.Request(
        base + path,
        data=json.dumps(body or {}).encode(),
        headers={"Content-Type": "application/json"},
    )
    with client.open(request, timeout=15) as response:
        result = json.load(response)
    if result["status"] != 0:
        raise SystemExit(f"{path}: {result.get('message', result['status'])}")
    return result.get("data")
api("/auth/login", credentials)
status = api("/ctrl/status")
if not status["coreOnline"]:
    raise SystemExit("Web authentication succeeded, but the core is offline.")
for module in ("process", "file", "net"):
    print(module, api(f"/{module}/showModuleStatus"))
api("/auth/logout")
print("Administrator initialized; authenticated core check passed.")
print(f"Credentials: {credential_path} (root only)")
PY
```

If the configuration uses a different port or database path, adjust those two values in the script. Hand over credentials through the user's approved secure channel; do not commit them or paste them into build logs.

## 6. Run acceptance checks

Build-time regression checks, from the checkout:

```sh
(cd webui && npm test)
(cd apps && go test ./...)
(cd apps && go test -race ./internal/web ./internal/worker)
```

On a dedicated test host, run the real kernel integration fixture. It verifies authentication, core IPC, filesystem browsing, process events and trust transitions, file audit and write blocking, network audit and connection blocking, policy operations, and event handling.

```sh
cd /opt/keld
python3 - <<'PY'
import json
import os
import subprocess
from pathlib import Path
credentials = json.loads(Path("/root/keld-admin-credentials.json").read_text())
environment = dict(
    os.environ,
    KELD_TEST_USERNAME=credentials["username"],
    KELD_TEST_PASSWORD=credentials["password"],
)
subprocess.run(
    ["python3", "scripts/integration-test.py", "http://127.0.0.1:8080", "--full"],
    env=environment,
    check=True,
)
PY
```

The full fixture changes global module settings, removes its own temporary policies, and leaves protection modules disabled. Run it before configuring production policies. On an existing production host, use the authenticated connectivity check and agreed, narrowly scoped enforcement tests instead; do not run the full fixture over active policies.

Before handover, confirm:

```sh
systemctl is-active keld.service keld-web.service
systemctl is-enabled keld.service keld-web.service
modinfo -F name keld
test -d /sys/module/keld
test -S /tmp/keld.sock
journalctl -u keld.service -u keld-web.service -n 50 --no-pager
```

Module status in the web interface reflects saved configuration. `coreOnline` confirms communication, not every policy's actual enforcement. Do not claim enforcement was tested unless a real operation was observed or blocked.

## 7. Provide access and hand over

For remote administration, an SSH tunnel keeps the service on loopback. Run this on the operator's workstation, replacing the target host:

```sh
ssh -N -L 8080:127.0.0.1:8080 root@TARGET_HOST
```

Open `http://127.0.0.1:8080` locally. If the workstation's port 8080 is occupied, use a different local port in the tunnel.

For a shared endpoint, configure the host's HTTPS reverse proxy to forward to the loopback listener and preserve the original Host. Keep browser requests same-origin and apply the deployment's access controls. Avoid exposing the first-login bootstrap over an untrusted connection.

The deployment handover must state:

- Target host, OS, running kernel, source commit, and module metadata.
- Service enablement and current state; access URL or tunnel instructions.
- Configuration/database paths and the secure location of administrator credentials.
- Which checks passed, which integrations remain unconfigured, and any unresolved restrictions.
- Actual protection mode and configured policies. Fresh installation leaves enforcement disabled; the operator can enable audit mode and review events before choosing defense policies.

## Optional integrations

**Telegram:** configure `/etc/keld/telegram.yaml` with the bot token, owner chat ID, and database path described by the template, then enable `keld-telegram.service`. Do not start an unconfigured bot.

**Desktop notifications:** run `keld-notify` in a supported graphical user session. Its default configuration is `~/.config/keld/notify.yaml`; copy and customize `apps/configs/notify.yaml`. Use the installed systemd user unit with `systemctl --user enable --now keld-notify.service` when the desktop environment supports it. The client connects to the configured web service and needs valid account credentials.

**Sample client:** `/usr/bin/keld-sample` consumes core process audit events. It requires the core and suitable process audit settings; it is not a service required by the console.

## Maintenance and troubleshooting

### Service or module startup fails

```sh
systemctl status keld.service keld-web.service --no-pager
journalctl -u keld.service -u keld-web.service -b --no-pager
dmesg | tail -n 100
modinfo keld
uname -r
ls -ld "/lib/modules/$(uname -r)/build"
```

- `Invalid module format`: check the running kernel, matching headers, and module `vermagic`; rebuild and reinstall for that kernel.
- Signature/key rejection or lockdown errors: satisfy the host's module-signing requirements.
- Missing kprobe/ftrace facilities or unsupported kernel changes: inspect the kernel configuration and compatibility code; do not force an incompatible module to load.
- Missing `/tmp/keld.sock` or failed core handshake: inspect core/module logs first, then restart the web service after the core is healthy.
- HTTP bind failures: resolve a port conflict or invalid listener configuration.
- Missing embedded assets: run `make web` or the full `make`, reinstall the web binary, and restart `keld-web.service`.
- Go/Node build failures: verify toolchain versions in the actual build environment and dependency registry access. Do not remove the lockfile to bypass an installation error.

### Update or rebuild after a kernel change

The default `make install` installs a module for the running kernel only. After changing kernels, rebuild and reinstall against the new running kernel's headers. The repository also supplies a DKMS configuration under `core/kernel-space/` for environments managing module rebuilds through DKMS.

For application updates, record the deployed commit, stop `keld-web.service`, optional `keld-telegram.service`, and `keld.service`, and back up `/etc/keld/`, `/var/lib/keld/`, and current binaries before replacing them. Copy SQLite databases only with their writers stopped. Build the intended revision, install, restore the intended configuration, start services, and repeat authenticated acceptance checks. `make install` overwrites configuration templates; do not discard the existing configuration.

### Uninstall

From the checkout:

```sh
make uninstall
```

This stops/disables the system services, unloads the module, and removes installed binaries, service files, and template configuration. Stop the notification user service separately in each user session where it was enabled. Databases under `/var/lib/keld/`, administrator credentials, and the source checkout are retained; remove them only when data removal is part of the requested operation.

## Further documentation

- [Web interface, API permissions, and localization](webui/README.md)
- [Core requirements](core/README.md)
- [Unix socket API](docs/api.md)
- [Architecture](docs/design.md)
- [Deployment validation and coverage limits](docs/validation.md)
- [License](LICENSE)
