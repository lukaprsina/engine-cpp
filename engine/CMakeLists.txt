set(PROJECT_FILES
    src/entrypoint.cpp
    
    src/platform/platform.cpp
    src/platform/platform.h
    src/platform/unix_platform.cpp
    src/platform/unix_platform.h
    src/platform/windows_platform.cpp
    src/platform/windows_platform.h

    src/core/application.cpp
    src/core/application.h
    src/core/window.cpp
    src/core/window.h
    )

add_executable(Engine WIN32 ${PROJECT_FILES})

target_include_directories(Engine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

target_precompile_headers(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h)

if(ANDROID)
    set(ENG_PLATFORM VK_USE_PLATFORM_ANDROID_KHR)
elseif(WIN32)
    set(ENG_PLATFORM VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    set(ENG_PLATFORM VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
    set(ENG_PLATFORM VK_USE_PLATFORM_XCB_KHR)
endif()

target_compile_definitions(Engine PRIVATE ENG_PLATFORM)

add_subdirectory("vendor")