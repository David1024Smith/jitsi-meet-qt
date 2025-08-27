QT += core widgets testlib

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = test_recentlist

# 包含路径
INCLUDEPATH += ../include

# 测试源文件
SOURCES += test_recentlist.cpp

# 被测试的源文件
SOURCES += \
    ../src/RecentListWidget.cpp \
    ../src/models/RecentItem.cpp

# 被测试的头文件
HEADERS += \
    ../include/RecentListWidget.h \
    ../include/models/RecentItem.h