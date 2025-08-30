# Audio Module - Qt Project Include File
# 音频模块 - Qt项目包含文件

######################################################################
# Module Information
######################################################################

# 模块基本信息
AUDIO_MODULE_NAME = AUDIO
AUDIO_MODULE_VERSION = 1.0.0
AUDIO_MODULE_DESCRIPTION = "Audio device management and processing module"

# 模块标识符
DEFINES += AUDIO_MODULE_AVAILABLE
DEFINES += AUDIO_MODULE_VERSION=\\\"$AUDIO_MODULE_VERSION\\\"

# 模块配置
CONFIG += audio_enabled
CONFIG += audio_webrtc_integration

######################################################################
# Module Dependencies
######################################################################

# 检查依赖模块
!utils_module_loaded {
    warning("Audio module recommends Utils module for logging")
}

######################################################################
# Include Paths
######################################################################

# 模块包含路径
INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/src
INCLUDEPATH += $$PWD/interfaces

######################################################################
# Headers
######################################################################

# 核心头文件
HEADERS += \
    $$PWD/include/AudioModule.h \
    $$PWD/include/AudioManager.h \
    $$PWD/include/AudioFactory.h

# 接口头文件
HEADERS += \
    $$PWD/interfaces/IAudioDevice.h \
    $$PWD/interfaces/IAudioManager.h \
    $$PWD/interfaces/IAudioProcessor.h

# 配置头文件
HEADERS += \
    $$PWD/config/AudioConfig.h

# 工具头文件
HEADERS += \
    $$PWD/utils/AudioUtils.h

# UI组件头文件
HEADERS += \
    $$PWD/widgets/AudioControlWidget.h \
    $$PWD/widgets/VolumeSliderWidget.h

######################################################################
# Sources
######################################################################

# 核心源文件
SOURCES += \
    $$PWD/src/AudioModule.cpp \
    $$PWD/src/AudioManager.cpp \
    $$PWD/src/AudioFactory.cpp

# 配置源文件
SOURCES += \
    $$PWD/config/AudioConfig.cpp

# 工具源文件
SOURCES += \
    $$PWD/utils/AudioUtils.cpp

# UI组件源文件
SOURCES += \
    $$PWD/widgets/AudioControlWidget.cpp \
    $$PWD/widgets/VolumeSliderWidget.cpp

######################################################################
# Resources
######################################################################

RESOURCES += $$PWD/resources/audio.qrc

######################################################################
# Platform Specific Configuration
######################################################################

# Windows 特定配置
win32 {
    LIBS += -lwinmm -lole32 -loleaut32
    DEFINES += AUDIO_MODULE_WINDOWS
    HEADERS += $$PWD/src/platform/AudioDevice_Windows.h
    SOURCES += $$PWD/src/platform/AudioDevice_Windows.cpp
}

# Linux 特定配置
unix:!macx {
    LIBS += -lpulse -lasound
    DEFINES += AUDIO_MODULE_LINUX
    HEADERS += $$PWD/src/platform/AudioDevice_Linux.h
    SOURCES += $$PWD/src/platform/AudioDevice_Linux.cpp
}

# macOS 特定配置
macx {
    LIBS += -framework CoreAudio -framework AudioUnit
    DEFINES += AUDIO_MODULE_MACOS
    HEADERS += $$PWD/src/platform/AudioDevice_macOS.h
    SOURCES += $$PWD/src/platform/AudioDevice_macOS.cpp
}

######################################################################
# Audio Specific Configuration
######################################################################

# 音频质量配置
DEFINES += AUDIO_DEFAULT_SAMPLE_RATE=48000
DEFINES += AUDIO_DEFAULT_CHANNELS=2
DEFINES += AUDIO_DEFAULT_BUFFER_SIZE=1024

# WebRTC 集成
DEFINES += AUDIO_WEBRTC_ENABLED

######################################################################
# Debug Information
######################################################################

message("Audio module v$AUDIO_MODULE_VERSION loaded")
message("  - Description: $AUDIO_MODULE_DESCRIPTION")
message("  - WebRTC Integration: Enabled")
message("  - Platform Support: Cross-platform")

# 导出模块信息
export(AUDIO_MODULE_NAME)
export(AUDIO_MODULE_VERSION)
# 平台特定
音频设备实现
HEADERS += \
    $PWD/src/platform/QtAudioDevice.h

SOURCES += \
    $PWD/src/platform/QtAudioDevice.cpp

# Qt音频模块依赖
QT += multimedia