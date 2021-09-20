set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(ENG_VALIDATION_LAYERS
    ON
    CACHE BOOL "Enable validation layers")
set(ENG_VALIDATION_LAYERS_GPU_ASSISTED
    ON
    CACHE BOOL "Enable GPU assisted validation layers")
set(ENG_SHIPPING
    OFF
    CACHE BOOL "Install everything into the build directory")
set(ENG_WSI_SELECTION
    "XCB"
    CACHE STRING "Select WSI target (XCB, XLIB, WAYLAND, D2D)")
set(ENG_PLATFORM
    "VK_USE_PLATFORM_WIN32_KHR"
    CACHE STRING "Platform")

# set(ENG_PLATFORM "VK_USE_PLATFORM_WIN32_KHR" CACHE STRING "Platform test")
if(ANDROID)
  set(ENG_PLATFORM VK_USE_PLATFORM_ANDROID_KHR)
elseif(WIN32)
  set(ENG_PLATFORM VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
  set(ENG_PLATFORM VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
  set(ENG_PLATFORM VK_USE_PLATFORM_${ENG_WSI_SELECTION}_KHR)
endif()

if(${ENG_PLATFORM} EQUAL VK_USE_PLATFORM_WIN32_KHR)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
      "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${TARGET_ARCH}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
      "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}/${TARGET_ARCH}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
      "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}/${TARGET_ARCH}")
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

set(CMAKE_CXX_STANDARD 14)
