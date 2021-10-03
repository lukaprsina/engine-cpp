set(SANDBOX_SRC ${CMAKE_SOURCE_DIR}/sandbox/src  CACHE STRING "Sandbox source directory")

set(SANDBOX_FILES
    # Header files
    ${SANDBOX_SRC}/sandbox.h
    # Source files
    ${SANDBOX_SRC}/sandbox.cpp)

set(ENG_PROJECT_FILES ${SANDBOX_FILES} CACHE STRING "All built files")