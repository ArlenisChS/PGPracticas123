cmake_minimum_required(VERSION 2.8)

project(ej2-5)

add_executable(ej2-5 main.cpp)
target_link_libraries(ej2-5 PGUPV)

include(../PGUPV/pgupv.cmake)

set_target_properties( ej2-5 PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/bin 
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin
)

install(TARGETS ej2-5 DESTINATION ${PG_SOURCE_DIR}/bin)
