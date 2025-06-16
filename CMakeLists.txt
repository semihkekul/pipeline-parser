cmake_minimum_required(VERSION 3.15)

# Set the project name
project(PipelineParser)

set(CMAKE_CXX_STANDARD 17)

# Main executable
add_executable(PipelineParser "logparser.cpp" "main.cpp")

set(LOGS_DIR "${CMAKE_SOURCE_DIR}/logs")
target_compile_definitions(PipelineParser PRIVATE LOGS_DIR="${LOGS_DIR}")

# Google Test
enable_testing()
add_subdirectory(googletest)
include_directories(googletest/googletest/include)


# Test executable
add_executable(PipelineParserTests "logparser.cpp" "test/testlogparser.cpp")
target_link_libraries(PipelineParserTests gtest gtest_main)

target_compile_definitions(PipelineParserTests PRIVATE LOGS_DIR="${LOGS_DIR}")


