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
    src/common/base.h    
    src/common/error.h
    src/common/vulkan.h
    src/common/strings.h
    #Source files
    src/common/error.cpp
    src/common/strings.cpp
    )

set(EVENT_FILES
    # Header Files
    src/events/application_event.h    
    src/events/event.h
    src/events/key_event.h    
    src/events/mouse_event.h)

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

set(VULKAN_API_FILES
    #Header files
    src/vulkan_api/instance.h
    src/vulkan_api/physical_device.h
    src/vulkan_api/device.h
    src/vulkan_api/resource_cache.h
    src/vulkan_api/queue.h
    src/vulkan_api/command_pool.h
    src/vulkan_api/fence_pool.h
    src/vulkan_api/render_context.h
    src/vulkan_api/swapchain.h
    #Source files
    src/vulkan_api/instance.cpp
    src/vulkan_api/physical_device.cpp
    src/vulkan_api/device.cpp
    src/vulkan_api/resource_cache.cpp
    src/vulkan_api/queue.cpp
    src/vulkan_api/command_pool.cpp
    src/vulkan_api/fence_pool.cpp
    src/vulkan_api/render_context.cpp
    src/vulkan_api/swapchain.cpp)

set(PROJECT_FILES
    ${CORE_FILES}
    ${COMMON_FILES}
    ${WINDOW_FILES}
    ${PLATFORM_FILES}
    ${VULKAN_API_FILES}
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

target_compile_definitions(Engine
    PRIVATE ENG_PLATFORM
    PRIVATE ENG_VALIDATION_LAYERS
    PRIVATE ENG_VALIDATION_LAYERS_GPU_ASSISTED
    )

if(MSVC)
  target_compile_options(Engine PRIVATE /W3 /WX)
else()
  target_compile_options(Engine PRIVATE -Wall)
endif()

add_subdirectory("vendor")