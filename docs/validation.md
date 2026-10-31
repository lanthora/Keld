# Initial release validation

Validated on 2026-10-31 by SSH deployment to `root@10.200.200.6`.

## Environment

- Arch Linux, x86-64, Linux `7.2.11-arch1-1`.
- Go `1.27.3-X:nodwarf5`, Node.js `26.12.0`, npm `12.2.1`.
- Kernel module release: `0.1.0`; syscall hook backend: ftrace.

## Results

- Removed the previous application's services, loaded module, installation files, databases, source checkout, package/debug files, caches, and deployment backups before the first test.
- Built the kernel module, C++ daemon, all four Go applications, and embedded Vue assets from the new checkout.
- Passed frontend type checks and all 10 frontend tests, including English application messages, localization completeness, and protection controls.
- Passed `go test -buildvcs=false ./...`.
- Passed `go test -buildvcs=false -race ./internal/web ./internal/worker`.
- Passed full uninstall checks for binaries, notification service, and loaded module, followed by reinstall and service startup.
- Passed real kernel integration tests after reinstall: administrator sign-in, session information, core communication, host filesystem browsing, process audit and trust transitions, file read/write auditing and write blocking, network loopback auditing and connection blocking, policy operations, event review/deletion, and sign-out.
- Confirmed both `keld.service` and `keld-web.service` active and enabled, with module metadata reporting version `0.1.0`.
- Confirmed no previous project filenames remained under the inspected installation, data, source, runtime, and temporary directories.
- Confirmed all original tracked implementation files were transferred; the former raster system-call diagram was replaced with an English SVG and the Gentoo ebuild was assigned its initial release filename.
- Checked all 15 README shell examples for syntax, parsed embedded Python, verified relative documentation links, and executed the documented administrator/core verification against the deployed service.
- Confirmed Chinese text is confined to localization resources, language selection, and localization tests. Third-party npm lock entries are unchanged.

The integration fixture removes its policies and leaves process, file, and network modules disabled with the core online. The web listener uses the default `127.0.0.1:8080`. Test administrator credentials are stored only on the test host in `/root/keld-admin-credentials.json` with mode `0600`.

Telegram delivery and graphical desktop notification delivery require external configuration and were not exercised. Distribution package installation and GitHub-hosted CI were not executed; the workflow uses no repository secrets. Other kernel versions and CPU architectures were not tested.
