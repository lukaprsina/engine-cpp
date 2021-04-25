cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(vendor)

#add_subdirectory(docopt)

#glfw
add_subdirectory(glfw)

#add_subdirectory(glm)
#add_subdirectory(spdlog)
#vma

#volk
set(VOLK_STATIC_DEFINES ${ENG_PLATFORM})
add_subdirectory(volk)
target_link_libraries(Engine PRIVATE volk)
