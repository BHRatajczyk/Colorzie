cmake_minimum_required(VERSION 3.26)
project(Colorzie)

set(CMAKE_CXX_STANDARD 14)

add_executable(Colorzie
        Colorzie/config/config.txt
        Colorzie/config/settings.txt
        Colorzie/maps/map1.txt
        Colorzie/maps/map10.txt
        Colorzie/maps/map11.txt
        Colorzie/maps/map12.txt
        Colorzie/maps/map13.txt
        Colorzie/maps/map2.txt
        Colorzie/maps/map3.txt
        Colorzie/maps/map4.txt
        Colorzie/maps/map5.txt
        Colorzie/maps/map6.txt
        Colorzie/maps/map7.txt
        Colorzie/maps/map8.txt
        Colorzie/maps/map9.txt
        Colorzie/Colorzie.depend
        Colorzie/Colorzie.layout
        Colorzie/main.cpp)
