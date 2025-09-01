cmake_minimum_required(VERSION 3.16)

project(PerformanceModule)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Charts)

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
    src/MetricsCollector.cpp
    src/PerformanceMetrics.cpp
    src/OptimizationType.cpp
    
    # Optimizers
    optimizers/StartupOptimizer.cpp
    optimizers/MemoryOptimizer.cpp
    optimizers/RenderOptimizer.cpp
    optimizers/BaseOptimizer.cpp
    
    # Monitors
    monitors/CPUMonitor.cpp
    monitors/MemoryMonitor.cpp
    monitors/NetworkMonitor.cpp
    monitors/BaseMonitor.cpp
    
    # Config
    config/PerformanceConfig.cpp
    
    # Utils
    utils/PerformanceUtils.cpp
    
    # Widgets
    widgets/PerformanceWidget.cpp
    widgets/MetricsChart.cpp
)

# Header files
set(PERFORMANCE_HEADERS
    # Core headers
    include/PerformanceModule.h
    include/PerformanceManager.h
    include/MetricsCollector.h
    include/PerformanceMetrics.h
    include/IOptimizer.h
    include/OptimizationType.h
    
    # Optimizer headers
    optimizers/StartupOptimizer.h
    optimizers/MemoryOptimizer.h
    optimizers/RenderOptimizer.h
    optimizers/BaseOptimizer.h
    
    # Monitor headers
    monitors/CPUMonitor.h
    monitors/MemoryMonitor.h
    monitors/NetworkMonitor.h
    monitors/BaseMonitor.h
    
    # Config headers
    config/PerformanceConfig.h
    
    # Utils headers
    utils/PerformanceUtils.h
    
    # Widget headers
    widgets/PerformanceWidget.h
    widgets/MetricsChart.h
    
    # Interface headers
    interfaces/IOptimizer.h
    interfaces/IPerformanceMonitor.h
    interfaces/IResourceTracker.h
    interfaces/OptimizationType.h
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
    Qt6::Charts
)

# Link Windows libraries for performance monitoring
if(WIN32)
    target_link_libraries(PerformanceModule
        pdh
        psapi
        iphlpapi
    )
endif()

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