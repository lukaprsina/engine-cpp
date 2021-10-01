set(SANDBOX_FILES
    # Header files
    src/sandbox.h
    # Source files
    src/sandbox.cpp)

    list(APPEND PROJECT_FILES ${SANDBOX_FILES})

add_library(sandbox STATIC ${SANDBOX_FILES})
target_link_libraries(sandbox PRIVATE engine)
target_link_libraries(engine_framework PRIVATE sandbox)
