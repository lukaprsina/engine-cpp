cmake_minimum_required(VERSION 3.19)

# vulkan-headers
add_subdirectory(vulkan-headers)
target_include_directories(
  Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-headers/include)
target_link_libraries(Engine PUBLIC Vulkan-Headers)
install(TARGETS Vulkan-Headers)

# volk
option(VOLK_PULL_IN_VULKAN "Vulkan as a transitive dependency" OFF)
option(VOLK_INSTALL "Create installation targets" OFF)

set(VOLK_STATIC_DEFINES ${ENG_PLATFORM})
add_subdirectory(volk)
target_link_libraries(volk PUBLIC Vulkan-Headers)
target_link_libraries(Engine PUBLIC volk)
install(TARGETS volk)

# stb TODO
target_compile_definitions(Engine PUBLIC STBI_NO_SIMD)
add_library(stb INTERFACE)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stb)

# docopt
option(WITH_TESTS "Build tests." OFF)
option(WITH_EXAMPLE "Build example." OFF)
option(USE_BOOST_REGEX "Replace std::regex with Boost.Regex" OFF)

add_subdirectory(docopt)
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/docopt)
target_link_libraries(Engine PRIVATE docopt)
install(TARGETS docopt)

# glfw
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_VULKAN_STATIC
       "Assume the Vulkan loader is linked with the application" OFF)

add_subdirectory(glfw)
target_include_directories(Engine
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include)
target_link_libraries(Engine PRIVATE glfw)
install(TARGETS glfw)

# glm
add_subdirectory(glm)

target_compile_definitions(
  glm
  INTERFACE GLM_FORCE_SWIZZLE GLM_FORCE_RADIANS GLM_FORCE_CTOR_INIT
            GLM_ENABLE_EXPERIMENTAL GLM_FORCE_DEPTH_ZERO_TO_ONE
            GLM_FORCE_QUAT_DATA_XYZW)

target_compile_definitions(
  Engine
  PUBLIC GLM_FORCE_SWIZZLE GLM_FORCE_RADIANS GLM_FORCE_CTOR_INIT
         GLM_ENABLE_EXPERIMENTAL GLM_FORCE_DEPTH_ZERO_TO_ONE
         GLM_FORCE_QUAT_DATA_XYZW)

target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/glm)
target_link_libraries(Engine PRIVATE glm)
install(TARGETS glm)

# spdlog
option(SPDLOG_BUILD_ALL "Build all artifacts" OFF)
option(SPDLOG_BUILD_SHARED "Build shared library" OFF)
option(
  SPDLOG_ENABLE_PCH
  "Build static or shared library using precompiled header to speed up compilation time"
  ON)
option(SPDLOG_BUILD_EXAMPLE "Build example" OFF)
option(SPDLOG_BUILD_EXAMPLE_HO "Build header only example" OFF)
option(SPDLOG_BUILD_TESTS "Build tests" OFF)
option(SPDLOG_BUILD_TESTS_HO "Build tests using the header only version" OFF)

add_subdirectory(spdlog)
target_include_directories(Engine
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include)
target_link_libraries(Engine PRIVATE spdlog)
install(TARGETS spdlog)

# vma
option(VMA_BUILD_SAMPLE "Build VulkanMemoryAllocator sample application" OFF)
option(VMA_BUILD_SAMPLE_SHADERS
       "Build VulkanMemoryAllocator sample application's shaders" OFF)
option(VMA_BUILD_REPLAY "Build VulkanMemoryAllocator replay application" OFF)

add_subdirectory(vma)
target_include_directories(Engine
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vma/include)

# entt
option(ENTT_BUILD_TESTING "Enable building tests." OFF)
option(ENTT_BUILD_DOCS "Enable building with documentation." OFF)

add_subdirectory(entt)
target_link_libraries(Engine PRIVATE EnTT::EnTT)
install(TARGETS EnTT)

# glslang
option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
option(BUILD_EXTERNAL "Build external dependencies in /External" ON)

add_subdirectory(glslang)

if(NOT TARGET glslang-default-resource-limits)
  add_library(glslang-default-resource-limits
              glslang/StandAlone/ResourceLimits.cpp)

  target_include_directories(
    glslang-default-resource-limits
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glslang/StandAlone)

endif()

target_link_libraries(Engine PRIVATE glslang-default-resource-limits)
target_link_libraries(Engine PRIVATE glslang)
target_link_libraries(Engine PRIVATE SPIRV)
install(TARGETS glslang-default-resource-limits)
install(TARGETS glslang)
install(TARGETS SPIRV)

# spirv-cross
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
target_include_directories(Engine
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross)

# thread_pool
target_compile_definitions(Engine PRIVATE _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)
# 'std::result_of<engine::GLTFLoader::LoadImages::<lambda_290143bf62d359265eaa393944e725e9> (void)>': warning STL4014: std::result_of and std::result_of_t are deprecated in C++17. They are superseded by std::invoke_result and std::invoke_result_t. You can define _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING or _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received this warning. [D:\dev\c++\engine\build\engine\Engine.vcxproj]
target_include_directories(Engine
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thread_pool)

# ktx-software
option(KTX_FEATURE_DOC "Create KTX documentation" OFF)
option(KTX_FEATURE_LOADTEST_APPS
       "Create load tests apps that load and display various KTX textures" OFF)
option(KTX_FEATURE_STATIC_LIBRARY "Create static libraries (shared otherwise)"
       ON)
option(KTX_FEATURE_TESTS "Create unit tests" OFF)

add_subdirectory(ktx-software)
target_include_directories(
  Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ktx-software/include)
target_link_libraries(Engine PRIVATE ktx)

# astc-encoder add_subdirectory(astc-encoder) target_include_directories( Engine
# PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/astc-encoder/Source)

# astc
set(ASTC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/astc-encoder)
set(ASTC_INCLUDE_DIR ${ASTC_DIR}/Source)
set(ASTC_SOURCE_DIR ${ASTC_DIR}/Source)

set(ASTC_SOURCES
    ${ASTC_SOURCE_DIR}/astc_image_load_store.cpp
    ${ASTC_SOURCE_DIR}/astc_pick_best_endpoint_format.cpp
    ${ASTC_SOURCE_DIR}/astc_color_quantize.cpp
    ${ASTC_SOURCE_DIR}/astc_weight_align.cpp
    ${ASTC_SOURCE_DIR}/astc_integer_sequence.cpp
    ${ASTC_SOURCE_DIR}/astc_ideal_endpoints_and_weights.cpp
    ${ASTC_SOURCE_DIR}/astc_find_best_partitioning.cpp
    ${ASTC_SOURCE_DIR}/astc_compress_symbolic.cpp
    ${ASTC_SOURCE_DIR}/astc_decompress_symbolic.cpp
    ${ASTC_SOURCE_DIR}/astc_symbolic_physical.cpp
    ${ASTC_SOURCE_DIR}/astc_toplevel.cpp
    ${ASTC_SOURCE_DIR}/astc_stb_tga.cpp
    ${ASTC_SOURCE_DIR}/softfloat.cpp
    ${ASTC_SOURCE_DIR}/mathlib.cpp
    ${ASTC_SOURCE_DIR}/astc_quantization.cpp
    ${ASTC_SOURCE_DIR}/astc_block_sizes2.cpp
    ${ASTC_SOURCE_DIR}/astc_percentile_tables.cpp
    ${ASTC_SOURCE_DIR}/astc_averages_and_directions.cpp
    ${ASTC_SOURCE_DIR}/astc_partition_tables.cpp
    ${ASTC_SOURCE_DIR}/astc_color_unquantize.cpp
    ${ASTC_SOURCE_DIR}/astc_encoding_choice_error.cpp
    ${ASTC_SOURCE_DIR}/astc_kmeans_partitioning.cpp
    ${ASTC_SOURCE_DIR}/astc_weight_quant_xfer_tables.cpp
    ${ASTC_SOURCE_DIR}/astc_compute_variance.cpp
    ${ASTC_SOURCE_DIR}/astc_ktx_dds.cpp)

add_library(astc ${ASTC_SOURCES})
target_include_directories(astc PUBLIC ${ASTC_INCLUDE_DIR})
target_compile_definitions(astc PRIVATE -DNO_STB_IMAGE_IMPLEMENTATION)

target_link_libraries(Engine PRIVATE astc)

# tinygltf
option(TINYGLTF_BUILD_LOADER_EXAMPLE
       "Build loader_example(load glTF and dump infos)" OFF)
option(TINYGLTF_BUILD_GL_EXAMPLES
       "Build GL exampels(requires glfw, OpenGL, etc)" OFF)
option(TINYGLTF_BUILD_VALIDATOR_EXAMPLE "Build validator exampe" OFF)
option(TINYGLTF_BUILD_BUILDER_EXAMPLE "Build glTF builder example" OFF)
option(
  TINYGLTF_HEADER_ONLY
  "On: header-only mode. Off: create tinygltf library(No TINYGLTF_IMPLEMENTATION required in your project)"
  OFF)
option(
  TINYGLTF_INSTALL
  "Install tinygltf files during install step. Usually set to OFF if you include tinygltf through add_subdirectory()"
  OFF)

add_subdirectory(tinygltf)
target_link_libraries(Engine PRIVATE tinygltf)
install(TARGETS tinygltf)
