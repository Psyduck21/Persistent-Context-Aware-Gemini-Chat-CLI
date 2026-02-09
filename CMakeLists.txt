cmake_minimum_required(VERSION 3.10)

project(persistent_cli)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Put executable in project root directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Get curl flags directly
execute_process(
    COMMAND curl-config --cflags
    OUTPUT_VARIABLE CURL_CFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND curl-config --libs
    OUTPUT_VARIABLE CURL_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_executable(persistent_cli
    src/main.cpp
    src/Conversation.cpp
    src/GeminiClient.cpp
    src/CLIHandler.cpp
)

target_include_directories(persistent_cli PRIVATE
    include
)

# IMPORTANT: compile flags
target_compile_options(persistent_cli PRIVATE
    ${CURL_CFLAGS}
)

# IMPORTANT: link flags
target_link_libraries(persistent_cli PRIVATE
    ${CURL_LIBS}
)
