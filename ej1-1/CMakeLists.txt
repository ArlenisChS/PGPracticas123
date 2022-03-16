cmake_minimum_required(VERSION 2.8)

project(ej1-1)

add_executable(ej1-1 main.cpp)
target_link_libraries(ej1-1 PGUPV)

include(../PGUPV/pgupv.cmake)

set_target_properties( ej1-1 PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/bin 
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin
)

install(TARGETS ej1-1 DESTINATION ${PG_SOURCE_DIR}/bin)
