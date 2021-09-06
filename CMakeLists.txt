cmake_minimum_required(VERSION 3.19)

project(engine)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bldsys)

include(global_options)

add_subdirectory(engine)
