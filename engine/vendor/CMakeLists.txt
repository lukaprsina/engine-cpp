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
option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
option(BUILD_EXTERNAL "Build external dependencies in /External" ON)

add_subdirectory(glslang)
                
target_link_libraries(Engine PRIVATE glslang)
target_link_libraries(Engine PRIVATE SPIRV)
install(TARGETS glslang)
install(TARGETS SPIRV)

#spirv-cross
add_subdirectory(spirv-cross)
target_link_libraries(Engine PRIVATE spirv-cross-core)
target_link_libraries(Engine PRIVATE spirv-cross-core)
target_link_libraries(Engine PRIVATE spirv-cross-glsl)
target_link_libraries(Engine PRIVATE spirv-cross-cpp)
target_link_libraries(Engine PRIVATE spirv-cross-reflect)
target_link_libraries(Engine PRIVATE spirv-cross-msl)
target_link_libraries(Engine PRIVATE spirv-cross-hlsl)
target_link_libraries(Engine PRIVATE spirv-cross-util)
target_link_libraries(Engine PRIVATE spirv-cross-c)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross)

#thread_pool
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thread_pool)

#stb
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb)

#ktx-software
option( KTX_FEATURE_DOC "Create KTX documentation" OFF )
option( KTX_FEATURE_LOADTEST_APPS "Create load tests apps that load and display various KTX textures" OFF )
option( KTX_FEATURE_STATIC_LIBRARY "Create static libraries (shared otherwise)" ON )
option( KTX_FEATURE_TESTS "Create unit tests" OFF )

add_subdirectory(ktx-software)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ktx-software/include)
target_link_libraries(Engine PRIVATE ktx-software)


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
