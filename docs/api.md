# Core API

The core exposes an AF_UNIX SOCK_DGRAM socket at /tmp/keld.sock.
Requests are limited to 1 KB. Oversized or invalid requests receive no response.
Up to 1024 pending requests are supported; excess connections are evicted using LRU.

## Testing

Use openbsd-netcat to verify the socket API. The response below confirms connectivity.
Press Ctrl+C to exit. Unix domain socket libraries are also available in most languages.

```bash
# Open a socket and send {"type":"user::test::echo"}
nc -uU /tmp/keld.sock <<< '{"type":"user::test::echo"}'
# Expected response: {"extra":null,"type":"user::test::echo"}
{"extra":null,"type":"user::test::echo"}
```

The remaining examples show request and response payloads without repeating socket setup.

## Correlation fields

Requests and responses are asynchronous.
Any request may include an application-defined "extra" field.
Its value is returned unchanged in the response for correlation.
It may hold any valid JSON value; omitting it produces null in the response.
Use a unique "extra" value to match a response to a request. Response order is not guaranteed.
The response "code" describes the result; 0 indicates success.

No extra value:

```json
{
    "type": "user::test::echo"
}
```

A scalar value:

```json
{
    "type": "user::test::echo",
    "extra": 1
}
```

An array:

```json
{
    "type": "user::test::echo",
    "extra": [
        "Alice",
        "Bob"
    ]
}
```

An object:

```json
{
    "type": "user::test::echo",
    "extra": {
        "reply_to": "Alice"
    }
}
```

## Control

### Set a token

After setting a token, subsequent requests must include a token field.
Other examples omit this field; add it when token authentication is enabled.

```json
{
    "type": "user::ctrl::token",
    "new": "this is the token"
}
```

Changing a token requires the previous token in the request.
The two most recent tokens remain valid to accommodate clients rotating tokens at different times.

```json
{
    "type": "user::ctrl::token",
    "token": "this is the token",
    "new": "this is another token"
}
```

### Stop the daemon

```json
{
    "type": "user::ctrl::exit"
}
```

## Processes

### Enable process protection

```json
{
    "type": "user::proc::enable"
}
```

### Subscribe to process creation events

Run a command after subscribing to receive a process event.

```json
{
    "type": "user::msg::sub",
    "section": "kernel::proc::report"
}
```

The subscription continues delivering process creation events.
The "cmd" field uses \u001f as a separator between:
working directory, executable path, and command arguments. This example uses ls.

```json
{
    "type": "kernel::proc::report",
    "cmd": "/root\u001f/usr/bin/ls\u001fls"
}
```

### Unsubscribe from process creation events

```json
{
    "type": "user::msg::unsub",
    "section": "kernel::proc::report"
}
```

### Subscribe to process audit events

Audit events use the same subscription and unsubscription operations.
Only the section value differs; other event sections follow the same mechanism.
A process may subscribe to the same section more than once.
Each event is delivered once. Delivery stops after matching unsubscriptions or a disconnected client.

```json
{
    "type": "user::msg::sub",
    "section": "audit::proc::report"
}
```

### Set the process decision mode

The judge value controls commands outside the trusted set, described below.

|Value|Meaning|
|-|-|
|0|Allow execution without reporting; disable decisions|
|1|Allow execution and report audit events|
|2|Deny execution and report defense events|

Audit events use the `"section":"audit::proc::report"` subscription.
With an empty trusted set, `"judge": 2` prevents new processes from starting.

```json
{
    "type": "user::proc::judge",
    "judge": 1
}
```

### Add a trusted command

The "cmd" value must exactly match the command from a process audit event.

```json
{
    "type": "user::proc::trusted::insert",
    "cmd": "/root\u001f/usr/bin/ls\u001fls"
}
```

### Remove a trusted command

```json
{
    "type": "user::proc::trusted::delete",
    "cmd": "/root\u001f/usr/bin/ls\u001fls"
}
```

### Clear trusted commands

```json
{
    "type": "user::proc::trusted::clear"
}
```

### Disable process protection

```json
{
    "type": "user::proc::disable"
}
```

## Files

### Enable file protection

```json
{
    "type": "user::file::enable"
}
```

### Set a file policy

The signed 32-bit "perm" mask currently uses eight bits, listed from least significant to most significant.

|Bit position|Meaning|
|-|-|
|1|Block reads|
|2|Block writes|
|3|Block deletion|
|4|Block renaming|
|5|Audit reads|
|6|Audit writes|
|7|Audit deletion|
|8|Audit renaming|

The example value 14 (decimal), or 00001110 (binary), blocks writing, deletion, and renaming for read-only access.
A "perm" value of 0 removes the file policy.

The "flag" value selects the update operation.

|Value|Meaning|
|-|-|
|0|Update if present; otherwise insert|
|1|Insert only; fail if a policy already exists for the path|
|2|Update only; fail if no policy exists for the path|


```json
{
    "type": "user::file::set",
    "path": "/etc/fstab",
    "perm": 14,
    "flag": 0
}
```

### Subscribe to file protection events

```json
{
    "type": "user::msg::sub",
    "section": "kernel::file::report"
}
```

Events are delivered when file operations match configured policies. Fields have the same meanings as in policy configuration.

```json
{
    "type": "kernel::file::report",
    "name": "/etc/fstab",
    "perm": 4
}
```

### Clear file policies

```json
{
    "type": "user::file::clear"
}
```

### Disable file protection

Disabling protection preserves policies. They take effect again when protection is enabled.

```json
{
    "type": "user::file::disable"
}
```

## Networks

### Enable network protection

```json
{
    "type": "user::net::enable"
}
```

### Insert a network policy

|Field|Type|Purpose|
|:-|:-|:-|
|id|s32|Identifier used for deletion|
|priority|s8|Lower values have higher priority; avoid relying on ordering between equal priorities|
|flags|s32|Bit mask: 1 inbound, 2 outbound, 4 handshake packets only, 8 TCP packets without payload only|
|response|u32|Bit 0 allows traffic; bit 1 reports events. For example, 0 silently drops and 3 allows with reporting|
|protocol|u8|IP protocol number, such as TCP 6 or ICMP 1|

All ranges specified by "begin" and "end" include both endpoints.

```json
{
    "type": "user::net::insert",
    "id": 0,
    "priority": 0,
    "addr": {
        "src": {
            "begin": "0.0.0.0",
            "end": "255.255.255.255"
        },
        "dst": {
            "begin": "0.0.0.0",
            "end": "255.255.255.255"
        }
    },
    "protocol": {
        "begin": 6,
        "end": 6
    },
    "port": {
        "src": {
            "begin": 0,
            "end": 65535
        },
        "dst": {
            "begin": 22,
            "end": 22
        }
    },
    "flags": 1,
    "response": 1
}
```

### Remove a network policy

```json
{
    "type": "user::net::delete",
    "id": 0
}
```

### Clear network policies

```json
{
    "type": "user::net::clear"
}
```

### Subscribe to network policy match events

```json
{
    "type": "user::msg::sub",
    "section": "kernel::net::report"
}
```

### Disable network protection

Disabling protection preserves network policies. They take effect again when protection is enabled.

```json
{
    "type": "user::net::disable"
}
```
