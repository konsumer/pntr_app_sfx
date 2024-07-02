This is a simple example.

Build with:

```
cmake -G Ninja -B build
cmake --build build
```

You can build for the web, too:

```
emcmake cmake -G Ninja -B build -DPLATFORM=Web
cmake --build build
```