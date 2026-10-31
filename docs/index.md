# Features

## File protection

- Detect reads, writes, deletion, and renaming.
- Audit or block selected operations.
- Report events when a configured policy matches.

## Process protection

- Observe process creation and control command execution.
- Match working directory, executable path, and command arguments.
- Manage trusted and untrusted commands; configure audit or defense mode.
- Report process events and execution counts.

## Network protection

- Match IP protocol, source and destination addresses.
- Match transport ports, TCP handshake packets, and TCP packets without payload.
- Configure policy priority, direction, logging, and accept/drop behavior.

## Core communication

- Unix datagram IPC supports multiple asynchronous clients and event subscriptions.
- Optional tokens authenticate requests; invalid or unauthorized requests receive no response.
- The `extra` field correlates asynchronous requests and responses without depending on response order.

## Web management

The embedded Vue interface provides authentication, account permissions, protection configuration, event review, policy management, host file browsing, diagnostics, and English/Simplified Chinese localization.

## More information

- [Background](prepare.md)
- [Design](design.md)
- [Core API](api.md)
- [About](about.md)
