QT += core widgets network
CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = test_handlers_models
TEMPLATE = app

SOURCES += \
    test_handlers_models.cpp \
    handlers/URLHandler.cpp \
    handlers/ProtocolHandler.cpp \
    handlers/AuthHandler.cpp \
    models/Meeting.cpp \
    models/Room.cpp \
    models/Invitation.cpp

HEADERS += \
    handlers/URLHandler.h \
    handlers/ProtocolHandler.h \
    handlers/AuthHandler.h \
    models/Meeting.h \
    models/Room.h \
    models/Invitation.h

INCLUDEPATH += . handlers models