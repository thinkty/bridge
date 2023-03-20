
# Bridge

![screenshot](https://github.com/thinkty/bridge/blob/main/screenshot.png)

**Bridge** is a simple topic-based [pub/sub](https://en.wikipedia.org/wiki/Publish%E2%80%93subscribe_pattern) broker on top of TCP with a very simple application protocol.
It uses [ncurses](https://en.wikipedia.org/wiki/Ncurses) for the terminal UI. 
It utilizes linear probing hash map (defined in [table.h](https://github.com/thinkty/bridge/blob/main/include/table.h)) to insert/remove the topics, subscribe/unsubscribe new connections.
It is built on a TCP server that handles new connections in new threads.

This is just a personal project and has a lot of limitations (which could be future plans?).
Some of the limits that I can think of at the moment are:
- Spawning threads on new connection (instead of pool of threads)
- No encryption or integrity checks
- QoS level 0 (send at most once and if an error occurs, just unsubscribe)
- Not so interactive UI (only able to move in the table)
- Memory leaks (valgrind) within ncurses itself but this seems like a different [issue](https://invisible-island.net/ncurses/ncurses.faq.html#config_leaks)
- No options to set port, connections, etc.

## Install

Clone or download the repository and run `make` to create the executable *bridge*.

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

For publishing to a topic, the command has to be `P`, followed by 7 bytes to specify the topic to publish to. Then it is followed by variable length of data.

```
Command (1 byte) | Topic (7 bytes) | Data (variable length)
```

## License

MIT
