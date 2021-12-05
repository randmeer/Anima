cmake_minimum_required(VERSION 3.20)
project(Anima)

set(CMAKE_CXX_STANDARD 14)

add_executable(Anima main.cpp)

target_include_directories(Anima PRIVATE /usr/local/Cellar/sdl2/2.0.18/include/SDL2)
target_link_libraries(Anima PRIVATE /usr/local/Cellar/sdl2/2.0.18/lib/libSDL2.dylib)

target_include_directories(Anima PRIVATE /usr/local/Cellar/sdl2_image/2.0.5/include/SDL2)
target_link_libraries(Anima PRIVATE /usr/local/Cellar/sdl2_image/2.0.5/lib/libSDL2_image.dylib)