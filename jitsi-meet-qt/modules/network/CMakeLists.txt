cmake_minimum_required(VERSION 3.16)

project(NetworkModule)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network WebSockets)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

# Source files
set(NETWORK_SOURCES
    # Core network files
    src/NetworkManager.cpp
    src/NetworkManagerImpl.cpp
    src/ConnectionManager.cpp
    src/NetworkMonitor.cpp
    
    # Connection handlers
    handlers/BaseConnectionHandler.cpp
    handlers/WebSocketHandler.cpp
    handlers/HttpHandler.cpp
    
    # Protocols
    protocols/WebRTCProtocol.cpp
    protocols/XMPPProtocol.cpp
    
    # Security
    security/TLSManager.cpp
    security/CertificateValidator.cpp
)

# Header files
set(NETWORK_HEADERS
    # Core headers
    include/NetworkManager.h
    include/NetworkManagerImpl.h
    include/ConnectionManager.h
    include/NetworkMonitor.h
    
    # Handler headers
    handlers/BaseConnectionHandler.h
    handlers/WebSocketHandler.h
    handlers/HttpHandler.h
    
    # Protocol headers
    protocols/WebRTCProtocol.h
    protocols/XMPPProtocol.h
    
    # Security headers
    security/TLSManager.h
    security/CertificateValidator.h
    
    # Interface headers
    interfaces/INetworkManager.h
    interfaces/IConnectionHandler.h
)

# Create the network library
add_library(NetworkModule STATIC
    ${NETWORK_SOURCES}
    ${NETWORK_HEADERS}
)

# Link Qt libraries
target_link_libraries(NetworkModule
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
)

# Set target properties
set_target_properties(NetworkModule PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Export include directories
target_include_directories(NetworkModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)