# Module Packaging and Distribution Configuration
# 模块打包和分发配置

######################################################################
# Packaging Configuration
######################################################################

# 打包系统版本
PACKAGING_VERSION = 1.0.0
DEFINES += PACKAGING_VERSION=\\\"$PACKAGING_VERSION\\\"

# 打包目录配置
PACKAGE_DIR = $$OUT_PWD/packages
DIST_DIR = $$OUT_PWD/dist
INSTALL_DIR = $$OUT_PWD/install

# 创建打包目录
!exists($$PACKAGE_DIR) {
    system(mkdir -p $$PACKAGE_DIR)
}
!exists($$DIST_DIR) {
    system(mkdir -p $$DIST_DIR)
}
!exists($$INSTALL_DIR) {
    system(mkdir -p $$INSTALL_DIR)
}

######################################################################
# Module Package Definitions
######################################################################

# 核心模块包
CORE_PACKAGE_NAME = jitsi-qt-core
CORE_PACKAGE_VERSION = $$MODULES_VERSION
CORE_PACKAGE_FILES = \
    $$PWD/core/lib*.so* \
    $$PWD/core/include/*.h \
    $$PWD/core/core.pri

# 音频模块包
AUDIO_PACKAGE_NAME = jitsi-qt-audio
AUDIO_PACKAGE_VERSION = $$MODULES_VERSION
AUDIO_PACKAGE_FILES = \
    $$PWD/audio/lib*.so* \
    $$PWD/audio/include/*.h \
    $$PWD/audio/audio.pri \
    $$PWD/audio/resources/*

# 网络模块包
NETWORK_PACKAGE_NAME = jitsi-qt-network
NETWORK_PACKAGE_VERSION = $$MODULES_VERSION
NETWORK_PACKAGE_FILES = \
    $$PWD/network/lib*.so* \
    $$PWD/network/include/*.h \
    $$PWD/network/network.pri

# UI模块包
UI_PACKAGE_NAME = jitsi-qt-ui
UI_PACKAGE_VERSION = $$MODULES_VERSION
UI_PACKAGE_FILES = \
    $$PWD/ui/lib*.so* \
    $$PWD/ui/include/*.h \
    $$PWD/ui/ui.pri \
    $$PWD/ui/resources/* \
    $$PWD/ui/themes/*

# 完整应用包
FULL_PACKAGE_NAME = jitsi-qt-complete
FULL_PACKAGE_VERSION = $$MODULES_VERSION

######################################################################
# Package Creation Targets
######################################################################

# 创建核心模块包
package_core.target = $$PACKAGE_DIR/$${CORE_PACKAGE_NAME}-$${CORE_PACKAGE_VERSION}.tar.gz
package_core.commands = \
    echo "Creating core module package..." && \
    cd $$PWD && \
    tar -czf $$PACKAGE_DIR/$${CORE_PACKAGE_NAME}-$${CORE_PACKAGE_VERSION}.tar.gz \
        --transform 's|^|$${CORE_PACKAGE_NAME}-$${CORE_PACKAGE_VERSION}/|' \
        $$CORE_PACKAGE_FILES

QMAKE_EXTRA_TARGETS += package_core

# 创建音频模块包
audio_module_loaded {
    package_audio.target = $$PACKAGE_DIR/$${AUDIO_PACKAGE_NAME}-$${AUDIO_PACKAGE_VERSION}.tar.gz
    package_audio.commands = \
        echo "Creating audio module package..." && \
        cd $$PWD && \
        tar -czf $$PACKAGE_DIR/$${AUDIO_PACKAGE_NAME}-$${AUDIO_PACKAGE_VERSION}.tar.gz \
            --transform 's|^|$${AUDIO_PACKAGE_NAME}-$${AUDIO_PACKAGE_VERSION}/|' \
            $$AUDIO_PACKAGE_FILES
    
    QMAKE_EXTRA_TARGETS += package_audio
}

# 创建网络模块包
network_module_loaded {
    package_network.target = $$PACKAGE_DIR/$${NETWORK_PACKAGE_NAME}-$${NETWORK_PACKAGE_VERSION}.tar.gz
    package_network.commands = \
        echo "Creating network module package..." && \
        cd $$PWD && \
        tar -czf $$PACKAGE_DIR/$${NETWORK_PACKAGE_NAME}-$${NETWORK_PACKAGE_VERSION}.tar.gz \
            --transform 's|^|$${NETWORK_PACKAGE_NAME}-$${NETWORK_PACKAGE_VERSION}/|' \
            $$NETWORK_PACKAGE_FILES
    
    QMAKE_EXTRA_TARGETS += package_network
}

# 创建UI模块包
ui_module_loaded {
    package_ui.target = $$PACKAGE_DIR/$${UI_PACKAGE_NAME}-$${UI_PACKAGE_VERSION}.tar.gz
    package_ui.commands = \
        echo "Creating UI module package..." && \
        cd $$PWD && \
        tar -czf $$PACKAGE_DIR/$${UI_PACKAGE_NAME}-$${UI_PACKAGE_VERSION}.tar.gz \
            --transform 's|^|$${UI_PACKAGE_NAME}-$${UI_PACKAGE_VERSION}/|' \
            $$UI_PACKAGE_FILES
    
    QMAKE_EXTRA_TARGETS += package_ui
}

# 创建完整应用包
package_full.target = $$PACKAGE_DIR/$${FULL_PACKAGE_NAME}-$${FULL_PACKAGE_VERSION}.tar.gz
package_full.depends = package_core
audio_module_loaded: package_full.depends += package_audio
network_module_loaded: package_full.depends += package_network
ui_module_loaded: package_full.depends += package_ui

package_full.commands = \
    echo "Creating full application package..." && \
    cd $$PACKAGE_DIR && \
    tar -czf $${FULL_PACKAGE_NAME}-$${FULL_PACKAGE_VERSION}.tar.gz \
        $${CORE_PACKAGE_NAME}-$${CORE_PACKAGE_VERSION}.tar.gz \
        $${AUDIO_PACKAGE_NAME}-$${AUDIO_PACKAGE_VERSION}.tar.gz \
        $${NETWORK_PACKAGE_NAME}-$${NETWORK_PACKAGE_VERSION}.tar.gz \
        $${UI_PACKAGE_NAME}-$${UI_PACKAGE_VERSION}.tar.gz

QMAKE_EXTRA_TARGETS += package_full

# 打包所有模块
package_all.target = package_all
package_all.depends = package_full
package_all.commands = echo "All packages created successfully"

QMAKE_EXTRA_TARGETS += package_all

######################################################################
# Distribution Targets
######################################################################

# 创建分发包
create_distribution.target = $$DIST_DIR/jitsi-qt-$${MODULES_VERSION}-distribution.tar.gz
create_distribution.depends = package_all
create_distribution.commands = \
    echo "Creating distribution package..." && \
    cd $$OUT_PWD && \
    tar -czf $$DIST_DIR/jitsi-qt-$${MODULES_VERSION}-distribution.tar.gz \
        --transform 's|^|jitsi-qt-$${MODULES_VERSION}/|' \
        packages/ \
        bin/ \
        lib/ \
        include/ \
        share/

QMAKE_EXTRA_TARGETS += create_distribution

# 创建安装程序
win32 {
    # Windows安装程序 (NSIS)
    create_installer_windows.target = $$DIST_DIR/jitsi-qt-$${MODULES_VERSION}-setup.exe
    create_installer_windows.depends = create_distribution
    create_installer_windows.commands = \
        echo "Creating Windows installer..." && \
        makensis /DVERSION=$${MODULES_VERSION} \
                 /DSOURCE_DIR=$$OUT_PWD \
                 /DOUTPUT_FILE=$$DIST_DIR/jitsi-qt-$${MODULES_VERSION}-setup.exe \
                 $$PWD/packaging/windows/installer.nsi
    
    QMAKE_EXTRA_TARGETS += create_installer_windows
}

unix:!macx {
    # Linux包 (DEB/RPM)
    create_deb_package.target = $$DIST_DIR/jitsi-qt_$${MODULES_VERSION}_amd64.deb
    create_deb_package.depends = create_distribution
    create_deb_package.commands = \
        echo "Creating DEB package..." && \
        $$PWD/packaging/linux/create_deb.sh $$MODULES_VERSION $$OUT_PWD $$DIST_DIR
    
    create_rpm_package.target = $$DIST_DIR/jitsi-qt-$${MODULES_VERSION}.x86_64.rpm
    create_rpm_package.depends = create_distribution
    create_rpm_package.commands = \
        echo "Creating RPM package..." && \
        $$PWD/packaging/linux/create_rpm.sh $$MODULES_VERSION $$OUT_PWD $$DIST_DIR
    
    QMAKE_EXTRA_TARGETS += create_deb_package create_rpm_package
}

macx {
    # macOS应用包和DMG
    create_app_bundle.target = $$DIST_DIR/Jitsi\ Meet\ Qt.app
    create_app_bundle.depends = create_distribution
    create_app_bundle.commands = \
        echo "Creating macOS app bundle..." && \
        $$PWD/packaging/macos/create_app_bundle.sh $$MODULES_VERSION $$OUT_PWD $$DIST_DIR
    
    create_dmg.target = $$DIST_DIR/jitsi-qt-$${MODULES_VERSION}.dmg
    create_dmg.depends = create_app_bundle
    create_dmg.commands = \
        echo "Creating DMG..." && \
        hdiutil create -volname "Jitsi Meet Qt $${MODULES_VERSION}" \
                       -srcfolder "$$DIST_DIR/Jitsi Meet Qt.app" \
                       -ov -format UDZO \
                       "$$DIST_DIR/jitsi-qt-$${MODULES_VERSION}.dmg"
    
    QMAKE_EXTRA_TARGETS += create_app_bundle create_dmg
}

######################################################################
# Installation System
######################################################################

# 安装目标配置
INSTALL_PREFIX = /usr/local
isEmpty(PREFIX): PREFIX = $$INSTALL_PREFIX

# 安装路径
target.path = $$PREFIX/bin
headers.path = $$PREFIX/include/jitsi-qt
libraries.path = $$PREFIX/lib
resources.path = $$PREFIX/share/jitsi-qt
plugins.path = $$PREFIX/lib/jitsi-qt/plugins

# 安装文件
headers.files = $$PWD/*/include/*.h
libraries.files = $$OUT_PWD/lib*.so*
resources.files = $$PWD/*/resources/*
plugins.files = $$PLUGIN_DIR/*.so

# 模块配置文件安装
module_configs.path = $$PREFIX/share/jitsi-qt/modules
module_configs.files = $$PWD/*/*.pri

INSTALLS += target headers libraries resources plugins module_configs

# 卸载目标
uninstall.target = uninstall
uninstall.commands = \
    echo "Uninstalling Jitsi Meet Qt..." && \
    rm -f $$PREFIX/bin/jitsi-meet-qt && \
    rm -rf $$PREFIX/include/jitsi-qt && \
    rm -f $$PREFIX/lib/libjitsi-qt*.so* && \
    rm -rf $$PREFIX/share/jitsi-qt && \
    rm -rf $$PREFIX/lib/jitsi-qt && \
    echo "Uninstallation complete"

QMAKE_EXTRA_TARGETS += uninstall

######################################################################
# Module Installation Tools
######################################################################

# 模块安装脚本
install_module.target = install_module
install_module.commands = \
    echo "Installing module..." && \
    $$PWD/tools/install_module.py $$MODULE_NAME $$MODULE_PACKAGE

# 模块卸载脚本
uninstall_module.target = uninstall_module
uninstall_module.commands = \
    echo "Uninstalling module..." && \
    $$PWD/tools/uninstall_module.py $$MODULE_NAME

QMAKE_EXTRA_TARGETS += install_module uninstall_module

######################################################################
# Package Verification
######################################################################

# 包完整性验证
verify_packages.target = verify_packages
verify_packages.commands = \
    echo "Verifying packages..." && \
    $$PWD/tools/verify_packages.py $$PACKAGE_DIR

# 依赖检查
check_dependencies.target = check_dependencies
check_dependencies.commands = \
    echo "Checking dependencies..." && \
    $$PWD/tools/check_dependencies.py $$OUT_PWD

QMAKE_EXTRA_TARGETS += verify_packages check_dependencies

######################################################################
# Continuous Integration Support
######################################################################

# CI构建目标
ci_build.target = ci_build
ci_build.depends = package_all verify_packages check_dependencies
ci_build.commands = echo "CI build completed successfully"

# CI测试目标
ci_test.target = ci_test
ci_test.commands = \
    echo "Running CI tests..." && \
    $$PWD/tools/run_ci_tests.py $$OUT_PWD

QMAKE_EXTRA_TARGETS += ci_build ci_test

######################################################################
# Packaging Summary
######################################################################

message("=== Packaging Configuration ===")
message("Package directory: $$PACKAGE_DIR")
message("Distribution directory: $$DIST_DIR")
message("Install directory: $$INSTALL_DIR")
message("Install prefix: $$PREFIX")

core_module_loaded {
    message("✓ Core module packaging configured")
}
audio_module_loaded {
    message("✓ Audio module packaging configured")
}
network_module_loaded {
    message("✓ Network module packaging configured")
}
ui_module_loaded {
    message("✓ UI module packaging configured")
}

message("Available targets:")
message("  - package_all: Create all module packages")
message("  - create_distribution: Create distribution package")
message("  - install: Install application")
message("  - uninstall: Uninstall application")
message("  - ci_build: CI build target")
message("===============================")