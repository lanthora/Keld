# Keld daemon

The daemon is written in C++, with C used for components where it is a better fit. It bridges the kernel Generic Netlink interface and application Unix datagram clients.

## Dependencies

- [nlohmann-json](https://github.com/nlohmann/json)
- [libnl](https://www.infradead.org/~tgr/libnl/doc/api/index.html#main_intro)

## TODO

1. Restore runtime state after a restart.
2. Add default configuration.
