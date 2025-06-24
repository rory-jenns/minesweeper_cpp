Minesweeper C++ (with graphics)


SDL code heavily pulled from [Lazy Foo' Productions SDL Tutorial](https://lazyfoo.net/tutorials/SDL)

Textures sourced from `MS-Texture` github repo [linked here](https://github.com/Minesweeper-World/MS-Texture)

## Build from Source

Using meson as a build tool - https://mesonbuild.com/

```
mkdir subprojects
meson wrap sdl2
meson wrap install sdl2
meson wrap install sdl2_image
meson build
cd build
meson compile
```

Then to run:

```
cd build;
./minesweeper
```