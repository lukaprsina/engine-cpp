cmake_minimum_required(VERSION 3.16)

project(engine)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake
     ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(global_options)

add_subdirectory(sandbox)
add_subdirectory(engine)