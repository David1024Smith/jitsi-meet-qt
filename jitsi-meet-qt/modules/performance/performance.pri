# Performance Module Configuration
# Jitsi Meet Qt - Performance Monitoring and Optimization Module
# Version: 1.0.0

PERFORMANCE_MODULE_VERSION = 1.0.0
DEFINES += PERFORMANCE_MODULE_VERSION=\\\"$$PERFORMANCE_MODULE_VERSION\\\"
DEFINES += PERFORMANCE_MODULE_AVAILABLE

message("Loading Performance Module v$$PERFORMANCE_MODULE_VERSION...")

# Module paths
PERFORMANCE_MODULE_PATH = $$PWD
PERFORMANCE_INCLUDE_PATH = $$PERFORMANCE_MODULE_PATH/include
PERFORMANCE_SRC_PATH = $$PERFORMANCE_MODULE_PATH/src
PERFORMANCE_INTERFACES_PATH = $$PERFORMANCE_MODULE_PATH/interfaces
PERFORMANCE_CONFIG_PATH = $$PERFORMANCE_MODULE_PATH/config
PERFORMANCE_MONITORS_PATH = $$PERFORMANCE_MODULE_PATH/monitors
PERFORMANCE_OPTIMIZERS_PATH = $$PERFORMANCE_MODULE_PATH/optimizers
PERFORMANCE_WIDGETS_PATH = $$PERFORMANCE_MODULE_PATH/widgets
PERFORMANCE_UTILS_PATH = $$PERFORMANCE_MODULE_PATH/utils
PERFORMANCE_TESTS_PATH = $$PERFORMANCE_MODULE_PATH/tests
PERFORMANCE_EXAMPLES_PATH = $$PERFORMANCE_MODULE_PATH/examples
PERFORMANCE_RESOURCES_PATH = $$PERFORMANCE_MODULE_PATH/resources

# Include paths
INCLUDEPATH += $$PERFORMANCE_INCLUDE_PATH
INCLUDEPATH += $$PERFORMANCE_INTERFACES_PATH
INCLUDEPATH += $$PERFORMANCE_CONFIG_PATH
INCLUDEPATH += $$PERFORMANCE_MONITORS_PATH
INCLUDEPATH += $$PERFORMANCE_OPTIMIZERS_PATH
INCLUDEPATH += $$PERFORMANCE_WIDGETS_PATH
INCLUDEPATH += $$PERFORMANCE_UTILS_PATH

# Headers
HEADERS += \
    # Core headers
    $$PERFORMANCE_INCLUDE_PATH/PerformanceModule.h \
    $$PERFORMANCE_INCLUDE_PATH/PerformanceManager.h \
    $$PERFORMANCE_INCLUDE_PATH/MetricsCollector.h \
    $$PERFORMANCE_INCLUDE_PATH/OptimizationType.h \
    \
    # Interface headers
    $$PERFORMANCE_INTERFACES_PATH/IPerformanceMonitor.h \
    $$PERFORMANCE_INTERFACES_PATH/IResourceTracker.h \
    $$PERFORMANCE_INTERFACES_PATH/IOptimizer.h \
    \
    # Configuration headers
    $$PERFORMANCE_CONFIG_PATH/PerformanceConfig.h \
    \
    # Monitor headers
    $$PERFORMANCE_MONITORS_PATH/CPUMonitor.h \
    $$PERFORMANCE_MONITORS_PATH/MemoryMonitor.h \
    $$PERFORMANCE_MONITORS_PATH/NetworkMonitor.h \
    $$PERFORMANCE_MONITORS_PATH/BaseMonitor.h \
    \
    # Optimizer headers
    $$PERFORMANCE_OPTIMIZERS_PATH/StartupOptimizer.h \
    $$PERFORMANCE_OPTIMIZERS_PATH/MemoryOptimizer.h \
    $$PERFORMANCE_OPTIMIZERS_PATH/RenderOptimizer.h \
    $$PERFORMANCE_OPTIMIZERS_PATH/BaseOptimizer.h \
    \
    # Widget headers
    $$PERFORMANCE_WIDGETS_PATH/PerformanceWidget.h \
    $$PERFORMANCE_WIDGETS_PATH/MetricsChart.h \
    \
    # Utility headers
    $$PERFORMANCE_UTILS_PATH/PerformanceUtils.h

# Sources
SOURCES += \
    # Core sources
    $$PERFORMANCE_SRC_PATH/PerformanceModule.cpp \
    $$PERFORMANCE_SRC_PATH/PerformanceManager.cpp \
    $$PERFORMANCE_SRC_PATH/MetricsCollector.cpp \
    $$PERFORMANCE_SRC_PATH/OptimizationType.cpp \
    \
    # Configuration sources
    $$PERFORMANCE_CONFIG_PATH/PerformanceConfig.cpp \
    \
    # Monitor sources
    $$PERFORMANCE_MONITORS_PATH/CPUMonitor.cpp \
    $$PERFORMANCE_MONITORS_PATH/MemoryMonitor.cpp \
    $$PERFORMANCE_MONITORS_PATH/NetworkMonitor.cpp \
    $$PERFORMANCE_MONITORS_PATH/BaseMonitor.cpp \
    \
    # Optimizer sources
    $$PERFORMANCE_OPTIMIZERS_PATH/StartupOptimizer.cpp \
    $$PERFORMANCE_OPTIMIZERS_PATH/MemoryOptimizer.cpp \
    $$PERFORMANCE_OPTIMIZERS_PATH/RenderOptimizer.cpp \
    $$PERFORMANCE_OPTIMIZERS_PATH/BaseOptimizer.cpp \
    \
    # Widget sources
    $$PERFORMANCE_WIDGETS_PATH/PerformanceWidget.cpp \
    $$PERFORMANCE_WIDGETS_PATH/MetricsChart.cpp \
    \
    # Utility sources
    $$PERFORMANCE_UTILS_PATH/PerformanceUtils.cpp

# Resources
RESOURCES += $$PERFORMANCE_RESOURCES_PATH/performance_resources.qrc

# Dependencies
QT += core widgets charts network

# Platform-specific configurations
win32 {
    DEFINES += PERFORMANCE_WINDOWS
    LIBS += -lpsapi -lpdh
}

unix:!macx {
    DEFINES += PERFORMANCE_LINUX
    LIBS += -lprocps
}

macx {
    DEFINES += PERFORMANCE_MACOS
    LIBS += -framework IOKit -framework CoreFoundation
}

# Compiler flags for performance optimization
QMAKE_CXXFLAGS += -O2
DEFINES += QT_NO_DEBUG_OUTPUT

# Export symbols for module
DEFINES += PERFORMANCE_MODULE_EXPORT

message("✓ Performance Module configuration loaded")
message("  - Version: $$PERFORMANCE_MODULE_VERSION")
message("  - Platform optimizations enabled")
message("  - Performance monitoring ready")