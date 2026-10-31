# Keld core

Kernel-level host intrusion detection and prevention with a C++ user-space daemon.

## Requirements

- Architectures: AMD64, ARM, ARM64.
- Linux 5.10 or later, with matching kernel headers. Compatibility depends on available kernel tracing and syscall facilities.
- CMake, make, a C++20 compiler, [nlohmann-json](https://github.com/nlohmann/json), and [libnl](https://www.infradead.org/~tgr/libnl/doc/api/index.html#main_intro).

Build the complete project with `make` from the repository root. The kernel output is `core/kernel-space/keld.ko`; the daemon is `core/user-space/build/keld`. See the root README for installation and service management.

## License

[GPL-2.0-only](../LICENSE).
