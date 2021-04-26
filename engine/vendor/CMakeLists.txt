cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(vendor)

#docopt
add_subdirectory(docopt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/docopt)
target_link_libraries(Engine PRIVATE docopt)

#glfw
add_subdirectory(glfw)

#add_subdirectory(glm)

#spdlog
add_subdirectory(spdlog)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include)
target_link_libraries(Engine PRIVATE spdlog)


#vma

#volk
set(VOLK_STATIC_DEFINES ${ENG_PLATFORM})
add_subdirectory(volk)
target_link_libraries(Engine PRIVATE volk)
