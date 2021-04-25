cmake_minimum_required(VERSION 3.20.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(vendor)

#add_subdirectory(docopt)
#add_subdirectory(glfw)
#add_subdirectory(glm)
#add_subdirectory(spdlog)
#vma

#volk
add_subdirectory(volk)
target_link_libraries(Engine PRIVATE volk)
