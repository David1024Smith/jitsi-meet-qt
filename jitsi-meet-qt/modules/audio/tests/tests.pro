# Audio Module Tests - QMake Project File

QT += core testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

# 目标名称
TARGET = AudioModuleTests

# 包含路径
INCLUDEPATH += \
    ../include \
    ../src \
    ../interfaces \
    ../config \
    ../utils \
    ../widgets

# 测试源文件
SOURCES += \
    AudioConfigTest.cpp \
    AudioUtilsTest.cpp \
    AudioModuleTest.cpp \
    AudioTestSuite.cpp \
    AudioTestRunner.cpp \
    ../config/AudioConfig.cpp \
    ../utils/AudioUtils.cpp

# 测试头文件
HEADERS += \
    AudioModuleTest.h \
    AudioTestSuite.h \
    ../config/AudioConfig.h \
    ../utils/AudioUtils.h \
    ../include/AudioModule.h \
    ../include/AudioManager.h \
    ../include/AudioFactory.h \
    ../interfaces/IAudioDevice.h \
    ../interfaces/IAudioManager.h \
    ../interfaces/IAudioProcessor.h

# 编译定义
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += AUDIO_MODULE_TEST
DEFINES += AUDIO_CONFIG_TEST
DEFINES += AUDIO_UTILS_TEST

# 输出目录
DESTDIR = $PWD/bin

# 临时文件目录
OBJECTS_DIR = $PWD/build/obj
MOC_DIR = $PWD/build/moc
RCC_DIR = $PWD/build/rcc
UI_DIR = $PWD/build/ui

# 清理规则
QMAKE_CLEAN += $DESTDIR/$TARGET

# 创建输出目录
!exists($DESTDIR) {
    system(mkdir -p $DESTDIR)
}

message("Audio module comprehensive tests project configured")
message("Target: $TARGET")
message("Output: $DESTDIR")
message("Tests included: AudioConfig, AudioUtils, AudioModule, TestSuite")