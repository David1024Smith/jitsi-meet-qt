# Dynamic Loading Configuration
# 动态加载配置

######################################################################
# Dynamic Loading Support
######################################################################

# 启用动态加载支持
dynamic_loading_support {
    DEFINES += DYNAMIC_LOADING_ENABLED
    QT += core
    
    # 动态库配置
    CONFIG += plugin
    CONFIG += shared
    
    message("Dynamic loading support enabled")
}

######################################################################
# Module Plugin Configuration
######################################################################

# 插件目录配置
PLUGIN_DIR = $$OUT_PWD/plugins
DEFINES += PLUGIN_DIR=\\\"$$PLUGIN_DIR\\\"

# 创建插件目录
!exists($$PLUGIN_DIR) {
    system(mkdir -p $$PLUGIN_DIR)
}

######################################################################
# Dynamic Module Definitions
######################################################################

# 音频模块动态加载
audio_module_loaded:dynamic_loading_support {
    # 创建音频模块插件
    AUDIO_PLUGIN_TARGET = audio_module_plugin
    
    # 音频插件配置
    audio_plugin.target = $$PLUGIN_DIR/lib$${AUDIO_PLUGIN_TARGET}.so
    audio_plugin.depends = $$PWD/audio/src/*.cpp
    audio_plugin.commands = \
        cd $$PWD/audio && \
        qmake -o Makefile.plugin audio_plugin.pro && \
        make -f Makefile.plugin && \
        cp lib$${AUDIO_PLUGIN_TARGET}.so $$PLUGIN_DIR/
    
    QMAKE_EXTRA_TARGETS += audio_plugin
    PRE_TARGETDEPS += $$audio_plugin.target
    
    DEFINES += AUDIO_MODULE_DYNAMIC
    message("Audio module configured for dynamic loading")
}

# 网络模块动态加载
network_module_loaded:dynamic_loading_support {
    # 创建网络模块插件
    NETWORK_PLUGIN_TARGET = network_module_plugin
    
    # 网络插件配置
    network_plugin.target = $$PLUGIN_DIR/lib$${NETWORK_PLUGIN_TARGET}.so
    network_plugin.depends = $$PWD/network/src/*.cpp
    network_plugin.commands = \
        cd $$PWD/network && \
        qmake -o Makefile.plugin network_plugin.pro && \
        make -f Makefile.plugin && \
        cp lib$${NETWORK_PLUGIN_TARGET}.so $$PLUGIN_DIR/
    
    QMAKE_EXTRA_TARGETS += network_plugin
    PRE_TARGETDEPS += $$network_plugin.target
    
    DEFINES += NETWORK_MODULE_DYNAMIC
    message("Network module configured for dynamic loading")
}

# UI模块动态加载
ui_module_loaded:dynamic_loading_support {
    # 创建UI模块插件
    UI_PLUGIN_TARGET = ui_module_plugin
    
    # UI插件配置
    ui_plugin.target = $$PLUGIN_DIR/lib$${UI_PLUGIN_TARGET}.so
    ui_plugin.depends = $$PWD/ui/src/*.cpp
    ui_plugin.commands = \
        cd $$PWD/ui && \
        qmake -o Makefile.plugin ui_plugin.pro && \
        make -f Makefile.plugin && \
        cp lib$${UI_PLUGIN_TARGET}.so $$PLUGIN_DIR/
    
    QMAKE_EXTRA_TARGETS += ui_plugin
    PRE_TARGETDEPS += $$ui_plugin.target
    
    DEFINES += UI_MODULE_DYNAMIC
    message("UI module configured for dynamic loading")
}

# 聊天模块动态加载
chat_module_loaded:dynamic_loading_support {
    # 创建聊天模块插件
    CHAT_PLUGIN_TARGET = chat_module_plugin
    
    # 聊天插件配置
    chat_plugin.target = $$PLUGIN_DIR/lib$${CHAT_PLUGIN_TARGET}.so
    chat_plugin.depends = $$PWD/chat/src/*.cpp
    chat_plugin.commands = \
        cd $$PWD/chat && \
        qmake -o Makefile.plugin chat_plugin.pro && \
        make -f Makefile.plugin && \
        cp lib$${CHAT_PLUGIN_TARGET}.so $$PLUGIN_DIR/
    
    QMAKE_EXTRA_TARGETS += chat_plugin
    PRE_TARGETDEPS += $$chat_plugin.target
    
    DEFINES += CHAT_MODULE_DYNAMIC
    message("Chat module configured for dynamic loading")
}

######################################################################
# Plugin Interface Definitions
######################################################################

# 插件接口头文件
PLUGIN_INTERFACE_HEADERS = \
    $$PWD/interfaces/IModulePlugin.h \
    $$PWD/interfaces/IAudioModulePlugin.h \
    $$PWD/interfaces/INetworkModulePlugin.h \
    $$PWD/interfaces/IUIModulePlugin.h \
    $$PWD/interfaces/IChatModulePlugin.h

# 确保插件接口存在
for(header, PLUGIN_INTERFACE_HEADERS) {
    !exists($$header) {
        warning("Plugin interface header not found: $$header")
    }
}

######################################################################
# Runtime Plugin Loading
######################################################################

# 运行时插件加载器
PLUGIN_LOADER_SOURCES = \
    $$PWD/core/src/PluginLoader.cpp \
    $$PWD/core/src/ModulePluginManager.cpp

PLUGIN_LOADER_HEADERS = \
    $$PWD/core/include/PluginLoader.h \
    $$PWD/core/include/ModulePluginManager.h

# 添加插件加载器到构建
SOURCES += $$PLUGIN_LOADER_SOURCES
HEADERS += $$PLUGIN_LOADER_HEADERS

######################################################################
# Plugin Metadata
######################################################################

# 插件元数据文件
PLUGIN_METADATA_DIR = $$PLUGIN_DIR/metadata

# 创建元数据目录
!exists($$PLUGIN_METADATA_DIR) {
    system(mkdir -p $$PLUGIN_METADATA_DIR)
}

# 生成插件元数据
generate_plugin_metadata.target = $$PLUGIN_METADATA_DIR/plugins.json
generate_plugin_metadata.commands = \
    echo "Generating plugin metadata..." && \
    $$PWD/tools/generate_plugin_metadata.py $$PLUGIN_DIR $$PLUGIN_METADATA_DIR/plugins.json

QMAKE_EXTRA_TARGETS += generate_plugin_metadata
POST_TARGETDEPS += $$generate_plugin_metadata.target

######################################################################
# Plugin Verification
######################################################################

# 插件验证
verify_plugins.target = verify_plugins
verify_plugins.commands = \
    echo "Verifying plugins..." && \
    $$PWD/tools/verify_plugins.py $$PLUGIN_DIR

QMAKE_EXTRA_TARGETS += verify_plugins

######################################################################
# Hot Reload Support (Development Mode)
######################################################################

CONFIG(debug, debug|release):dynamic_loading_support {
    # 热重载支持
    DEFINES += HOT_RELOAD_ENABLED
    
    # 文件监控
    QT += core
    SOURCES += $$PWD/core/src/HotReloadManager.cpp
    HEADERS += $$PWD/core/include/HotReloadManager.h
    
    message("Hot reload support enabled for development")
}

######################################################################
# Plugin Security
######################################################################

# 插件签名验证
CONFIG(release, debug|release):dynamic_loading_support {
    DEFINES += PLUGIN_SIGNATURE_VERIFICATION
    
    # 签名验证工具
    sign_plugins.target = sign_plugins
    sign_plugins.commands = \
        echo "Signing plugins..." && \
        $$PWD/tools/sign_plugins.py $$PLUGIN_DIR
    
    QMAKE_EXTRA_TARGETS += sign_plugins
    POST_TARGETDEPS += $$sign_plugins.target
    
    message("Plugin signature verification enabled")
}

######################################################################
# Dynamic Loading Summary
######################################################################

dynamic_loading_support {
    message("=== Dynamic Loading Configuration ===")
    message("Plugin directory: $$PLUGIN_DIR")
    message("Metadata directory: $$PLUGIN_METADATA_DIR")
    
    audio_module_loaded {
        message("✓ Audio module plugin configured")
    }
    network_module_loaded {
        message("✓ Network module plugin configured")
    }
    ui_module_loaded {
        message("✓ UI module plugin configured")
    }
    chat_module_loaded {
        message("✓ Chat module plugin configured")
    }
    
    message("===================================")
}