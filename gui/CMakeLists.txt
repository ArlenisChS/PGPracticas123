cmake_minimum_required(VERSION 2.8)

project(gui)

add_executable(gui main.cpp)
target_link_libraries(gui PGUPV)

include(../PGUPV/pgupv.cmake)

set_target_properties( gui PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/bin 
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin
)

install(TARGETS gui DESTINATION ${PG_SOURCE_DIR}/bin)
