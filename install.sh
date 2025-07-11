sudo apt install meson

mkdir subprojects
meson wrap sdl2
meson wrap install sdl2
meson wrap install sdl2_image
meson build
cd build
meson compile