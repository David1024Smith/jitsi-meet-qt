cmake_minimum_required(VERSION 3.16)

project(PerformanceModule)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

# Source files
set(PERFORMANCE_SOURCES
    # Core performance files
    src/PerformanceModule.cpp
    src/PerformanceManager.cpp
    src/PerformanceMonitor.cpp
    
    # Optimizers
    optimizers/StartupOptimizer.cpp
    optimizers/MemoryOptimizer.cpp
    optimizers/NetworkOptimizer.cpp
    
    # Profilers
    profilers/CPUProfiler.cpp
    profilers/MemoryProfiler.cpp
    profilers/NetworkProfiler.cpp
    
    # Metrics
    metrics/MetricsCollector.cpp
    metrics/PerformanceMetrics.cpp
)

# Header files
set(PERFORMANCE_HEADERS
    # Core headers
    include/PerformanceModule.h
    include/PerformanceManager.h
    include/PerformanceMonitor.h
    
    # Optimizer headers
    optimizers/StartupOptimizer.h
    optimizers/MemoryOptimizer.h
    optimizers/NetworkOptimizer.h
    
    # Profiler headers
    profilers/CPUProfiler.h
    profilers/MemoryProfiler.h
    profilers/NetworkProfiler.h
    
    # Metrics headers
    metrics/MetricsCollector.h
    metrics/PerformanceMetrics.h
    
    # Interface headers
    interfaces/IPerformanceManager.h
    interfaces/IPerformanceMonitor.h
)

# Create the performance library
add_library(PerformanceModule STATIC
    ${PERFORMANCE_SOURCES}
    ${PERFORMANCE_HEADERS}
)

# Link Qt libraries
target_link_libraries(PerformanceModule
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# Set target properties
set_target_properties(PerformanceModule PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Export include directories
target_include_directories(PerformanceModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)