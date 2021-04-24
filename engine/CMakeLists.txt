set(PROJECT_FILES
    src/entrypoint.cpp
    src/platform/platform.cpp)

add_executable(Engine ${PROJECT_FILES})

target_include_directories(Engine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

target_precompile_headers(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h)

add_subdirectory("vendor")