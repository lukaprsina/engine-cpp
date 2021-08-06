cmake_minimum_required(VERSION 3.19)

#docopt
option(WITH_TESTS "Build tests." OFF)
option(WITH_EXAMPLE "Build example." OFF)
option(USE_BOOST_REGEX "Replace std::regex with Boost.Regex" OFF)

add_subdirectory(docopt)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/docopt)
target_link_libraries(Engine PRIVATE docopt)
install(TARGETS docopt)

#glfw
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_VULKAN_STATIC "Assume the Vulkan loader is linked with the application" OFF)

add_subdirectory(glfw)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include)
target_link_libraries(Engine PRIVATE glfw)
install(TARGETS glfw)

#glm
add_subdirectory(glm)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/glm)
target_link_libraries(Engine PRIVATE glm)
install(TARGETS glm)

#spdlog
option(SPDLOG_BUILD_ALL "Build all artifacts" OFF)
option(SPDLOG_BUILD_SHARED "Build shared library" OFF)
option(SPDLOG_ENABLE_PCH "Build static or shared library using precompiled header to speed up compilation time" ON)
option(SPDLOG_BUILD_EXAMPLE "Build example" OFF)
option(SPDLOG_BUILD_EXAMPLE_HO "Build header only example" OFF)
option(SPDLOG_BUILD_TESTS "Build tests" OFF)
option(SPDLOG_BUILD_TESTS_HO "Build tests using the header only version" OFF)

add_subdirectory(spdlog)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include)
target_link_libraries(Engine PRIVATE spdlog)
install(TARGETS spdlog)

#vma
option(VMA_BUILD_SAMPLE "Build VulkanMemoryAllocator sample application" OFF)
option(VMA_BUILD_SAMPLE_SHADERS "Build VulkanMemoryAllocator sample application's shaders" OFF)
option(VMA_BUILD_REPLAY "Build VulkanMemoryAllocator replay application" OFF)

add_subdirectory(vma)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vma/include)

#volk
option(VOLK_PULL_IN_VULKAN "Vulkan as a transitive dependency" OFF)
option(VOLK_INSTALL "Create installation targets" OFF)

set(VOLK_STATIC_DEFINES ${ENG_PLATFORM})
add_subdirectory(volk)
target_link_libraries(Engine PRIVATE volk)
install(TARGETS volk)

#entt
option(ENTT_BUILD_TESTING "Enable building tests." OFF)
option(ENTT_BUILD_DOCS "Enable building with documentation." OFF)

add_subdirectory(entt)
target_link_libraries(Engine PRIVATE EnTT::EnTT)
install(TARGETS EnTT)

#glslang
option(ENABLE_SPVREMAPPER OFF)
option(ENABLE_GLSLANG_BINARIES OFF)
option(ENABLE_HLSL OFF)
option(ENABLE_OPT OFF)
option(BUILD_TESTING OFF)
option(BUILD_EXTERNAL OFF)

add_subdirectory(glslang)
target_link_libraries(Engine PRIVATE glslang)
install(TARGETS glslang)

#spirv-cross
add_subdirectory(spirv-cross)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross)


#tinygltf
option(TINYGLTF_BUILD_LOADER_EXAMPLE "Build loader_example(load glTF and dump infos)" OFF)
option(TINYGLTF_BUILD_GL_EXAMPLES "Build GL exampels(requires glfw, OpenGL, etc)" OFF)
option(TINYGLTF_BUILD_VALIDATOR_EXAMPLE "Build validator exampe" OFF)
option(TINYGLTF_BUILD_BUILDER_EXAMPLE "Build glTF builder example" OFF)
option(TINYGLTF_HEADER_ONLY "On: header-only mode. Off: create tinygltf library(No TINYGLTF_IMPLEMENTATION required in your project)" OFF)
option(TINYGLTF_INSTALL "Install tinygltf files during install step. Usually set to OFF if you include tinygltf through add_subdirectory()" OFF)

add_subdirectory(tinygltf)
target_link_libraries(Engine PRIVATE tinygltf)
install(TARGETS tinygltf)
