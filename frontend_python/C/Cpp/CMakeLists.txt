cmake_minimum_required(VERSION 3.16)
project(handheld_launcher_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_executable(handheld_launcher
    main.cpp
    launcher/app.cpp
    launcher/core/config.cpp
    launcher/core/input.cpp
    launcher/core/library.cpp
    launcher/core/sdl_manager.cpp
    launcher/core/state.cpp
    launcher/core/assets.cpp
    launcher/ui/splash.cpp
    launcher/ui/main_menu.cpp
    launcher/ui/game_list.cpp
)

target_include_directories(handheld_launcher PRIVATE .)

target_link_libraries(handheld_launcher PRIVATE
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
)
