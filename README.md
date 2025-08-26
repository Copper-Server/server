# CoPPer server
Fast and extendable server implementation for latest Minecraft versions


### How it's works

Every part of the server is operated by plugins. The console, chat, predicates, world generation and any other thing is managed by plugins and accessed via the API. This allows any part of the server to be reimplemented or removed altogether without rewriting the entire server.

The server is asynchronously multithreaded.

### Compilation and running

The easiest way to compile is by using an IDE that supports `CMakePresets.json`. If so, use the Release configuration (or Debug), then configure and build the project with CMake.

---

### Notice

NOT AN OFFICIAL MINECRAFT SERVER IMPLEMENTATION. NOT APPROVED BY OR ASSOCIATED WITH MOJANG OR MICROSOFT

the `resources` folder is not licensed and does not belong to anyone associated with maintainer/s of this repository, all data here is generated using data generators and custom extractors.

## SAST Tools

[PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.