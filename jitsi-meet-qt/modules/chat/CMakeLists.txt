cmake_minimum_required(VERSION 3.16)

project(ChatModule)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

# Source files
set(CHAT_SOURCES
    # Core chat files
    src/ChatManager.cpp
    src/MessageHandler.cpp
    src/ChatMessage.cpp
    src/ChatRoom.cpp
    src/Participant.cpp
    
    # Storage
    storage/MessageStorage.cpp
    storage/ChatStorage.cpp
    
    # UI components
    ui/ChatWidget.cpp
    ui/MessageWidget.cpp
    ui/ParticipantWidget.cpp
    
    # Handlers
    handlers/FileHandler.cpp
    handlers/EmojiHandler.cpp
    handlers/NotificationHandler.cpp
)

# Header files
set(CHAT_HEADERS
    # Core headers
    include/ChatManager.h
    include/MessageHandler.h
    include/ChatMessage.h
    include/ChatRoom.h
    include/Participant.h
    
    # Storage headers
    storage/MessageStorage.h
    storage/ChatStorage.h
    
    # UI headers
    ui/ChatWidget.h
    ui/MessageWidget.h
    ui/ParticipantWidget.h
    
    # Handler headers
    handlers/FileHandler.h
    handlers/EmojiHandler.h
    handlers/NotificationHandler.h
    
    # Interface headers
    interfaces/IChatManager.h
    interfaces/IMessageHandler.h
)

# Create the chat library
add_library(ChatModule STATIC
    ${CHAT_SOURCES}
    ${CHAT_HEADERS}
)

# Link Qt libraries
target_link_libraries(ChatModule
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
)

# Set target properties
set_target_properties(ChatModule PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Export include directories
target_include_directories(ChatModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)