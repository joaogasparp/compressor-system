cmake_minimum_required(VERSION 3.16)
project(CompressorSystem VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wall")

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(src)

# Source files
file(GLOB_RECURSE SOURCES
    "src/core/*.cpp"
    "src/algorithms/*.cpp"
    "src/utils/*.cpp"
)

file(GLOB_RECURSE HEADERS
    "src/core/*.hpp"
    "src/algorithms/*.hpp"
    "src/utils/*.hpp"
)

# Create web server executable
add_executable(web_server src/web_server.cpp ${SOURCES})

# Link libraries
target_link_libraries(web_server Threads::Threads m)

# Optional: Link zlib if available
find_package(ZLIB)
if(ZLIB_FOUND)
    target_link_libraries(web_server ${ZLIB_LIBRARIES})
    target_compile_definitions(web_server PRIVATE HAVE_ZLIB)
endif()

# Install rules
install(TARGETS web_server DESTINATION bin)
