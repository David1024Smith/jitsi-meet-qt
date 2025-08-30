# Meeting Module Configuration
# Jitsi Meet Qt - Meeting Link Management Module
# Version: 1.0.0

MEETING_MODULE_VERSION = 1.0.0
DEFINES += MEETING_MODULE_VERSION=\\\"$MEETING_MODULE_VERSION\\\"
DEFINES += MEETING_MODULE_ENABLED

message("Loading Meeting Module v$MEETING_MODULE_VERSION...")

# Module paths
MEETING_MODULE_PATH = $$PWD
INCLUDEPATH += $$MEETING_MODULE_PATH/include
INCLUDEPATH += $$MEETING_MODULE_PATH/interfaces
INCLUDEPATH += $$MEETING_MODULE_PATH/config
INCLUDEPATH += $$MEETING_MODULE_PATH/handlers
INCLUDEPATH += $$MEETING_MODULE_PATH/models
INCLUDEPATH += $$MEETING_MODULE_PATH/widgets

# Core headers
HEADERS += \
    $$MEETING_MODULE_PATH/include/MeetingModule.h \
    $$MEETING_MODULE_PATH/include/MeetingManager.h \
    $$MEETING_MODULE_PATH/include/LinkHandler.h

# Interface headers
HEADERS += \
    $$MEETING_MODULE_PATH/interfaces/IMeetingManager.h \
    $$MEETING_MODULE_PATH/interfaces/ILinkHandler.h \
    $$MEETING_MODULE_PATH/interfaces/IRoomManager.h

# Configuration headers
HEADERS += \
    $$MEETING_MODULE_PATH/config/MeetingConfig.h

# Handler headers
HEADERS += \
    $$MEETING_MODULE_PATH/handlers/URLHandler.h \
    $$MEETING_MODULE_PATH/handlers/ProtocolHandler.h \
    $$MEETING_MODULE_PATH/handlers/AuthHandler.h

# Model headers
HEADERS += \
    $$MEETING_MODULE_PATH/models/Meeting.h \
    $$MEETING_MODULE_PATH/models/Room.h \
    $$MEETING_MODULE_PATH/models/Invitation.h

# Widget headers
HEADERS += \
    $$MEETING_MODULE_PATH/widgets/MeetingWidget.h \
    $$MEETING_MODULE_PATH/widgets/JoinDialog.h \
    $$MEETING_MODULE_PATH/widgets/CreateDialog.h

# Core sources
SOURCES += \
    $$MEETING_MODULE_PATH/src/MeetingModule.cpp \
    $$MEETING_MODULE_PATH/src/MeetingManager.cpp \
    $$MEETING_MODULE_PATH/src/LinkHandler.cpp

# Configuration sources
SOURCES += \
    $$MEETING_MODULE_PATH/config/MeetingConfig.cpp

# Handler sources
SOURCES += \
    $$MEETING_MODULE_PATH/handlers/URLHandler.cpp \
    $$MEETING_MODULE_PATH/handlers/ProtocolHandler.cpp \
    $$MEETING_MODULE_PATH/handlers/AuthHandler.cpp

# Model sources
SOURCES += \
    $$MEETING_MODULE_PATH/models/Meeting.cpp \
    $$MEETING_MODULE_PATH/models/Room.cpp \
    $$MEETING_MODULE_PATH/models/Invitation.cpp

# Widget sources
SOURCES += \
    $$MEETING_MODULE_PATH/widgets/MeetingWidget.cpp \
    $$MEETING_MODULE_PATH/widgets/JoinDialog.cpp \
    $$MEETING_MODULE_PATH/widgets/CreateDialog.cpp

# Resources
RESOURCES += $$MEETING_MODULE_PATH/resources/meeting_resources.qrc

# Dependencies
QT += core widgets network

# Compiler flags
QMAKE_CXXFLAGS += -std=c++17

# Module-specific defines
DEFINES += MEETING_MODULE_AVAILABLE

message("✓ Meeting Module configuration loaded")
message("  - Meeting link processing and validation")
message("  - Room management and authentication")
message("  - Meeting creation and joining workflows")
message("  - URL protocol handling")