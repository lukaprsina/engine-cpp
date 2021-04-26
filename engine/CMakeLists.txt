set(CORE_FILES
    # Header Files    
    src/core/application.h
    src/core/options.h
    src/core/log.h
    # Source Files    
    src/core/application.cpp
    src/core/options.cpp
    src/core/log.cpp)

set(COMMON_FILES
    # Header Files
    src/common/base_common.h
    src/common/log_common.h
    src/common/vulkan_common.h)

set(WINDOW_FILES
    # Header Files
    src/window/glfw_window.h
    src/window/window.h
    src/window/headless_window.h
    # Source Files
    src/window/window.cpp
    src/window/headless_window.cpp
    src/window/glfw_window.cpp)

set(PLATFORM_FILES
    # Header Files
    src/platform/platform.h
    # Source Files
    src/entrypoint.cpp
    src/platform/platform.cpp)

set(WINDOWS_FILES
    # Header Files
    src/platform/windows_platform.h
    # Source Files
    src/platform/windows_platform.cpp)

set(UNIX_FILES
    # Header Files
    src/platform/unix_platform.h
    # Source Files
    src/platform/unix_platform.cpp)

source_group("core\\" FILES ${CORE_FILES})
source_group("common\\" FILES ${COMMON_FILES})
source_group("window\\" FILES ${WINDOW_FILES})
source_group("platform\\" FILES ${PLATFORM_FILES})
source_group("platform\\windows" FILES ${WINDOWS_FILES})
source_group("platform\\unix" FILES ${UNIX_FILES})

set(PROJECT_FILES
    ${CORE_FILES}
    ${COMMON_FILES}
    ${WINDOW_FILES}
    ${PLATFORM_FILES}
    )

if(ANDROID)
    set(ENG_PLATFORM VK_USE_PLATFORM_ANDROID_KHR)
elseif(WIN32)
    list(APPEND PROJECT_FILES ${WINDOWS_FILES})
    set(ENG_PLATFORM VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    set(ENG_PLATFORM VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
    list(APPEND PROJECT_FILES ${UNIX_FILES})
    set(ENG_PLATFORM VK_USE_PLATFORM_XCB_KHR)
endif()

add_executable(Engine WIN32 ${PROJECT_FILES})

target_include_directories(Engine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

target_precompile_headers(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h)

target_compile_definitions(Engine PRIVATE ENG_PLATFORM)

add_subdirectory("vendor")