cmake_minimum_required(VERSION 3.16)

project(UIModule)

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
set(UI_SOURCES
    # Core UI files
    src/UIModule.cpp
    src/ThemeManager.cpp
    src/StyleManager.cpp
    
    # Widgets
    widgets/BaseWidget.cpp
    widgets/CustomButton.cpp
    widgets/CustomDialog.cpp
    
    # Layouts
    layouts/FlexibleLayout.cpp
    layouts/ResponsiveLayout.cpp
    
    # Components
    components/NavigationBar.cpp
    components/StatusBar.cpp
    components/ToolBar.cpp
)

# Header files
set(UI_HEADERS
    # Core headers
    include/UIModule.h
    include/ThemeManager.h
    include/StyleManager.h
    
    # Widget headers
    widgets/BaseWidget.h
    widgets/CustomButton.h
    widgets/CustomDialog.h
    
    # Layout headers
    layouts/FlexibleLayout.h
    layouts/ResponsiveLayout.h
    
    # Component headers
    components/NavigationBar.h
    components/StatusBar.h
    components/ToolBar.h
    
    # Interface headers
    interfaces/IThemeManager.h
    interfaces/IStyleManager.h
)

# Create the UI library
add_library(UIModule STATIC
    ${UI_SOURCES}
    ${UI_HEADERS}
)

# Link Qt libraries
target_link_libraries(UIModule
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# Set target properties
set_target_properties(UIModule PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Export include directories
target_include_directories(UIModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)