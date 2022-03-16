cmake_minimum_required(VERSION 3.0)

project(PG)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")


if (MSVC)
 add_compile_options(/W3 /WX)
else()
 set (CMAKE_CXX_STANDARD 14)
 add_compile_options(-W -Wall)
#  set(CMAKE_CXX_FLAGS "-O2 -std=c++0x -Wall")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -D_DEBUG")
endif()










add_subdirectory(librerias/guipg) #Mandatory
add_subdirectory(PGUPV) #Mandatory
add_subdirectory(ej3-1) 
add_subdirectory(ej3-2) 
add_subdirectory(ej3-3) 
add_subdirectory(ej3-4) 
add_subdirectory(ej2-1)
add_subdirectory(ej2-2)
add_subdirectory(ej2-3)
add_subdirectory(ej2-4)
add_subdirectory(ej2-5)
add_subdirectory(ej2-6)
add_subdirectory(gui) 
add_subdirectory(p2) 
add_subdirectory(ej1-1)
add_subdirectory(ej1-2)
add_subdirectory(ej1-3)
add_subdirectory(ej1-4)
add_subdirectory(p0)
add_subdirectory(p1)
