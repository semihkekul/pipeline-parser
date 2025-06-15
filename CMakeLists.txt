cmake_minimum_required(VERSION 3.10)

# Set the project name
project(PipelineParser)

# Add the executable
add_executable(PipelineParser logparser.cpp "main.cpp")


set(LOGS_DIR "${CMAKE_SOURCE_DIR}/logs")
target_compile_definitions(PipelineParser PRIVATE LOGS_DIR="${LOGS_DIR}")


