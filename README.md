# CoPPer server
Fast and extendable server implementation for latest Minecraft versions


### How it's works
__*this would be replaced with wiki link*__

Every part of the server is operated by plugins. The console, chat, predicates, world generation and any other thing is managed by plugins and accessed via the API. This allows any part of the server to be reimplemented or removed altogether without rewriting the entire server.

The server is asynchronously multithreaded.

### Compilation and running
Requirements:
* ninja
* default c++ compiler, if your differs from `CMakePresets.json` specify own in this file
* java dev kit 21 version(for data extraction)
* cmake
* make libtool autoconf unzip wget pkg-config(for vcpkg linux deps)

_on linux you may also run `chmod +x ./tools/extractor/gradlew` to enable gradle used for extraction_

After running cmake would be compiled and run tools for data extraction, reflection and for embedding the resources.
On first configure it would take some time.

After completing there would be resources folder.

Compile and run server.
### Notice

NOT AN OFFICIAL MINECRAFT SERVER IMPLEMENTATION. NOT APPROVED BY OR ASSOCIATED WITH MOJANG OR MICROSOFT

### SAST Tools

[PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.