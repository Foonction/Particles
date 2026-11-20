cmake_minimum_required(VERSION 3.15)
project(particle CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#################
if(WIN32)
    find_package(unofficial-pdcurses CONFIG REQUIRED)
else()
    find_package(Curses REQUIRED)
endif()
#################
add_executable(particle
    src/main.cpp
    include/Settings.h
    include/Particle.h
    include/Averange.h
    include/Physics.h
    #здесь перечисление файлов
)
target_link_libraries(particle PRIVATE
    $<$<PLATFORM_ID:Windows>:unofficial::pdcurses::pdcurses>
    $<$<NOT:$<PLATFORM_ID:Windows>>:${CURSES_LIBRARIES}>

)