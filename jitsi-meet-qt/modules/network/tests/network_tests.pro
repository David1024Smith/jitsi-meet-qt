QT += core network testlib websockets
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = NetworkModuleTests
TEMPLATE = app

# 定义测试宏
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += NETWORK_MODULE_TESTS

# 包含路径
INCLUDEPATH += \
    . \
    .. \
    ../interfaces \
    ../include \
    ../src \
    ../protocols \
    ../utils \
    ../config \
    ../widgets

# 测试源文件
SOURCES += \
    main.cpp \
    NetworkModuleTest.cpp \
    NetworkTestRunner.cpp \
    NetworkTestSuite.cpp \
    MockObjects.cpp

# 测试头文件
HEADERS += \
    NetworkModuleTest.h

# 网络模块源文件
SOURCES += \
    ../src/NetworkManager.cpp \
    ../src/NetworkManagerImpl.cpp \
    ../src/ConnectionFactory.cpp \
    ../src/BaseConnectionHandler.cpp \
    ../src/NetworkModule.cpp \
    ../protocols/WebRTCProtocol.cpp \
    ../protocols/HTTPProtocol.cpp \
    ../protocols/WebSocketProtocol.cpp \
    ../utils/NetworkQualityMonitor.cpp \
    ../utils/NetworkDiagnostics.cpp \
    ../utils/NetworkUtils.cpp \
    ../config/NetworkConfig.cpp \
    ../widgets/ConnectionWidget.cpp \
    ../widgets/NetworkStatusWidget.cpp

# 网络模块头文件
HEADERS += \
    ../interfaces/INetworkManager.h \
    ../interfaces/IConnectionHandler.h \
    ../interfaces/IProtocolHandler.h \
    ../include/NetworkManager.h \
    ../include/ConnectionFactory.h \
    ../protocols/WebRTCProtocol.h \
    ../protocols/HTTPProtocol.h \
    ../protocols/WebSocketProtocol.h \
    ../utils/NetworkQualityMonitor.h \
    ../utils/NetworkDiagnostics.h \
    ../utils/NetworkUtils.h \
    ../config/NetworkConfig.h \
    ../widgets/ConnectionWidget.h \
    ../widgets/NetworkStatusWidget.h

# 编译器标志
win32 {
    QMAKE_CXXFLAGS += /W4
} else {
    QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic
}

# 输出目录
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui

# 测试数据文件
DISTFILES += \
    data/test_config.json.in \
    run_tests.bat \
    run_tests.sh \
    README.md

# 创建输出目录
!exists($$DESTDIR) {
    system($$QMAKE_MKDIR $$DESTDIR)
}

# 复制测试数据
copydata.commands = $(COPY_DIR) $$PWD/data $$DESTDIR
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

# 测试目标
test.commands = $$DESTDIR/$$TARGET
test.depends = $$DESTDIR/$$TARGET
QMAKE_EXTRA_TARGETS += test

# 清理目标
QMAKE_CLEAN += $$DESTDIR/$$TARGET