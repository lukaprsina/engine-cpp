set(SANDBOX_FILES
    # Header files
    src/sandbox.h
    # Source files
    src/sandbox.cpp)

add_library(sandbox STATIC ${SANDBOX_FILES})
target_include_directories(sandbox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
