cmake_minimum_required(VERSION 3.19)

#docopt
add_subdirectory(docopt)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/docopt)
target_link_libraries(Engine PRIVATE docopt)
install(TARGETS docopt)

#glfw
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_INSTALL OFF)

add_subdirectory(glfw)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include)
target_link_libraries(Engine PRIVATE glfw)
install(TARGETS glfw)

#add_subdirectory(glm)

#spdlog
add_subdirectory(spdlog)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include)
target_link_libraries(Engine PRIVATE spdlog)
install(TARGETS spdlog)

#vma
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vma/include)

#volk
set(VOLK_STATIC_DEFINES ${ENG_PLATFORM})
add_subdirectory(volk)
target_link_libraries(Engine PRIVATE volk)
install(TARGETS volk)

#entt
add_subdirectory(entt)
target_link_libraries(Engine PRIVATE EnTT::EnTT)
install(TARGETS EnTT)