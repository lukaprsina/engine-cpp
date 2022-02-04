cmake_minimum_required(VERSION 3.19)

set(SUBMODULE_LIST "")
set(EXTERNAL_LIST "")
set(SUBMODULE_MISSING_LIST "")
set(EXTERNAL_MISSING_LIST "")

macro(create_dependency)
  set(options NO_CMAKE INTERFACE PUBLIC)
  set(oneValueArgs PATH INCLUDE LIB PACKAGE)
  set(multiValueArgs SOURCES)

  cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_PATH}/.git)
    if(${TARGET_NO_CMAKE})
      if(${TARGET_INTERFACE})
        add_library(${TARGET_PATH} INTERFACE ${TARGET_SOURCES})
      else()
        add_library(${TARGET_PATH} STATIC ${TARGET_SOURCES})
      endif()
      set(TARGET_LIB ${TARGET_PATH})
    else()
      add_subdirectory(${TARGET_PATH})
    endif()

    set(${TARGET_PATH}_FOUND TRUE)
    set(${TARGET_PATH}_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_PATH}/${TARGET_INCLUDE})
    set(${TARGET_PATH}_LIBRARIES ${TARGET_LIB})
    set(${TARGET_PATH}_SUBMODULE TRUE)
    set(${TARGET_PATH}_EXTERNAL FALSE)

    list(APPEND SUBMODULE_LIST ${TARGET_PATH})
  elseif(NOT "${TARGET_PACKAGE}" STREQUAL "")
    find_package(${TARGET_PACKAGE})

    set(${TARGET_PATH}_FOUND ${${TARGET_PACKAGE}_FOUND})
    set(${TARGET_PATH}_INCLUDE_DIRS ${${TARGET_PACKAGE}_INCLUDE_DIRS})
    set(${TARGET_PATH}_LIBRARIES ${${TARGET_PACKAGE}_LIBRARIES})

    if("${${TARGET_PACKAGE}_INCLUDE_DIRS}" STREQUAL ""
       AND NOT "${${TARGET_PACKAGE}_INCLUDE_DIR}" STREQUAL "")
      list(APPEND ${TARGET_PATH}_INCLUDE_DIRS ${${TARGET_PACKAGE}_INCLUDE_DIR})
    endif()

    if(${${TARGET_PACKAGE}_FOUND})
      list(APPEND EXTERNAL_LIST ${TARGET_PATH})
      set(${TARGET_PATH}_SUBMODULE FALSE)
      set(${TARGET_PATH}_EXTERNAL TRUE)
    else()
      list(APPEND EXTERNAL_MISSING_LIST ${TARGET_PATH})
      set(${TARGET_PATH}_SUBMODULE FALSE)
      set(${TARGET_PATH}_EXTERNAL FALSE)
    endif()

  else()
    list(APPEND SUBMODULE_MISSING_LIST ${TARGET_PATH})
  endif()

  # also build path
  message(
    ${TARGET_PATH}
    ": "
    ${${TARGET_PATH}_FOUND}
    ", "
    ${${TARGET_PATH}_INCLUDE_DIRS}
    ", "
    ${${TARGET_PATH}_LIBRARIES}
    "\n"
    ${TARGET_SOURCES})

    if(${${TARGET_PATH}_FOUND})
      if(${TARGET_PUBLIC})
        target_include_directories(engine PUBLIC ${${TARGET_PATH}_INCLUDE_DIRS})
        target_link_libraries(engine PUBLIC ${${TARGET_PATH}_LIBRARIES})
      else()
        target_include_directories(engine PRIVATE ${${TARGET_PATH}_INCLUDE_DIRS})
        target_link_libraries(engine PRIVATE ${${TARGET_PATH}_LIBRARIES})
      endif()
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
  "Vulkan"
  PUBLIC)
# Don't link and include vulkan directly, but through volk'

# volk
set(VOLK_PULL_IN_VULKAN OFF)
set(VOLK_STATIC_DEFINES ${ENG_PLATFORM})

create_dependency(PATH "volk" LIB "volk" PUBLIC)
if(${volk_FOUND})
  target_include_directories(${volk_LIBRARIES} PUBLIC ${vulkan_INCLUDE_DIRS})
  target_link_libraries(${volk_LIBRARIES} PUBLIC ${vulkan_LIBRARIES})
endif()

# stb
create_dependency(PATH "stb" NO_CMAKE INTERFACE)
if(${stb_FOUND})
  target_compile_definitions(${stb_LIBRARIES} INTERFACE STBI_NO_SIMD)
  target_include_directories(engine PRIVATE ${stb_INCLUDE_DIRS})
  target_link_libraries(engine PRIVATE ${stb_LIBRARIES})
endif()

# docopt
create_dependency(PATH "docopt" LIB "docopt" PUBLIC)

# glfw
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL ON)
set(GLFW_VULKAN_STATIC OFF)

create_dependency(
  PATH
  "glfw"
  INCLUDE
  "include"
  LIB
  "glfw"
  PACKAGE
  "GLFW")

# glm
create_dependency(PATH "glm" LIB "glm" PACKAGE "GLM" PUBLIC)
if(${glm_FOUND})
  target_compile_definitions(
    engine PUBLIC GLM_FORCE_SWIZZLE GLM_FORCE_RADIANS GLM_FORCE_CTOR_INIT
                   GLM_ENABLE_EXPERIMENTAL GLM_FORCE_DEPTH_ZERO_TO_ONE)
endif()

# spdlog TODO definitions set(SPDLOG_FMT_EXTERNAL "Use external fmt library
# instead of bundled" ON)

create_dependency(
  PATH
  "spdlog"
  INCLUDE
  "include"
  LIB
  "spdlog"
  #PACKAGE
  #"spdlog"
  PUBLIC)

# vma
create_dependency(PATH "vma" INCLUDE "include" PUBLIC)

# entt
create_dependency(PATH "entt" INCLUDE "src" LIB "EnTT::EnTT" PUBLIC)

# glslang
set(BUILD_EXTERNAL "Build external dependencies in /External" OFF)

create_dependency(PATH "glslang" LIB "glslang")
if(${glslang_FOUND})
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
endif()

# spirv-cross
create_dependency(PATH "spirv-cross")
if(${spirv-cross_FOUND})  
  target_link_libraries(
    engine
    PRIVATE spirv-cross-glsl
            spirv-cross-cpp
            spirv-cross-reflect
            spirv-cross-msl
            spirv-cross-hlsl
            spirv-cross-util
            spirv-cross-c)
endif()

# thread_pool
create_dependency(PATH "thread_pool" NO_CMAKE INTERFACE)
if(${thread_pool_FOUND})
  target_compile_definitions(
    engine PRIVATE _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)
endif()

# ktx
set(KTX_FEATURE_STATIC_LIBRARY ON)
set(KTX_FEATURE_TESTS OFF)

create_dependency(PATH "ktx" INCLUDE "include" LIB "ktx")

# astc-encoder
set(ASTC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/astc-encoder)
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

create_dependency(
  PATH
  "astc-encoder"
  INCLUDE
  "Source"
  SOURCES
  ${ASTC_SOURCES}
  NO_CMAKE)
if(${astc-encoder_FOUND})
  target_compile_definitions(${astc-encoder_LIBRARIES}
                             PRIVATE -DNO_STB_IMAGE_IMPLEMENTATION)
endif()

# tinygltf
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF)

create_dependency(PATH "tinygltf" LIB "tinygltf" PUBLIC)

# imgui
set(IMGUI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(IMGUI_BACKENDS_DIR ${IMGUI_SOURCE_DIR}/backends)
set(IMGUI_FONTS_DIR ${IMGUI_SOURCE_DIR}/misc/fonts)

set(IMGUI_SOURCES
    # Header files
    ${IMGUI_SOURCE_DIR}/imgui.h
    ${IMGUI_SOURCE_DIR}/imconfig.h
    ${IMGUI_SOURCE_DIR}/imgui_internal.h
    ${IMGUI_SOURCE_DIR}/imstb_rectpack.h
    ${IMGUI_SOURCE_DIR}/imstb_textedit.h
    ${IMGUI_SOURCE_DIR}/imstb_truetype.h
    ${IMGUI_BACKENDS_DIR}/imgui_impl_vulkan.h
    ${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.h
    # Source files
    ${IMGUI_SOURCE_DIR}/imgui.cpp
    ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
    ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
    ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
    ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_vulkan.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp)

if(ENG_PLATFORM EQUAL VK_USE_PLATFORM_WIN32_KHR)
  list(
    APPEND
    IMGUI_SOURCES
    # Header files
    ${IMGUI_BACKENDS_DIR}/imgui_impl_win32.h
    # Source files
    ${IMGUI_BACKENDS_DIR}/imgui_impl_win32.cpp)
endif()

create_dependency(
  PATH
  "imgui"
  SOURCES
  ${IMGUI_SOURCES}
  LIB
  "imgui"
  NO_CMAKE
  PUBLIC)
if(${imgui_FOUND})
  target_include_directories(
    ${imgui_LIBRARIES} PUBLIC ${imgui_INCLUDE_DIRS} ${vulkan_INCLUDE_DIRS}
                              ${glfw_INCLUDE_DIRS})
  target_link_libraries(${imgui_LIBRARIES} PUBLIC ${vulkan_LIBRARIES}
                                                  ${glfw_LIBRARIES})
endif()

message("SUBMODULE_LIST=" ${SUBMODULE_LIST})
message("EXTERNAL_LIST=" ${EXTERNAL_LIST})
message("SUBMODULE_MISSING_LIST=" ${SUBMODULE_MISSING_LIST})
message("EXTERNAL_MISSING_LIST=" ${EXTERNAL_MISSING_LIST})

set(DEPENDENCY_CONFIG "{\"submodules\":[")
list(LENGTH SUBMODULE_LIST length)
if(${length} GREATER 0)
  list(GET SUBMODULE_LIST -1 LAST_ITEM)
endif()

foreach(DEPENDENCY ${SUBMODULE_LIST})
  string(APPEND DEPENDENCY_CONFIG "\"" ${DEPENDENCY} "\"")
  list(FIND DEPENDENCY_CONFIG DEPENDENCY index)
  if(NOT "${LAST_ITEM}" STREQUAL "${DEPENDENCY}")
    string(APPEND DEPENDENCY_CONFIG ",\n")
  endif()
endforeach()

string(APPEND DEPENDENCY_CONFIG "],\"external\":[")
list(LENGTH EXTERNAL_LIST length)
if(${length} GREATER 0)
  list(GET EXTERNAL_LIST -1 LAST_ITEM)
endif()

foreach(DEPENDENCY ${EXTERNAL_LIST})
  string(APPEND DEPENDENCY_CONFIG "\"" ${DEPENDENCY} "\"")
  list(FIND DEPENDENCY_CONFIG DEPENDENCY index)
  if(NOT "${LAST_ITEM}" STREQUAL "${DEPENDENCY}")
    string(APPEND DEPENDENCY_CONFIG ",\n")
  endif()
endforeach()

string(APPEND DEPENDENCY_CONFIG "],\"submodule_missing\":[")
list(LENGTH SUBMODULE_MISSING_LIST length)
if(${length} GREATER 0)
  list(GET SUBMODULE_MISSING_LIST -1 LAST_ITEM)
endif()

foreach(DEPENDENCY ${SUBMODULE_MISSING_LIST})
  string(APPEND DEPENDENCY_CONFIG "\"" ${DEPENDENCY} "\"")
  list(FIND DEPENDENCY_CONFIG DEPENDENCY index)
  if(NOT "${LAST_ITEM}" STREQUAL "${DEPENDENCY}")
    string(APPEND DEPENDENCY_CONFIG ",\n")
  endif()
endforeach()

string(APPEND DEPENDENCY_CONFIG "],\"external_missing\":[")
list(LENGTH EXTERNAL_MISSING_LIST length)
if(${length} GREATER 0)
  list(GET EXTERNAL_MISSING_LIST -1 LAST_ITEM)
endif()

foreach(DEPENDENCY ${EXTERNAL_MISSING_LIST})
  string(APPEND DEPENDENCY_CONFIG "\"" ${DEPENDENCY} "\"")
  list(FIND DEPENDENCY_CONFIG DEPENDENCY index)
  if(NOT "${LAST_ITEM}" STREQUAL "${DEPENDENCY}")
    string(APPEND DEPENDENCY_CONFIG ",\n")
  endif()
endforeach()

string(APPEND DEPENDENCY_CONFIG "]}")

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/config.json ${DEPENDENCY_CONFIG})

list(LENGTH SUBMODULE_MISSING_LIST submodule_length)
list(LENGTH EXTERNAL_MISSING_LIST external_length)

if(NOT ${submodule_length} EQUAL 0 AND NOT ${external_length} EQUAL 0)
  message(FATAL_ERROR "Dependencies not found")
endif()

install(TARGETS ${ktx_LIBRARIES})
install(TARGETS ${docopt_LIBRARIES})
install(TARGETS ${imgui_LIBRARIES})
