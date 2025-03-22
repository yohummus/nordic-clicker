cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app)

target_sources(app PRIVATE
    src/battery.c
    src/bluetooth.c
    src/buttons.c
    src/leds.c
    src/main.c
    src/speaker.c
)
