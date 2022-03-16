cmake_minimum_required(VERSION 2.8)

project(p3)

add_executable(p3 main.cpp)
target_link_libraries(p3 PGUPV)

include(../PGUPV/pgupv.cmake)

set_target_properties( p3 PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/bin 
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin
)

install(TARGETS p3 DESTINATION ${PG_SOURCE_DIR}/bin)
