QT += core network
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = test_crypto_basic
TEMPLATE = app

SOURCES += test_crypto_basic.cpp \
    string/StringUtils.cpp \
    string/Validator.cpp \
    crypto/HashUtils.cpp

HEADERS += \
    string/StringUtils.h \
    string/Validator.h \
    crypto/HashUtils.h

# Include paths
INCLUDEPATH += .