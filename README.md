
# Bridge

**Bridge** is a simple topic-based [pub/sub](https://en.wikipedia.org/wiki/Publish%E2%80%93subscribe_pattern) broker on top of TCP.

## Protocol

This is a pub/sub protocol based on TCP with focus on simplicity and readability.
Every scenario begins with the 1 byte of command to indicate operation.

### Subscribe

For subscribing to a new topic, the command has to be `S`, followed by 7 bytes to specify the topic.

```
Command (1 byte) | Topic (7 bytes)
```

### Unsubscribe

For unsubscribing to a topic, the command has to be `U`, followed by 7 bytes to specify the topic to unsubscribe.

```
Command (1 byte) | Topic (7 bytes)
```

### Publish

For publishing to a topic, the command has to be `P`, followed by 7 bytes to specify the topic to publish to. Then the 4 bytes indicate length of the payload in bytes.

```
Command (1 byte) | Topic (7 bytes) | Length (4 bytes) | Data
```
