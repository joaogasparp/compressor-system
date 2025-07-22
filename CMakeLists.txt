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
find_package(PkgConfig)

# Try to find FFTW3 for QFNC advanced fractal analysis
if(PKG_CONFIG_FOUND)
    pkg_check_modules(FFTW3 fftw3)
endif()

# Include directories
include_directories(src)

# Source files
file(GLOB_RECURSE SOURCES
    "src/core/*.cpp"
    "src/algorithms/*.cpp"
    "src/benchmark/*.cpp"
    "src/cli/*.cpp"
    "src/utils/*.cpp"
)

file(GLOB_RECURSE HEADERS
    "src/core/*.hpp"
    "src/algorithms/*.hpp"
    "src/benchmark/*.hpp"
    "src/cli/*.hpp"
    "src/utils/*.hpp"
)

# Create executable
add_executable(compressor src/main.cpp ${SOURCES})

# Create web server executable
add_executable(web_server src/web_server.cpp ${SOURCES})

# Link libraries
target_link_libraries(compressor Threads::Threads m)
target_link_libraries(web_server Threads::Threads m)

# Optional: Link zlib if available
find_package(ZLIB)
if(ZLIB_FOUND)
    target_link_libraries(compressor ${ZLIB_LIBRARIES})
    target_compile_definitions(compressor PRIVATE HAVE_ZLIB)
endif()

# Enable testing
enable_testing()

# Test executable
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")
if(TEST_SOURCES)
    add_executable(test_compressor ${TEST_SOURCES} ${SOURCES})
    target_link_libraries(test_compressor Threads::Threads)
    if(ZLIB_FOUND)
        target_link_libraries(test_compressor ${ZLIB_LIBRARIES})
        target_compile_definitions(test_compressor PRIVATE HAVE_ZLIB)
    endif()
    add_test(NAME CompressorTests COMMAND test_compressor)
endif()

# Install rules
install(TARGETS compressor DESTINATION bin)
