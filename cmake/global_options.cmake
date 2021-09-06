set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(ENG_VALIDATION_LAYERS
    ON
    CACHE BOOL "Enable validation layers")
set(ENG_VALIDATION_LAYERS_GPU_ASSISTED
    ON
    CACHE BOOL "Enable GPU assisted validation layers")
set(VKB_WSI_SELECTION
    "XCB"
    CACHE STRING "Select WSI target (XCB, XLIB, WAYLAND, D2D)")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/${CMAKE_BUILD_TYPE}/${TARGET_ARCH}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib/${CMAKE_BUILD_TYPE}/${TARGET_ARCH}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "lib/${CMAKE_BUILD_TYPE}/${TARGET_ARCH}")

set(CMAKE_CXX_STANDARD 14)