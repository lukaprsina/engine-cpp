cmake_minimum_required(VERSION 3.19)

set(ENG_VENDOR_SUBMODULE_LIST "")
set(ENG_VENDOR_NOT_FOUND_LIST "")

macro(create_dependency)
  set(options NO_CMAKE)
  set(oneValueArgs PATH INCLUDE LIB PACKAGE)
  set(multiValueArgs)

  cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_PATH}/.git)
    if(${TARGET_NO_CMAKE})
      add_library(${TARGET_PATH} INTERFACE)
      set(TARGET_LIB ${TARGET_PATH})
    else()
      add_subdirectory(${TARGET_PATH})
    endif()

    set(${TARGET_PATH}_FOUND TRUE)
    set(${TARGET_PATH}_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_PATH}/${TARGET_INCLUDE})
    set(${TARGET_PATH}_LIBRARIES ${TARGET_LIB})

    list(APPEND ENG_VENDOR_SUBMODULE_LIST ${TARGET_PATH})
  elseif(NOT "${TARGET_PACKAGE}" STREQUAL "")
    find_package(${TARGET_PACKAGE})

    if("${TARGET_PACKAGE}" STREQUAL "Spdlog")
      string(TOUPPER ${TARGET_PACKAGE} TARGET_PACKAGE)
    endif()

    set(${TARGET_PATH}_FOUND ${${TARGET_PACKAGE}_FOUND})
    set(${TARGET_PATH}_INCLUDE_DIRS ${${TARGET_PACKAGE}_INCLUDE_DIRS})
    set(${TARGET_PATH}_LIBRARIES ${${TARGET_PACKAGE}_LIBRARIES})

    if("${${TARGET_PACKAGE}_INCLUDE_DIRS}" STREQUAL ""
       AND NOT "${${TARGET_PACKAGE}_INCLUDE_DIR}" STREQUAL "")
      list(APPEND ${TARGET_PATH}_INCLUDE_DIRS ${${TARGET_PACKAGE}_INCLUDE_DIR})
    endif()

    if(${${TARGET_PACKAGE}_FOUND})
      list(APPEND ENG_VENDOR_SUBMODULE_LIST ${TARGET_PATH})
    else()
      list(APPEND ENG_VENDOR_NOT_FOUND_LIST ${TARGET_PATH})
    endif()

    message(
      ${TARGET_PATH}
      ": "
      ${${TARGET_PATH}_FOUND}
      ", "
      ${${TARGET_PATH}_INCLUDE_DIRS}
      ", "
      ${${TARGET_PATH}_LIBRARIES})

  else()
    list(APPEND ENG_VENDOR_NOT_FOUND_LIST ${TARGET_PATH})
  endif()
endmacro()

# vulkan
create_dependency(
  PATH
  "vulkan"
  INCLUDE
  "include"
  LIB
  "Vulkan-Headers"
  PACKAGE
  "Vulkan")
target_include_directories(engine PRIVATE ${vulkan_INCLUDE_DIRS})
target_link_libraries(engine PRIVATE ${vulkan_LIBRARIES})

# volk
option(VOLK_PULL_IN_VULKAN "Vulkan as a transitive dependency" OFF)

create_dependency(PATH "volk" LIB "volk")
target_link_libraries(${volk_LIBRARIES} PRIVATE ${vulkan-headers_LIBRARIES})
target_link_libraries(engine PRIVATE ${volk_LIBRARIES})

# stb TODO
create_dependency(PATH "stb" NO_CMAKE)
target_compile_definitions(${stb_LIBRARIES} INTERFACE STBI_NO_SIMD)
target_include_directories(engine PRIVATE ${stb_INCLUDE_DIRS})
target_link_libraries(engine PRIVATE ${stb_LIBRARIES})

# docopt
create_dependency(PATH "docopt" LIB "docopt")
target_include_directories(engine PRIVATE ${docopt_INCLUDE_DIRS})
target_link_libraries(engine PRIVATE ${docopt_LIBRARIES})

# glfw
option(GLFW_BUILD_DOCS OFF)
option(GLFW_INSTALL ON)
option(GLFW_VULKAN_STATIC OFF)

create_dependency(
  PATH
  "glfw"
  INCLUDE
  "include"
  LIB
  "glfw"
  PACKAGE
  "GLFW")
target_include_directories(engine PRIVATE ${glfw_INCLUDE_DIRS})
target_link_libraries(engine PRIVATE ${glfw_LIBRARIES})

# glm
create_dependency(PATH "glm" LIB "glm" PACKAGE "GLM")
target_compile_definitions(
  engine PRIVATE GLM_FORCE_SWIZZLE GLM_FORCE_RADIANS GLM_FORCE_CTOR_INIT
                 GLM_ENABLE_EXPERIMENTAL GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(engine PRIVATE ${glm_INCLUDE_DIRS})

# spdlog TODO definitions
option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" ON)
option(SPDLOG_FMT_EXTERNAL_HO
       "Use external fmt header-only library instead of bundled" ON)
create_dependency(
  PATH
  "spdlog"
  INCLUDE
  "include"
  LIB
  "spdlog"
  PACKAGE
  "Spdlog")
target_compile_definitions(engine PRIVATE SPDLOG_FMT_EXTERNAL)
target_include_directories(engine PRIVATE ${spdlog_INCLUDE_DIRS})
target_link_libraries(engine PRIVATE fmt)
target_link_libraries(engine PRIVATE ${spdlog_LIBRARIES})

# vma
create_dependency(PATH "vma" INCLUDE "include")
target_include_directories(engine PRIVATE ${vma_INCLUDE_DIRS})

# entt
create_dependency(PATH "entt" INCLUDE "src" LIB "EnTT::EnTT")
target_link_libraries(engine PRIVATE EnTT::EnTT)

# glslang
option(BUILD_EXTERNAL "Build external dependencies in /External" OFF)

create_dependency(PATH "glslang" LIB "glslang")
target_include_directories(engine PRIVATE ${glslang_INCLUDE_DIRS})
target_link_libraries(engine PRIVATE ${glslang_LIBRARIES})

if(NOT TARGET glslang-default-resource-limits)
  add_library(glslang-default-resource-limits
              glslang/StandAlone/ResourceLimits.cpp)

  target_include_directories(
    glslang-default-resource-limits
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glslang/StandAlone)

endif()

target_link_libraries(engine PRIVATE glslang-default-resource-limits)
target_link_libraries(engine PRIVATE glslang)
target_link_libraries(engine PRIVATE SPIRV)

# spirv-cross
create_dependency(PATH "spirv-cross")
target_include_directories(engine PRIVATE ${spirv-cross_INCLUDE_DIRS})
target_link_libraries(
  engine
  PRIVATE spirv-cross-glsl
          spirv-cross-cpp
          spirv-cross-reflect
          spirv-cross-msl
          spirv-cross-hlsl
          spirv-cross-util
          spirv-cross-c)
target_link_libraries(engine PRIVATE ${spirv-cross_LIBRARIES})

# thread_pool
create_dependency(PATH "thread_pool" NO_CMAKE)
target_compile_definitions(engine
                           PRIVATE _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)

target_include_directories(engine PRIVATE ${thread_pool_INCLUDE_DIRS})

# ktx
option(KTX_FEATURE_STATIC_LIBRARY ON)
option(KTX_FEATURE_TESTS OFF)

create_dependency(PATH "ktx" INCLUDE "include" LIB "ktx")
target_include_directories(engine PRIVATE ${ktx_INCLUDE_DIRS})
target_link_libraries(engine PRIVATE ${ktx_LIBRARIES})

# astc-encoder
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

target_link_libraries(engine PRIVATE astc)

# tinygltf
option(TINYGLTF_BUILD_LOADER_EXAMPLE OFF)

create_dependency(PATH "tinygltf" LIB "tinygltf")
target_link_libraries(engine PRIVATE ${tinygltf_LIBRARIES})
target_include_directories(engine PRIVATE ${tinygltf_INCLUDE_DIR})

message("ENG_VENDOR_SUBMODULE_LIST=" ${ENG_VENDOR_SUBMODULE_LIST})
message("ENG_VENDOR_NOT_FOUND_LIST=" ${ENG_VENDOR_NOT_FOUND_LIST})

set(SUBMODULE_CONFIG "{\"submodules\":[")
list(LENGTH ENG_VENDOR_SUBMODULE_LIST length)
if(${length} GREATER 0)
  list(GET ENG_VENDOR_SUBMODULE_LIST -1 LAST_ITEM)
endif()

foreach(SUBMODULE ${ENG_VENDOR_SUBMODULE_LIST})
  string(APPEND SUBMODULE_CONFIG "\"" ${SUBMODULE} "\"")
  list(FIND SUBMODULE_CONFIG SUBMODULE index)
  if(NOT "${LAST_ITEM}" STREQUAL "${SUBMODULE}")
    string(APPEND SUBMODULE_CONFIG ",\n")
  endif()
endforeach()

string(APPEND SUBMODULE_CONFIG "],\"not_found\":[")
list(LENGTH ENG_VENDOR_NOT_FOUND_LIST length)
if(${length} GREATER 0)
  list(GET ENG_VENDOR_NOT_FOUND_LIST -1 LAST_ITEM)
endif()

foreach(SUBMODULE ${ENG_VENDOR_NOT_FOUND_LIST})
  string(APPEND SUBMODULE_CONFIG "\"" ${SUBMODULE} "\"")
  list(FIND SUBMODULE_CONFIG SUBMODULE index)
  if(NOT "${LAST_ITEM}" STREQUAL "${SUBMODULE}")
    string(APPEND SUBMODULE_CONFIG ",\n")
  endif()
endforeach()

string(APPEND SUBMODULE_CONFIG "]}")

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/config.json ${SUBMODULE_CONFIG})
