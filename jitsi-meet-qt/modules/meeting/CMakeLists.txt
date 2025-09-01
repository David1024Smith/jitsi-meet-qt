cmake_minimum_required(VERSION 3.16)

project(MeetingModule)

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
set(MEETING_SOURCES
    # Core meeting files
    src/MeetingModule.cpp
    src/MeetingManager.cpp
    src/ConferenceRoom.cpp
    
    # Session management
    session/SessionManager.cpp
    session/ParticipantManager.cpp
    
    # Controls
    controls/MeetingControls.cpp
    controls/ParticipantControls.cpp
    
    # Recording
    recording/RecordingManager.cpp
    recording/StreamRecorder.cpp
)

# Header files
set(MEETING_HEADERS
    # Core headers
    include/MeetingModule.h
    include/MeetingManager.h
    include/ConferenceRoom.h
    
    # Session headers
    session/SessionManager.h
    session/ParticipantManager.h
    
    # Control headers
    controls/MeetingControls.h
    controls/ParticipantControls.h
    
    # Recording headers
    recording/RecordingManager.h
    recording/StreamRecorder.h
    
    # Interface headers
    interfaces/IMeetingManager.h
    interfaces/IConferenceRoom.h
)

# Create the meeting library
add_library(MeetingModule STATIC
    ${MEETING_SOURCES}
    ${MEETING_HEADERS}
)

# Link Qt libraries
target_link_libraries(MeetingModule
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
)

# Set target properties
set_target_properties(MeetingModule PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Export include directories
target_include_directories(MeetingModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)