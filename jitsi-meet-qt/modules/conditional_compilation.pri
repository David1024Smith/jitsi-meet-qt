# Conditional Compilation Configuration
# 条件编译配置

######################################################################
# Conditional Compilation Features
######################################################################

# 特性标志定义
FEATURE_FLAGS = $$split(JITSI_FEATURES, ",")

# 音频特性
contains(FEATURE_FLAGS, advanced_audio) | CONFIG(advanced_audio) {
    DEFINES += ADVANCED_AUDIO_FEATURES
    message("Advanced audio features enabled")
}

contains(FEATURE_FLAGS, noise_suppression) | CONFIG(noise_suppression) {
    DEFINES += NOISE_SUPPRESSION_ENABLED
    message("Noise suppression enabled")
}

# 视频特性
contains(FEATURE_FLAGS, hd_video) | CONFIG(hd_video) {
    DEFINES += HD_VIDEO_SUPPORT
    message("HD video support enabled")
}

contains(FEATURE_FLAGS, hardware_acceleration) | CONFIG(hardware_acceleration) {
    DEFINES += HARDWARE_ACCELERATION
    message("Hardware acceleration enabled")
}

# 网络特性
contains(FEATURE_FLAGS, p2p_mode) | CONFIG(p2p_mode) {
    DEFINES += P2P_MODE_SUPPORT
    message("P2P mode support enabled")
}

contains(FEATURE_FLAGS, bandwidth_optimization) | CONFIG(bandwidth_optimization) {
    DEFINES += BANDWIDTH_OPTIMIZATION
    message("Bandwidth optimization enabled")
}

# 安全特性
contains(FEATURE_FLAGS, end_to_end_encryption) | CONFIG(e2ee) {
    DEFINES += END_TO_END_ENCRYPTION
    message("End-to-end encryption enabled")
}

# 调试特性
contains(FEATURE_FLAGS, debug_logging) | CONFIG(debug_logging) {
    DEFINES += DEBUG_LOGGING_ENABLED
    message("Debug logging enabled")
}

contains(FEATURE_FLAGS, performance_profiling) | CONFIG(performance_profiling) {
    DEFINES += PERFORMANCE_PROFILING
    message("Performance profiling enabled")
}

######################################################################
# Platform-Specific Compilation
######################################################################

# Windows特定编译选项
win32 {
    DEFINES += PLATFORM_WINDOWS
    
    # Windows特定特性
    contains(FEATURE_FLAGS, windows_integration) | CONFIG(windows_integration) {
        DEFINES += WINDOWS_INTEGRATION
        message("Windows integration features enabled")
    }
    
    # DirectShow支持
    contains(FEATURE_FLAGS, directshow) | CONFIG(directshow) {
        DEFINES += DIRECTSHOW_SUPPORT
        LIBS += -lstrmiids -lole32
        message("DirectShow support enabled")
    }
}

# Linux特定编译选项
unix:!macx {
    DEFINES += PLATFORM_LINUX
    
    # PulseAudio支持
    contains(FEATURE_FLAGS, pulseaudio) | CONFIG(pulseaudio) {
        DEFINES += PULSEAUDIO_SUPPORT
        LIBS += -lpulse
        message("PulseAudio support enabled")
    }
    
    # X11集成
    contains(FEATURE_FLAGS, x11_integration) | CONFIG(x11_integration) {
        DEFINES += X11_INTEGRATION
        LIBS += -lX11 -lXext
        message("X11 integration enabled")
    }
}

# macOS特定编译选项
macx {
    DEFINES += PLATFORM_MACOS
    
    # Core Audio支持
    contains(FEATURE_FLAGS, coreaudio) | CONFIG(coreaudio) {
        DEFINES += COREAUDIO_SUPPORT
        LIBS += -framework CoreAudio -framework AudioUnit
        message("Core Audio support enabled")
    }
}

######################################################################
# Module-Specific Conditional Compilation
######################################################################

# 音频模块条件编译
audio_module_loaded {
    # 高级音频处理
    contains(FEATURE_FLAGS, advanced_audio_processing) {
        DEFINES += ADVANCED_AUDIO_PROCESSING
        INCLUDEPATH += $PWD/audio/advanced
    }
    
    # 音频编解码器
    contains(FEATURE_FLAGS, opus_codec) {
        DEFINES += OPUS_CODEC_SUPPORT
        LIBS += -lopus
    }
}

# 视频模块条件编译
camera_module_loaded {
    # 高级视频处理
    contains(FEATURE_FLAGS, advanced_video_processing) {
        DEFINES += ADVANCED_VIDEO_PROCESSING
        INCLUDEPATH += $PWD/camera/advanced
    }
    
    # 视频编解码器
    contains(FEATURE_FLAGS, h264_codec) {
        DEFINES += H264_CODEC_SUPPORT
        LIBS += -lx264
    }
}

# 网络模块条件编译
network_module_loaded {
    # WebRTC优化
    contains(FEATURE_FLAGS, webrtc_optimization) {
        DEFINES += WEBRTC_OPTIMIZATION
        INCLUDEPATH += $PWD/network/webrtc_optimized
    }
    
    # TURN服务器支持
    contains(FEATURE_FLAGS, turn_server) {
        DEFINES += TURN_SERVER_SUPPORT
    }
}

# UI模块条件编译
ui_module_loaded {
    # 高DPI支持
    contains(FEATURE_FLAGS, high_dpi) {
        DEFINES += HIGH_DPI_SUPPORT
        QT += svg
    }
    
    # 自定义主题
    contains(FEATURE_FLAGS, custom_themes) {
        DEFINES += CUSTOM_THEMES_SUPPORT
        INCLUDEPATH += $PWD/ui/themes/custom
    }
}

######################################################################
# Development Mode Features
######################################################################

# 开发模式
CONFIG(debug, debug|release) {
    # 自动启用调试特性
    DEFINES += DEBUG_MODE
    DEFINES += VERBOSE_LOGGING
    DEFINES += MEMORY_LEAK_DETECTION
    
    # 开发工具
    contains(FEATURE_FLAGS, dev_tools) | CONFIG(dev_tools) {
        DEFINES += DEVELOPMENT_TOOLS
        message("Development tools enabled")
    }
}

# 测试模式
CONFIG(test) {
    DEFINES += TEST_MODE
    DEFINES += MOCK_OBJECTS_ENABLED
    message("Test mode compilation")
}

######################################################################
# Experimental Features
######################################################################

experimental_features {
    message("=== Experimental Features ===")
    
    # AI增强功能
    contains(FEATURE_FLAGS, ai_enhancement) {
        DEFINES += AI_ENHANCEMENT_EXPERIMENTAL
        message("AI enhancement (experimental)")
    }
    
    # 虚拟背景
    contains(FEATURE_FLAGS, virtual_background) {
        DEFINES += VIRTUAL_BACKGROUND_EXPERIMENTAL
        message("Virtual background (experimental)")
    }
    
    # 实时翻译
    contains(FEATURE_FLAGS, real_time_translation) {
        DEFINES += REAL_TIME_TRANSLATION_EXPERIMENTAL
        message("Real-time translation (experimental)")
    }
    
    message("=== End Experimental Features ===")
}

######################################################################
# Feature Summary
######################################################################

message("=== Conditional Compilation Summary ===")
message("Enabled features: $$DEFINES")
message("Platform: $$QMAKE_HOST.os")
message("Architecture: $$QMAKE_HOST.arch")
message("Compiler: $$QMAKE_CXX")
message("==========================================")