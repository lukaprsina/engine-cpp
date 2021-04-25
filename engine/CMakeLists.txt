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

add_executable(Engine ${PROJECT_FILES})

target_include_directories(Engine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

target_precompile_headers(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h)

add_subdirectory("vendor")