cmake_minimum_required(VERSION 2.8)

project(ej2-3)

add_executable(ej2-3 main.cpp)
target_link_libraries(ej2-3 PGUPV)

include(../PGUPV/pgupv.cmake)

set_target_properties( ej2-3 PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/bin 
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin
)

install(TARGETS ej2-3 DESTINATION ${PG_SOURCE_DIR}/bin)
