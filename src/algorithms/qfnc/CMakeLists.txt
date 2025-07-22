# QFNC Algorithm CMake Configuration
# Revolutionary Quantum Fractal Neural Compressor

# Find required packages for advanced features
find_package(PkgConfig)

# Try to find FFTW3 for fractal analysis
if(PKG_CONFIG_FOUND)
    pkg_check_modules(FFTW3 fftw3)
endif()

# QFNC Algorithm source files
set(QFNC_SOURCES
    qfnc_algorithm.cpp
    qfnc_algorithm.hpp
)

# Create QFNC library
add_library(qfnc_algorithm STATIC ${QFNC_SOURCES})

# Set compiler flags for advanced optimization
target_compile_features(qfnc_algorithm PRIVATE cxx_std_17)
target_compile_options(qfnc_algorithm PRIVATE
    -O3
    -march=native
    -mtune=native
    -ffast-math
    -funroll-loops
    -flto
)

# Link math library for complex mathematical operations
target_link_libraries(qfnc_algorithm PRIVATE m)

# Link FFTW3 if available for fractal analysis
if(FFTW3_FOUND)
    target_include_directories(qfnc_algorithm PRIVATE ${FFTW3_INCLUDE_DIRS})
    target_link_libraries(qfnc_algorithm PRIVATE ${FFTW3_LIBRARIES})
    target_compile_definitions(qfnc_algorithm PRIVATE HAVE_FFTW3)
else()
    message(STATUS "FFTW3 not found - using fallback fractal analysis")
endif()

# Enable threading for parallel quantum simulation
find_package(Threads REQUIRED)
target_link_libraries(qfnc_algorithm PRIVATE Threads::Threads)

# Include directories
target_include_directories(qfnc_algorithm PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Export target for main project
set_target_properties(qfnc_algorithm PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
)

# Add to parent scope
set(QFNC_ALGORITHM_TARGET qfnc_algorithm PARENT_SCOPE)
