# CoPPer server
Fast and extendable server implementation for lastest Minecraft versions

----

### How it's works

Currently, every part of the server is operated by plugins. The console, chat, predicates, world generation, and its management are all managed by plugins and accessed via the API. This allows any part of the server to be reimplemented without rewriting the entire server.
Additionally, some plugins can be removed, such as the console. If removed, command execution would no longer be available through the console. This provides high possibilities for extensibility and optimization.

### Compilation and running

The easiest way to compile is by using an IDE that supports `CMakePresets.json`. If so, use the Release configuration (optionally), then configure and build the project with CMake.

----

### Notice

NOT AN OFFICIAL MINECRAFT SERVER IMPLEMENTATION. NOT APPROVED BY OR ASSOCIATED WITH MOJANG OR MICROSOFT
