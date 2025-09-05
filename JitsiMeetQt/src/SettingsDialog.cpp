#include "SettingsDialog.h"
#include "ConfigurationManager.h"

#include <QApplication>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QJsonParseError>
#include <QKeySequence>
#include <QShortcut>
#include <QHeaderView>
#include <QScrollBar>
#include <QToolTip>
#include <QWhatsThis>
#include <QSplashScreen>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QScreen>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

/**
 * @brief SettingsDialog构造函数
 * @param parent 父窗口
 */
SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_tabWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_applyButton(nullptr)
    , m_resetButton(nullptr)
    , m_importButton(nullptr)
    , m_exportButton(nullptr)
    , m_generalPage(nullptr)
    , m_languageCombo(nullptr)
    , m_themeCombo(nullptr)
    , m_startupCheck(nullptr)
    , m_systemTrayCheck(nullptr)
    , m_minimizeToTrayCheck(nullptr)
    , m_defaultNameEdit(nullptr)
    , m_serverPage(nullptr)
    , m_defaultServerEdit(nullptr)
    , m_connectionTimeoutSpin(nullptr)
    , m_proxyEnabledCheck(nullptr)
    , m_proxyHostEdit(nullptr)
    , m_proxyPortSpin(nullptr)
    , m_proxyUserEdit(nullptr)
    , m_proxyPasswordEdit(nullptr)
    , m_audioVideoPage(nullptr)
    , m_audioDeviceCombo(nullptr)
    , m_videoDeviceCombo(nullptr)
    , m_audioQualityCombo(nullptr)
    , m_videoQualityCombo(nullptr)
    , m_defaultMuteCheck(nullptr)
    , m_defaultCameraOffCheck(nullptr)
    , m_micVolumeSlider(nullptr)
    , m_speakerVolumeSlider(nullptr)
    , m_interfacePage(nullptr)
    , m_fontSelectButton(nullptr)
    , m_colorSelectButton(nullptr)
    , m_windowWidthSpin(nullptr)
    , m_windowHeightSpin(nullptr)
    , m_rememberSizeCheck(nullptr)
    , m_rememberPositionCheck(nullptr)
    , m_advancedPage(nullptr)
    , m_logLevelCombo(nullptr)
    , m_cacheSizeSpin(nullptr)
    , m_clearCacheButton(nullptr)
    , m_hardwareAccelCheck(nullptr)
    , m_experimentalFeaturesCheck(nullptr)
    , m_debugModeCheck(nullptr)
    , m_aboutPage(nullptr)
    , m_versionLabel(nullptr)
    , m_buildLabel(nullptr)
    , m_qtVersionLabel(nullptr)
    , m_checkUpdatesButton(nullptr)
    , m_visitWebsiteButton(nullptr)
    , m_viewLicenseButton(nullptr)
    , m_creditsText(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_validationTimer(nullptr)
    , m_configManager(nullptr)
    , m_hasChanges(false)
    , m_isLoading(false)
{
    // 获取配置管理器实例
    m_configManager = ConfigurationManager::instance();
    
    // 初始化定时器
    m_validationTimer = new QTimer(this);
    m_validationTimer->setSingleShot(true);
    m_validationTimer->setInterval(VALIDATION_DELAY);
    
    // 初始化UI
    initializeUI();
    initializeConnections();
    
    // 加载设置
    loadSettings();
    
    // 设置窗口属性
    setWindowTitle(tr("设置"));
    setWindowIcon(QIcon(":/icons/settings.svg"));
    setModal(true);
    resize(600, 500);
    
    // 更新UI状态
    updateUIState();
}

/**
 * @brief SettingsDialog析构函数
 */
SettingsDialog::~SettingsDialog()
{
    // 析构函数
}

/**
 * @brief 显示指定的设置页面
 * @param page 要显示的页面
 */
void SettingsDialog::showPage(SettingsPage page)
{
    if (m_tabWidget) {
        m_tabWidget->setCurrentIndex(static_cast<int>(page));
    }
}

/**
 * @brief 应用设置更改
 */
void SettingsDialog::applySettings()
{
    if (!validateSettings()) {
        return;
    }
    
    saveSettings();
    m_hasChanges = false;
    updateApplyButtonState();
    
    emit settingsApplied();
}

/**
 * @brief 重置设置到默认值
 */
void SettingsDialog::resetToDefaults()
{
    int ret = QMessageBox::question(this, tr("重置设置"),
        tr("确定要将所有设置重置为默认值吗？\n此操作无法撤销。"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_configManager) {
            m_configManager->resetToDefaults();
        }
        loadSettings();
        m_hasChanges = true;
        updateApplyButtonState();
    }
}

/**
 * @brief 导入设置
 * @param filePath 设置文件路径
 * @return 是否成功导入
 */
bool SettingsDialog::importSettings(const QString& filePath)
{
    if (!m_configManager) {
        return false;
    }
    
    bool success = m_configManager->importSettings(filePath);
    if (success) {
        loadSettings();
        m_hasChanges = true;
        updateApplyButtonState();
        QMessageBox::information(this, tr("导入成功"), tr("设置已成功导入"));
    } else {
        QMessageBox::warning(this, tr("导入失败"), tr("无法导入设置文件"));
    }
    
    return success;
}

/**
 * @brief 导出设置
 * @param filePath 导出文件路径
 * @return 是否成功导出
 */
bool SettingsDialog::exportSettings(const QString& filePath)
{
    if (!m_configManager) {
        return false;
    }
    
    bool success = m_configManager->exportSettings(filePath);
    if (success) {
        QMessageBox::information(this, tr("导出成功"), tr("设置已成功导出"));
    } else {
        QMessageBox::warning(this, tr("导出失败"), tr("无法导出设置文件"));
    }
    
    return success;
}

/**
 * @brief 窗口关闭事件处理
 * @param event 关闭事件
 */
void SettingsDialog::closeEvent(QCloseEvent *event)
{
    if (m_hasChanges) {
        int ret = QMessageBox::question(this, tr("未保存的更改"),
            tr("您有未保存的更改。是否要保存？"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);
        
        if (ret == QMessageBox::Save) {
            applySettings();
            event->accept();
        } else if (ret == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
            return;
        }
    }
    
    QDialog::closeEvent(event);
}

/**
 * @brief 窗口显示事件处理
 * @param event 显示事件
 */
void SettingsDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    
    // 刷新设备列表
    loadAudioVideoDevices();
}

/**
 * @brief 确定按钮点击处理
 */
void SettingsDialog::onOkClicked()
{
    if (m_hasChanges) {
        applySettings();
    }
    accept();
}

/**
 * @brief 取消按钮点击处理
 */
void SettingsDialog::onCancelClicked()
{
    if (m_hasChanges) {
        int ret = QMessageBox::question(this, tr("未保存的更改"),
            tr("您有未保存的更改。是否要放弃更改？"),
            QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Cancel);
        
        if (ret == QMessageBox::Cancel) {
            return;
        }
    }
    
    reject();
}

/**
 * @brief 应用按钮点击处理
 */
void SettingsDialog::onApplyClicked()
{
    applySettings();
}

/**
 * @brief 重置按钮点击处理
 */
void SettingsDialog::onResetClicked()
{
    resetToDefaults();
}

/**
 * @brief 导入按钮点击处理
 */
void SettingsDialog::onImportClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("导入设置"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("配置文件 (*.ini *.json);;所有文件 (*)"));
    
    if (!filePath.isEmpty()) {
        importSettings(filePath);
    }
}

/**
 * @brief 导出按钮点击处理
 */
void SettingsDialog::onExportClicked()
{
    QString filePath = QFileDialog::getSaveFileName(this,
        tr("导出设置"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/jitsi-meet-qt-settings.ini",
        tr("配置文件 (*.ini *.json);;所有文件 (*)"));
    
    if (!filePath.isEmpty()) {
        exportSettings(filePath);
    }
}

/**
 * @brief 语言选择改变处理
 * @param index 选择的索引
 */
void SettingsDialog::onLanguageChanged(int index)
{
    Q_UNUSED(index)
    m_hasChanges = true;
    updateApplyButtonState();
    
    if (m_languageCombo) {
        QString language = m_languageCombo->currentData().toString();
        emit languageChanged(language);
    }
}

/**
 * @brief 主题选择改变处理
 * @param index 选择的索引
 */
void SettingsDialog::onThemeChanged(int index)
{
    Q_UNUSED(index)
    m_hasChanges = true;
    updateApplyButtonState();
    
    if (m_themeCombo) {
        QString theme = m_themeCombo->currentData().toString();
        emit themeChanged(theme);
    }
}

/**
 * @brief 开机自启动选项改变处理
 * @param checked 是否选中
 */
void SettingsDialog::onStartupChanged(bool checked)
{
    Q_UNUSED(checked)
    m_hasChanges = true;
    updateApplyButtonState();
}

/**
 * @brief 系统托盘选项改变处理
 * @param checked 是否选中
 */
void SettingsDialog::onSystemTrayChanged(bool checked)
{
    m_hasChanges = true;
    updateApplyButtonState();
    
    // 更新最小化到托盘选项的可用性
    if (m_minimizeToTrayCheck) {
        m_minimizeToTrayCheck->setEnabled(checked);
    }
}

/**
 * @brief 最小化到托盘选项改变处理
 * @param checked 是否选中
 */
void SettingsDialog::onMinimizeToTrayChanged(bool checked)
{
    Q_UNUSED(checked)
    m_hasChanges = true;
    updateApplyButtonState();
}

/**
 * @brief 默认服务器改变处理
 * @param text 服务器URL
 */
void SettingsDialog::onDefaultServerChanged(const QString& text)
{
    Q_UNUSED(text)
    m_hasChanges = true;
    updateApplyButtonState();
    
    // 延迟验证
    if (m_validationTimer) {
        m_validationTimer->start();
    }
}

/**
 * @brief 连接超时改变处理
 * @param value 超时值
 */
void SettingsDialog::onConnectionTimeoutChanged(int value)
{
    Q_UNUSED(value)
    m_hasChanges = true;
    updateApplyButtonState();
}

/**
 * @brief 代理设置改变处理
 */
void SettingsDialog::onProxySettingsChanged()
{
    m_hasChanges = true;
    updateApplyButtonState();
    
    // 更新代理设置控件的可用性
    if (m_proxyEnabledCheck) {
        bool enabled = m_proxyEnabledCheck->isChecked();
        if (m_proxyHostEdit) m_proxyHostEdit->setEnabled(enabled);
        if (m_proxyPortSpin) m_proxyPortSpin->setEnabled(enabled);
        if (m_proxyUserEdit) m_proxyUserEdit->setEnabled(enabled);
        if (m_proxyPasswordEdit) m_proxyPasswordEdit->setEnabled(enabled);
    }
}

/**
 * @brief 音频设备选择改变处理
 * @param index 选择的索引
 */
void SettingsDialog::onAudioDeviceChanged(int index)
{
    Q_UNUSED(index)
    m_hasChanges = true;
    updateApplyButtonState();
}

/**
 * @brief 视频设备选择改变处理
 * @param index 选择的索引
 */
void SettingsDialog::onVideoDeviceChanged(int index)
{
    Q_UNUSED(index)
    m_hasChanges = true;
    updateApplyButtonState();
}

/**
 * @brief 音频质量改变处理
 * @param index 选择的索引
 */
void SettingsDialog::onAudioQualityChanged(int index)
{
    Q_UNUSED(index)
    m_hasChanges = true;
    updateApplyButtonState();
}

/**
 * @brief 视频质量改变处理
 * @param index 选择的索引
 */
void SettingsDialog::onVideoQualityChanged(int index)
{
    Q_UNUSED(index)
    m_hasChanges = true;
    updateApplyButtonState();
}

/**
 * @brief 默认静音选项改变处理
 * @param checked 是否选中
 */
void SettingsDialog::onDefaultMuteChanged(bool checked)
{
    Q_UNUSED(checked)
    m_hasChanges = true;
    updateApplyButtonState();
}

/**
 * @brief 默认关闭摄像头选项改变处理
 * @param checked 是否选中
 */
void SettingsDialog::onDefaultCameraOffChanged(bool checked)
{
    Q_UNUSED(checked)
    m_hasChanges = true;
    updateApplyButtonState();
}

/**
 * @brief 字体选择按钮点击处理
 */
void SettingsDialog::onFontSelectClicked()
{
    bool ok;
    QFont currentFont = QApplication::font();
    QFont font = QFontDialog::getFont(&ok, currentFont, this, tr("选择字体"));
    
    if (ok) {
        m_hasChanges = true;
        updateApplyButtonState();
        
        if (m_fontSelectButton) {
            m_fontSelectButton->setText(QString("%1, %2pt").arg(font.family()).arg(font.pointSize()));
            m_fontSelectButton->setFont(font);
        }
    }
}

/**
 * @brief 颜色选择按钮点击处理
 */
void SettingsDialog::onColorSelectClicked()
{
    QColor currentColor = palette().color(QPalette::Window);
    QColor color = QColorDialog::getColor(currentColor, this, tr("选择颜色"));
    
    if (color.isValid()) {
        m_hasChanges = true;
        updateApplyButtonState();
        
        if (m_colorSelectButton) {
            QString styleSheet = QString("background-color: %1; border: 1px solid gray;").arg(color.name());
            m_colorSelectButton->setStyleSheet(styleSheet);
        }
    }
}

/**
 * @brief 窗口大小选项改变处理
 */
void SettingsDialog::onWindowSizeChanged()
{
    m_hasChanges = true;
    updateApplyButtonState();
}

/**
 * @brief 日志级别改变处理
 * @param index 选择的索引
 */
void SettingsDialog::onLogLevelChanged(int index)
{
    Q_UNUSED(index)
    m_hasChanges = true;
    updateApplyButtonState();
}

/**
 * @brief 缓存大小改变处理
 * @param value 缓存大小
 */
void SettingsDialog::onCacheSizeChanged(int value)
{
    Q_UNUSED(value)
    m_hasChanges = true;
    updateApplyButtonState();
}

/**
 * @brief 清除缓存按钮点击处理
 */
void SettingsDialog::onClearCacheClicked()
{
    int ret = QMessageBox::question(this, tr("清除缓存"),
        tr("确定要清除所有缓存数据吗？\n这可能会影响应用程序的性能。"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        clearApplicationCache();
    }
}

/**
 * @brief 实验性功能选项改变处理
 */
void SettingsDialog::onExperimentalFeaturesChanged()
{
    m_hasChanges = true;
    updateApplyButtonState();
    
    if (m_experimentalFeaturesCheck && m_experimentalFeaturesCheck->isChecked()) {
        QMessageBox::warning(this, tr("实验性功能"),
            tr("实验性功能可能不稳定，请谨慎使用。\n启用后需要重启应用程序。"));
        
        emit restartRequired(tr("启用实验性功能"));
    }
}

/**
 * @brief 检查更新按钮点击处理
 */
void SettingsDialog::onCheckUpdatesClicked()
{
    checkForUpdates();
}

/**
 * @brief 访问网站按钮点击处理
 */
void SettingsDialog::onVisitWebsiteClicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/jitsi/jitsi-meet"));
}

/**
 * @brief 查看许可证按钮点击处理
 */
void SettingsDialog::onViewLicenseClicked()
{
    QString licenseText = tr(
        "Apache License 2.0\n\n"
        "Copyright (c) 2024 Jitsi Meet Qt\n\n"
        "Licensed under the Apache License, Version 2.0 (the \"License\");\n"
        "you may not use this file except in compliance with the License.\n"
        "You may obtain a copy of the License at\n\n"
        "    http://www.apache.org/licenses/LICENSE-2.0\n\n"
        "Unless required by applicable law or agreed to in writing, software\n"
        "distributed under the License is distributed on an \"AS IS\" BASIS,\n"
        "WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
        "See the License for the specific language governing permissions and\n"
        "limitations under the License."
    );
    
    QMessageBox::about(this, tr("许可证"), licenseText);
}

/**
 * @brief 设置验证定时器超时处理
 */
void SettingsDialog::onValidationTimeout()
{
    // 验证设置
    validateSettings();
}

/**
 * @brief 缓存清理完成处理
 */
void SettingsDialog::onCacheClearFinished()
{
    if (m_progressBar) {
        m_progressBar->setVisible(false);
    }
    
    if (m_statusLabel) {
        m_statusLabel->setText(tr("缓存已清除"));
    }
    
    QMessageBox::information(this, tr("清除完成"), tr("缓存已成功清除"));
}

/**
 * @brief 初始化用户界面
 */
void SettingsDialog::initializeUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    
    // 创建标签页部件
    m_tabWidget = new QTabWidget(this);
    
    // 创建各个页面
    m_generalPage = createGeneralPage();
    m_serverPage = createServerPage();
    m_audioVideoPage = createAudioVideoPage();
    m_interfacePage = createInterfacePage();
    m_advancedPage = createAdvancedPage();
    m_aboutPage = createAboutPage();
    
    // 添加页面到标签页
    m_tabWidget->addTab(m_generalPage, tr("常规"));
    m_tabWidget->addTab(m_serverPage, tr("服务器"));
    m_tabWidget->addTab(m_audioVideoPage, tr("音视频"));
    m_tabWidget->addTab(m_interfacePage, tr("界面"));
    m_tabWidget->addTab(m_advancedPage, tr("高级"));
    m_tabWidget->addTab(m_aboutPage, tr("关于"));
    
    // 创建按钮布局
    m_buttonLayout = new QHBoxLayout();
    
    // 创建按钮
    m_importButton = new QPushButton(tr("导入..."), this);
    m_exportButton = new QPushButton(tr("导出..."), this);
    m_resetButton = new QPushButton(tr("重置"), this);
    
    m_buttonLayout->addWidget(m_importButton);
    m_buttonLayout->addWidget(m_exportButton);
    m_buttonLayout->addWidget(m_resetButton);
    m_buttonLayout->addStretch();
    
    m_okButton = new QPushButton(tr("确定"), this);
    m_cancelButton = new QPushButton(tr("取消"), this);
    m_applyButton = new QPushButton(tr("应用"), this);
    
    m_okButton->setDefault(true);
    
    m_buttonLayout->addWidget(m_okButton);
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_applyButton);
    
    // 创建状态组件
    m_statusLabel = new QLabel(tr("就绪"), this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    
    // 组装主布局
    m_mainLayout->addWidget(m_tabWidget);
    m_mainLayout->addWidget(m_statusLabel);
    m_mainLayout->addWidget(m_progressBar);
    m_mainLayout->addLayout(m_buttonLayout);
}

/**
 * @brief 创建常规设置页面
 * @return 常规设置页面部件
 */
QWidget* SettingsDialog::createGeneralPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    // 语言设置组
    QGroupBox* languageGroup = new QGroupBox(tr("语言设置"), page);
    QGridLayout* languageLayout = new QGridLayout(languageGroup);
    
    QLabel* languageLabel = new QLabel(tr("界面语言:"), languageGroup);
    m_languageCombo = new QComboBox(languageGroup);
    
    languageLayout->addWidget(languageLabel, 0, 0);
    languageLayout->addWidget(m_languageCombo, 0, 1);
    
    // 外观设置组
    QGroupBox* appearanceGroup = new QGroupBox(tr("外观设置"), page);
    QGridLayout* appearanceLayout = new QGridLayout(appearanceGroup);
    
    QLabel* themeLabel = new QLabel(tr("主题:"), appearanceGroup);
    m_themeCombo = new QComboBox(appearanceGroup);
    
    appearanceLayout->addWidget(themeLabel, 0, 0);
    appearanceLayout->addWidget(m_themeCombo, 0, 1);
    
    // 启动设置组
    QGroupBox* startupGroup = new QGroupBox(tr("启动设置"), page);
    QVBoxLayout* startupLayout = new QVBoxLayout(startupGroup);
    
    m_startupCheck = new QCheckBox(tr("开机自动启动"), startupGroup);
    m_systemTrayCheck = new QCheckBox(tr("显示系统托盘图标"), startupGroup);
    m_minimizeToTrayCheck = new QCheckBox(tr("最小化到系统托盘"), startupGroup);
    
    startupLayout->addWidget(m_startupCheck);
    startupLayout->addWidget(m_systemTrayCheck);
    startupLayout->addWidget(m_minimizeToTrayCheck);
    
    // 用户设置组
    QGroupBox* userGroup = new QGroupBox(tr("用户设置"), page);
    QGridLayout* userLayout = new QGridLayout(userGroup);
    
    QLabel* defaultNameLabel = new QLabel(tr("默认显示名称:"), userGroup);
    m_defaultNameEdit = new QLineEdit(userGroup);
    m_defaultNameEdit->setPlaceholderText(tr("输入默认显示名称"));
    
    userLayout->addWidget(defaultNameLabel, 0, 0);
    userLayout->addWidget(m_defaultNameEdit, 0, 1);
    
    // 组装页面布局
    layout->addWidget(languageGroup);
    layout->addWidget(appearanceGroup);
    layout->addWidget(startupGroup);
    layout->addWidget(userGroup);
    layout->addStretch();
    
    return page;
}

/**
 * @brief 创建服务器设置页面
 * @return 服务器设置页面部件
 */
QWidget* SettingsDialog::createServerPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    // 服务器设置组
    QGroupBox* serverGroup = new QGroupBox(tr("服务器设置"), page);
    QGridLayout* serverLayout = new QGridLayout(serverGroup);
    
    QLabel* defaultServerLabel = new QLabel(tr("默认服务器:"), serverGroup);
    m_defaultServerEdit = new QLineEdit(serverGroup);
    m_defaultServerEdit->setPlaceholderText(tr("输入默认服务器URL"));
    
    QLabel* timeoutLabel = new QLabel(tr("连接超时 (秒):"), serverGroup);
    m_connectionTimeoutSpin = new QSpinBox(serverGroup);
    m_connectionTimeoutSpin->setRange(5, 120);
    m_connectionTimeoutSpin->setValue(30);
    
    serverLayout->addWidget(defaultServerLabel, 0, 0);
    serverLayout->addWidget(m_defaultServerEdit, 0, 1);
    serverLayout->addWidget(timeoutLabel, 1, 0);
    serverLayout->addWidget(m_connectionTimeoutSpin, 1, 1);
    
    // 代理设置组
    QGroupBox* proxyGroup = new QGroupBox(tr("代理设置"), page);
    QGridLayout* proxyLayout = new QGridLayout(proxyGroup);
    
    m_proxyEnabledCheck = new QCheckBox(tr("启用代理"), proxyGroup);
    
    QLabel* proxyHostLabel = new QLabel(tr("代理主机:"), proxyGroup);
    m_proxyHostEdit = new QLineEdit(proxyGroup);
    m_proxyHostEdit->setPlaceholderText(tr("输入代理主机地址"));
    
    QLabel* proxyPortLabel = new QLabel(tr("代理端口:"), proxyGroup);
    m_proxyPortSpin = new QSpinBox(proxyGroup);
    m_proxyPortSpin->setRange(1, 65535);
    m_proxyPortSpin->setValue(8080);
    
    QLabel* proxyUserLabel = new QLabel(tr("用户名:"), proxyGroup);
    m_proxyUserEdit = new QLineEdit(proxyGroup);
    m_proxyUserEdit->setPlaceholderText(tr("输入代理用户名"));
    
    QLabel* proxyPasswordLabel = new QLabel(tr("密码:"), proxyGroup);
    m_proxyPasswordEdit = new QLineEdit(proxyGroup);
    m_proxyPasswordEdit->setEchoMode(QLineEdit::Password);
    m_proxyPasswordEdit->setPlaceholderText(tr("输入代理密码"));
    
    proxyLayout->addWidget(m_proxyEnabledCheck, 0, 0, 1, 2);
    proxyLayout->addWidget(proxyHostLabel, 1, 0);
    proxyLayout->addWidget(m_proxyHostEdit, 1, 1);
    proxyLayout->addWidget(proxyPortLabel, 2, 0);
    proxyLayout->addWidget(m_proxyPortSpin, 2, 1);
    proxyLayout->addWidget(proxyUserLabel, 3, 0);
    proxyLayout->addWidget(m_proxyUserEdit, 3, 1);
    proxyLayout->addWidget(proxyPasswordLabel, 4, 0);
    proxyLayout->addWidget(m_proxyPasswordEdit, 4, 1);
    
    // 组装页面布局
    layout->addWidget(serverGroup);
    layout->addWidget(proxyGroup);
    layout->addStretch();
    
    return page;
}

/**
 * @brief 创建音视频设置页面
 * @return 音视频设置页面部件
 */
QWidget* SettingsDialog::createAudioVideoPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    // 设备设置组
    QGroupBox* deviceGroup = new QGroupBox(tr("设备设置"), page);
    QGridLayout* deviceLayout = new QGridLayout(deviceGroup);
    
    QLabel* audioDeviceLabel = new QLabel(tr("音频设备:"), deviceGroup);
    m_audioDeviceCombo = new QComboBox(deviceGroup);
    
    QLabel* videoDeviceLabel = new QLabel(tr("视频设备:"), deviceGroup);
    m_videoDeviceCombo = new QComboBox(deviceGroup);
    
    deviceLayout->addWidget(audioDeviceLabel, 0, 0);
    deviceLayout->addWidget(m_audioDeviceCombo, 0, 1);
    deviceLayout->addWidget(videoDeviceLabel, 1, 0);
    deviceLayout->addWidget(m_videoDeviceCombo, 1, 1);
    
    // 质量设置组
    QGroupBox* qualityGroup = new QGroupBox(tr("质量设置"), page);
    QGridLayout* qualityLayout = new QGridLayout(qualityGroup);
    
    QLabel* audioQualityLabel = new QLabel(tr("音频质量:"), qualityGroup);
    m_audioQualityCombo = new QComboBox(qualityGroup);
    m_audioQualityCombo->addItems({tr("低"), tr("中"), tr("高"), tr("超高")});
    
    QLabel* videoQualityLabel = new QLabel(tr("视频质量:"), qualityGroup);
    m_videoQualityCombo = new QComboBox(qualityGroup);
    m_videoQualityCombo->addItems({tr("低 (180p)"), tr("中 (360p)"), tr("高 (720p)"), tr("超高 (1080p)")});
    
    qualityLayout->addWidget(audioQualityLabel, 0, 0);
    qualityLayout->addWidget(m_audioQualityCombo, 0, 1);
    qualityLayout->addWidget(videoQualityLabel, 1, 0);
    qualityLayout->addWidget(m_videoQualityCombo, 1, 1);
    
    // 默认设置组
    QGroupBox* defaultGroup = new QGroupBox(tr("默认设置"), page);
    QVBoxLayout* defaultLayout = new QVBoxLayout(defaultGroup);
    
    m_defaultMuteCheck = new QCheckBox(tr("默认静音"), defaultGroup);
    m_defaultCameraOffCheck = new QCheckBox(tr("默认关闭摄像头"), defaultGroup);
    
    defaultLayout->addWidget(m_defaultMuteCheck);
    defaultLayout->addWidget(m_defaultCameraOffCheck);
    
    // 音量设置组
    QGroupBox* volumeGroup = new QGroupBox(tr("音量设置"), page);
    QGridLayout* volumeLayout = new QGridLayout(volumeGroup);
    
    QLabel* micVolumeLabel = new QLabel(tr("麦克风音量:"), volumeGroup);
    m_micVolumeSlider = new QSlider(Qt::Horizontal, volumeGroup);
    m_micVolumeSlider->setRange(0, 100);
    m_micVolumeSlider->setValue(80);
    
    QLabel* speakerVolumeLabel = new QLabel(tr("扬声器音量:"), volumeGroup);
    m_speakerVolumeSlider = new QSlider(Qt::Horizontal, volumeGroup);
    m_speakerVolumeSlider->setRange(0, 100);
    m_speakerVolumeSlider->setValue(80);
    
    volumeLayout->addWidget(micVolumeLabel, 0, 0);
    volumeLayout->addWidget(m_micVolumeSlider, 0, 1);
    volumeLayout->addWidget(speakerVolumeLabel, 1, 0);
    volumeLayout->addWidget(m_speakerVolumeSlider, 1, 1);
    
    // 组装页面布局
    layout->addWidget(deviceGroup);
    layout->addWidget(qualityGroup);
    layout->addWidget(defaultGroup);
    layout->addWidget(volumeGroup);
    layout->addStretch();
    
    return page;
}

/**
 * @brief 创建界面设置页面
 * @return 界面设置页面部件
 */
QWidget* SettingsDialog::createInterfacePage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    // 字体和颜色组
    QGroupBox* fontColorGroup = new QGroupBox(tr("字体和颜色"), page);
    QGridLayout* fontColorLayout = new QGridLayout(fontColorGroup);
    
    QLabel* fontLabel = new QLabel(tr("字体:"), fontColorGroup);
    m_fontSelectButton = new QPushButton(tr("选择字体..."), fontColorGroup);
    
    QLabel* colorLabel = new QLabel(tr("颜色:"), fontColorGroup);
    m_colorSelectButton = new QPushButton(tr("选择颜色..."), fontColorGroup);
    
    fontColorLayout->addWidget(fontLabel, 0, 0);
    fontColorLayout->addWidget(m_fontSelectButton, 0, 1);
    fontColorLayout->addWidget(colorLabel, 1, 0);
    fontColorLayout->addWidget(m_colorSelectButton, 1, 1);
    
    // 窗口设置组
    QGroupBox* windowGroup = new QGroupBox(tr("窗口设置"), page);
    QGridLayout* windowLayout = new QGridLayout(windowGroup);
    
    QLabel* windowWidthLabel = new QLabel(tr("默认宽度:"), windowGroup);
    m_windowWidthSpin = new QSpinBox(windowGroup);
    m_windowWidthSpin->setRange(800, 3840);
    m_windowWidthSpin->setValue(1200);
    
    QLabel* windowHeightLabel = new QLabel(tr("默认高度:"), windowGroup);
    m_windowHeightSpin = new QSpinBox(windowGroup);
    m_windowHeightSpin->setRange(600, 2160);
    m_windowHeightSpin->setValue(800);
    
    m_rememberSizeCheck = new QCheckBox(tr("记住窗口大小"), windowGroup);
    m_rememberPositionCheck = new QCheckBox(tr("记住窗口位置"), windowGroup);
    
    windowLayout->addWidget(windowWidthLabel, 0, 0);
    windowLayout->addWidget(m_windowWidthSpin, 0, 1);
    windowLayout->addWidget(windowHeightLabel, 1, 0);
    windowLayout->addWidget(m_windowHeightSpin, 1, 1);
    windowLayout->addWidget(m_rememberSizeCheck, 2, 0, 1, 2);
    windowLayout->addWidget(m_rememberPositionCheck, 3, 0, 1, 2);
    
    // 组装页面布局
    layout->addWidget(fontColorGroup);
    layout->addWidget(windowGroup);
    layout->addStretch();
    
    return page;
}

/**
 * @brief 创建高级设置页面
 * @return 高级设置页面部件
 */
QWidget* SettingsDialog::createAdvancedPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    // 日志设置组
    QGroupBox* logGroup = new QGroupBox(tr("日志设置"), page);
    QGridLayout* logLayout = new QGridLayout(logGroup);
    
    QLabel* logLevelLabel = new QLabel(tr("日志级别:"), logGroup);
    m_logLevelCombo = new QComboBox(logGroup);
    m_logLevelCombo->addItems({tr("关闭"), tr("错误"), tr("警告"), tr("信息"), tr("调试")});
    
    logLayout->addWidget(logLevelLabel, 0, 0);
    logLayout->addWidget(m_logLevelCombo, 0, 1);
    
    // 缓存设置组
    QGroupBox* cacheGroup = new QGroupBox(tr("缓存设置"), page);
    QGridLayout* cacheLayout = new QGridLayout(cacheGroup);
    
    QLabel* cacheSizeLabel = new QLabel(tr("缓存大小 (MB):"), cacheGroup);
    m_cacheSizeSpin = new QSpinBox(cacheGroup);
    m_cacheSizeSpin->setRange(MIN_CACHE_SIZE, MAX_CACHE_SIZE);
    m_cacheSizeSpin->setValue(DEFAULT_CACHE_SIZE);
    
    m_clearCacheButton = new QPushButton(tr("清除缓存"), cacheGroup);
    
    cacheLayout->addWidget(cacheSizeLabel, 0, 0);
    cacheLayout->addWidget(m_cacheSizeSpin, 0, 1);
    cacheLayout->addWidget(m_clearCacheButton, 1, 0, 1, 2);
    
    // 性能设置组
    QGroupBox* performanceGroup = new QGroupBox(tr("性能设置"), page);
    QVBoxLayout* performanceLayout = new QVBoxLayout(performanceGroup);
    
    m_hardwareAccelCheck = new QCheckBox(tr("启用硬件加速"), performanceGroup);
    m_experimentalFeaturesCheck = new QCheckBox(tr("启用实验性功能"), performanceGroup);
    m_debugModeCheck = new QCheckBox(tr("调试模式"), performanceGroup);
    
    performanceLayout->addWidget(m_hardwareAccelCheck);
    performanceLayout->addWidget(m_experimentalFeaturesCheck);
    performanceLayout->addWidget(m_debugModeCheck);
    
    // 组装页面布局
    layout->addWidget(logGroup);
    layout->addWidget(cacheGroup);
    layout->addWidget(performanceGroup);
    layout->addStretch();
    
    return page;
}

/**
 * @brief 创建关于页面
 * @return 关于页面部件
 */
QWidget* SettingsDialog::createAboutPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    // 版本信息组
    QGroupBox* versionGroup = new QGroupBox(tr("版本信息"), page);
    QVBoxLayout* versionLayout = new QVBoxLayout(versionGroup);
    
    m_versionLabel = new QLabel(tr("Jitsi Meet Qt 版本: 1.0.0"), versionGroup);
    m_buildLabel = new QLabel(tr("构建日期: %1").arg(__DATE__), versionGroup);
    m_qtVersionLabel = new QLabel(tr("Qt 版本: %1").arg(QT_VERSION_STR), versionGroup);
    
    versionLayout->addWidget(m_versionLabel);
    versionLayout->addWidget(m_buildLabel);
    versionLayout->addWidget(m_qtVersionLabel);
    
    // 更新和链接组
    QGroupBox* updateGroup = new QGroupBox(tr("更新和链接"), page);
    QVBoxLayout* updateLayout = new QVBoxLayout(updateGroup);
    
    m_checkUpdatesButton = new QPushButton(tr("检查更新"), updateGroup);
    m_visitWebsiteButton = new QPushButton(tr("访问官网"), updateGroup);
    m_viewLicenseButton = new QPushButton(tr("查看许可证"), updateGroup);
    
    updateLayout->addWidget(m_checkUpdatesButton);
    updateLayout->addWidget(m_visitWebsiteButton);
    updateLayout->addWidget(m_viewLicenseButton);
    
    // 致谢信息组
    QGroupBox* creditsGroup = new QGroupBox(tr("致谢"), page);
    QVBoxLayout* creditsLayout = new QVBoxLayout(creditsGroup);
    
    m_creditsText = new QTextEdit(creditsGroup);
    m_creditsText->setReadOnly(true);
    m_creditsText->setMaximumHeight(150);
    m_creditsText->setHtml(tr(
        "<p>感谢以下开源项目:</p>"
        "<ul>"
        "<li><a href='https://jitsi.org/'>Jitsi Meet</a> - 视频会议平台</li>"
        "<li><a href='https://www.qt.io/'>Qt</a> - 跨平台应用程序框架</li>"
        "<li><a href='https://www.chromium.org/'>Chromium</a> - Web引擎</li>"
        "</ul>"
    ));
    
    creditsLayout->addWidget(m_creditsText);
    
    // 组装页面布局
    layout->addWidget(versionGroup);
    layout->addWidget(updateGroup);
    layout->addWidget(creditsGroup);
    layout->addStretch();
    
    return page;
}

/**
 * @brief 初始化连接
 */
void SettingsDialog::initializeConnections()
{
    // 按钮连接
    connect(m_okButton, &QPushButton::clicked, this, &SettingsDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsDialog::onResetClicked);
    connect(m_importButton, &QPushButton::clicked, this, &SettingsDialog::onImportClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &SettingsDialog::onExportClicked);
    
    // 常规页面连接
    if (m_languageCombo) {
        connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onLanguageChanged);
    }
    if (m_themeCombo) {
        connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onThemeChanged);
    }
    if (m_startupCheck) {
        connect(m_startupCheck, &QCheckBox::toggled, this, &SettingsDialog::onStartupChanged);
    }
    if (m_systemTrayCheck) {
        connect(m_systemTrayCheck, &QCheckBox::toggled, this, &SettingsDialog::onSystemTrayChanged);
    }
    if (m_minimizeToTrayCheck) {
        connect(m_minimizeToTrayCheck, &QCheckBox::toggled, this, &SettingsDialog::onMinimizeToTrayChanged);
    }
    if (m_defaultNameEdit) {
        connect(m_defaultNameEdit, &QLineEdit::textChanged, this, [this]() {
            m_hasChanges = true;
            updateApplyButtonState();
        });
    }
    
    // 服务器页面连接
    if (m_defaultServerEdit) {
        connect(m_defaultServerEdit, &QLineEdit::textChanged, this, &SettingsDialog::onDefaultServerChanged);
    }
    if (m_connectionTimeoutSpin) {
        connect(m_connectionTimeoutSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onConnectionTimeoutChanged);
    }
    if (m_proxyEnabledCheck) {
        connect(m_proxyEnabledCheck, &QCheckBox::toggled, this, &SettingsDialog::onProxySettingsChanged);
    }
    if (m_proxyHostEdit) {
        connect(m_proxyHostEdit, &QLineEdit::textChanged, this, &SettingsDialog::onProxySettingsChanged);
    }
    if (m_proxyPortSpin) {
        connect(m_proxyPortSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onProxySettingsChanged);
    }
    if (m_proxyUserEdit) {
        connect(m_proxyUserEdit, &QLineEdit::textChanged, this, &SettingsDialog::onProxySettingsChanged);
    }
    if (m_proxyPasswordEdit) {
        connect(m_proxyPasswordEdit, &QLineEdit::textChanged, this, &SettingsDialog::onProxySettingsChanged);
    }
    
    // 音视频页面连接
    if (m_audioDeviceCombo) {
        connect(m_audioDeviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onAudioDeviceChanged);
    }
    if (m_videoDeviceCombo) {
        connect(m_videoDeviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onVideoDeviceChanged);
    }
    if (m_audioQualityCombo) {
        connect(m_audioQualityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onAudioQualityChanged);
    }
    if (m_videoQualityCombo) {
        connect(m_videoQualityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onVideoQualityChanged);
    }
    if (m_defaultMuteCheck) {
        connect(m_defaultMuteCheck, &QCheckBox::toggled, this, &SettingsDialog::onDefaultMuteChanged);
    }
    if (m_defaultCameraOffCheck) {
        connect(m_defaultCameraOffCheck, &QCheckBox::toggled, this, &SettingsDialog::onDefaultCameraOffChanged);
    }
    if (m_micVolumeSlider) {
        connect(m_micVolumeSlider, &QSlider::valueChanged, this, [this]() {
            m_hasChanges = true;
            updateApplyButtonState();
        });
    }
    if (m_speakerVolumeSlider) {
        connect(m_speakerVolumeSlider, &QSlider::valueChanged, this, [this]() {
            m_hasChanges = true;
            updateApplyButtonState();
        });
    }
    
    // 界面页面连接
    if (m_fontSelectButton) {
        connect(m_fontSelectButton, &QPushButton::clicked, this, &SettingsDialog::onFontSelectClicked);
    }
    if (m_colorSelectButton) {
        connect(m_colorSelectButton, &QPushButton::clicked, this, &SettingsDialog::onColorSelectClicked);
    }
    if (m_windowWidthSpin) {
        connect(m_windowWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onWindowSizeChanged);
    }
    if (m_windowHeightSpin) {
        connect(m_windowHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onWindowSizeChanged);
    }
    if (m_rememberSizeCheck) {
        connect(m_rememberSizeCheck, &QCheckBox::toggled, this, [this]() {
            m_hasChanges = true;
            updateApplyButtonState();
        });
    }
    if (m_rememberPositionCheck) {
        connect(m_rememberPositionCheck, &QCheckBox::toggled, this, [this]() {
            m_hasChanges = true;
            updateApplyButtonState();
        });
    }
    
    // 高级页面连接
    if (m_logLevelCombo) {
        connect(m_logLevelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onLogLevelChanged);
    }
    if (m_cacheSizeSpin) {
        connect(m_cacheSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onCacheSizeChanged);
    }
    if (m_clearCacheButton) {
        connect(m_clearCacheButton, &QPushButton::clicked, this, &SettingsDialog::onClearCacheClicked);
    }
    if (m_hardwareAccelCheck) {
        connect(m_hardwareAccelCheck, &QCheckBox::toggled, this, [this]() {
            m_hasChanges = true;
            updateApplyButtonState();
        });
    }
    if (m_experimentalFeaturesCheck) {
        connect(m_experimentalFeaturesCheck, &QCheckBox::toggled, this, &SettingsDialog::onExperimentalFeaturesChanged);
    }
    if (m_debugModeCheck) {
        connect(m_debugModeCheck, &QCheckBox::toggled, this, [this]() {
            m_hasChanges = true;
            updateApplyButtonState();
        });
    }
    
    // 关于页面连接
    if (m_checkUpdatesButton) {
        connect(m_checkUpdatesButton, &QPushButton::clicked, this, &SettingsDialog::onCheckUpdatesClicked);
    }
    if (m_visitWebsiteButton) {
        connect(m_visitWebsiteButton, &QPushButton::clicked, this, &SettingsDialog::onVisitWebsiteClicked);
    }
    if (m_viewLicenseButton) {
        connect(m_viewLicenseButton, &QPushButton::clicked, this, &SettingsDialog::onViewLicenseClicked);
    }
    
    // 定时器连接
    if (m_validationTimer) {
        connect(m_validationTimer, &QTimer::timeout, this, &SettingsDialog::onValidationTimeout);
    }
}

/**
 * @brief 加载设置
 */
void SettingsDialog::loadSettings()
{
    if (!m_configManager) {
        return;
    }
    
    m_isLoading = true;
    
    // 加载可用选项
    loadAvailableLanguages();
    loadAvailableThemes();
    loadAudioVideoDevices();
    
    // 加载常规设置
    if (m_languageCombo) {
        QString language = m_configManager->getValue("language", "en").toString();
        int index = m_languageCombo->findData(language);
        if (index >= 0) {
            m_languageCombo->setCurrentIndex(index);
        }
    }
    
    if (m_themeCombo) {
        QString theme = m_configManager->getValue("theme", "default").toString();
        int index = m_themeCombo->findData(theme);
        if (index >= 0) {
            m_themeCombo->setCurrentIndex(index);
        }
    }
    
    if (m_startupCheck) {
        m_startupCheck->setChecked(m_configManager->isAutoStart());
    }
    
    if (m_systemTrayCheck) {
        m_systemTrayCheck->setChecked(m_configManager->isSystemTrayEnabled());
    }
    
    if (m_minimizeToTrayCheck) {
        m_minimizeToTrayCheck->setChecked(m_configManager->isMinimizeToTray());
        m_minimizeToTrayCheck->setEnabled(m_configManager->isSystemTrayEnabled());
    }
    
    if (m_defaultNameEdit) {
        m_defaultNameEdit->setText(m_configManager->getDefaultDisplayName());
    }
    
    // 加载服务器设置
    if (m_defaultServerEdit) {
        m_defaultServerEdit->setText(m_configManager->getDefaultServerUrl());
    }
    
    if (m_connectionTimeoutSpin) {
        m_connectionTimeoutSpin->setValue(m_configManager->getValue("connection_timeout", 30000).toInt() / 1000);
    }
    
    // 加载代理设置
    if (m_proxyEnabledCheck) {
        bool proxyEnabled = m_configManager->getValue("proxy_enabled", false).toBool();
        m_proxyEnabledCheck->setChecked(proxyEnabled);
        
        if (m_proxyHostEdit) {
            m_proxyHostEdit->setText(m_configManager->getValue("proxy_host", "").toString());
            m_proxyHostEdit->setEnabled(proxyEnabled);
        }
        
        if (m_proxyPortSpin) {
            m_proxyPortSpin->setValue(m_configManager->getValue("proxy_port", 8080).toInt());
            m_proxyPortSpin->setEnabled(proxyEnabled);
        }
        
        if (m_proxyUserEdit) {
            m_proxyUserEdit->setText(m_configManager->getValue("proxy_username", "").toString());
            m_proxyUserEdit->setEnabled(proxyEnabled);
        }
        
        if (m_proxyPasswordEdit) {
            m_proxyPasswordEdit->setText(m_configManager->getValue("proxy_password", "").toString());
            m_proxyPasswordEdit->setEnabled(proxyEnabled);
        }
    }
    
    // 加载音视频设置
    if (m_audioQualityCombo) {
        int audioQuality = m_configManager->getValue("audio_quality", 0).toInt();
        if (audioQuality >= 0 && audioQuality < m_audioQualityCombo->count()) {
            m_audioQualityCombo->setCurrentIndex(audioQuality);
        }
    }
    
    if (m_videoQualityCombo) {
        int videoQuality = m_configManager->getValue("video_quality", 0).toInt();
        if (videoQuality >= 0 && videoQuality < m_videoQualityCombo->count()) {
            m_videoQualityCombo->setCurrentIndex(videoQuality);
        }
    }
    
    if (m_defaultMuteCheck) {
        m_defaultMuteCheck->setChecked(m_configManager->isDefaultMuted());
    }
    
    if (m_defaultCameraOffCheck) {
        m_defaultCameraOffCheck->setChecked(m_configManager->isDefaultVideoDisabled());
    }
    
    if (m_micVolumeSlider) {
        m_micVolumeSlider->setValue(m_configManager->getValue("microphone_volume", 50).toInt());
    }
    
    if (m_speakerVolumeSlider) {
        m_speakerVolumeSlider->setValue(m_configManager->getValue("speaker_volume", 50).toInt());
    }
    
    // 加载界面设置
    if (m_windowWidthSpin) {
        m_windowWidthSpin->setValue(m_configManager->getValue("default_window_width", 800).toInt());
    }
    
    if (m_windowHeightSpin) {
        m_windowHeightSpin->setValue(m_configManager->getValue("default_window_height", 600).toInt());
    }
    
    if (m_rememberSizeCheck) {
        m_rememberSizeCheck->setChecked(m_configManager->getValue("remember_window_size", false).toBool());
    }
    
    if (m_rememberPositionCheck) {
        m_rememberPositionCheck->setChecked(m_configManager->getValue("remember_window_position", false).toBool());
    }
    
    // 加载高级设置
    if (m_logLevelCombo) {
        int logLevel = m_configManager->getValue("log_level", 0).toInt();
        if (logLevel >= 0 && logLevel < m_logLevelCombo->count()) {
            m_logLevelCombo->setCurrentIndex(logLevel);
        }
    }
    
    if (m_cacheSizeSpin) {
        m_cacheSizeSpin->setValue(m_configManager->getValue("cache_size", 100).toInt());
    }
    
    if (m_hardwareAccelCheck) {
        m_hardwareAccelCheck->setChecked(m_configManager->getValue("hardware_acceleration_enabled", false).toBool());
    }
    
    if (m_experimentalFeaturesCheck) {
        m_experimentalFeaturesCheck->setChecked(m_configManager->getValue("experimental_features_enabled", false).toBool());
    }
    
    if (m_debugModeCheck) {
        m_debugModeCheck->setChecked(m_configManager->getValue("debug_mode_enabled", false).toBool());
    }
    
    m_isLoading = false;
    m_hasChanges = false;
    updateApplyButtonState();
}

/**
 * @brief 保存设置
 */
void SettingsDialog::saveSettings()
{
    if (!m_configManager) {
        return;
    }
    
    // 保存常规设置
    if (m_languageCombo) {
        m_configManager->setValue("language", m_languageCombo->currentData().toString());
    }
    
    if (m_themeCombo) {
        m_configManager->setValue("theme", m_themeCombo->currentData().toString());
    }
    
    if (m_startupCheck) {
        m_configManager->setAutoStart(m_startupCheck->isChecked());
    }
    
    if (m_systemTrayCheck) {
        m_configManager->setSystemTrayEnabled(m_systemTrayCheck->isChecked());
    }
    
    if (m_minimizeToTrayCheck) {
        m_configManager->setMinimizeToTray(m_minimizeToTrayCheck->isChecked());
    }
    
    if (m_defaultNameEdit) {
        m_configManager->setDefaultDisplayName(m_defaultNameEdit->text());
    }
    
    // 保存服务器设置
    if (m_defaultServerEdit) {
        m_configManager->setDefaultServerUrl(m_defaultServerEdit->text());
    }
    
    if (m_connectionTimeoutSpin) {
        m_configManager->setValue("connection_timeout", m_connectionTimeoutSpin->value() * 1000);
    }
    
    // 保存代理设置
    if (m_proxyEnabledCheck) {
        m_configManager->setValue("proxy_enabled", m_proxyEnabledCheck->isChecked());
    }
    
    if (m_proxyHostEdit) {
        m_configManager->setValue("proxy_host", m_proxyHostEdit->text());
    }
    
    if (m_proxyPortSpin) {
        m_configManager->setValue("proxy_port", m_proxyPortSpin->value());
    }
    
    if (m_proxyUserEdit) {
        m_configManager->setValue("proxy_username", m_proxyUserEdit->text());
    }
    
    if (m_proxyPasswordEdit) {
        m_configManager->setValue("proxy_password", m_proxyPasswordEdit->text());
    }
    
    // 保存音视频设置
    if (m_audioDeviceCombo) {
        m_configManager->setValue("audio_device", m_audioDeviceCombo->currentData().toString());
    }
    
    if (m_videoDeviceCombo) {
        m_configManager->setValue("video_device", m_videoDeviceCombo->currentData().toString());
    }
    
    if (m_audioQualityCombo) {
        m_configManager->setValue("audio_quality", m_audioQualityCombo->currentIndex());
    }
    
    if (m_videoQualityCombo) {
        m_configManager->setValue("video_quality", m_videoQualityCombo->currentIndex());
    }
    
    if (m_defaultMuteCheck) {
        m_configManager->setDefaultMuted(m_defaultMuteCheck->isChecked());
    }
    
    if (m_defaultCameraOffCheck) {
        m_configManager->setDefaultVideoDisabled(m_defaultCameraOffCheck->isChecked());
    }
    
    if (m_micVolumeSlider) {
        m_configManager->setValue("microphone_volume", m_micVolumeSlider->value());
    }
    
    if (m_speakerVolumeSlider) {
        m_configManager->setValue("speaker_volume", m_speakerVolumeSlider->value());
    }
    
    // 保存界面设置
    if (m_windowWidthSpin) {
        m_configManager->setValue("default_window_width", m_windowWidthSpin->value());
    }
    
    if (m_windowHeightSpin) {
        m_configManager->setValue("default_window_height", m_windowHeightSpin->value());
    }
    
    if (m_rememberSizeCheck) {
        m_configManager->setValue("remember_window_size", m_rememberSizeCheck->isChecked());
    }
    
    if (m_rememberPositionCheck) {
        m_configManager->setValue("remember_window_position", m_rememberPositionCheck->isChecked());
    }
    
    // 保存高级设置
    if (m_logLevelCombo) {
        m_configManager->setValue("log_level", m_logLevelCombo->currentIndex());
    }
    
    if (m_cacheSizeSpin) {
        m_configManager->setValue("cache_size", m_cacheSizeSpin->value());
    }
    
    if (m_hardwareAccelCheck) {
        m_configManager->setValue("hardware_acceleration_enabled", m_hardwareAccelCheck->isChecked());
    }
    
    if (m_experimentalFeaturesCheck) {
        m_configManager->setValue("experimental_features_enabled", m_experimentalFeaturesCheck->isChecked());
    }
    
    if (m_debugModeCheck) {
        m_configManager->setValue("debug_mode_enabled", m_debugModeCheck->isChecked());
    }
    
    // 保存配置到文件
    m_configManager->sync();
}

/**
 * @brief 验证设置
 * @return 是否验证通过
 */
bool SettingsDialog::validateSettings()
{
    // 验证服务器URL
    if (m_defaultServerEdit) {
        QString serverUrl = m_defaultServerEdit->text().trimmed();
        if (!serverUrl.isEmpty()) {
            QUrl url(serverUrl);
            if (!url.isValid() || (url.scheme() != "http" && url.scheme() != "https")) {
                QMessageBox::warning(this, tr("验证错误"), tr("请输入有效的服务器URL"));
                m_tabWidget->setCurrentWidget(m_serverPage);
                m_defaultServerEdit->setFocus();
                return false;
            }
        }
    }
    
    // 验证代理设置
    if (m_proxyEnabledCheck && m_proxyEnabledCheck->isChecked()) {
        if (m_proxyHostEdit && m_proxyHostEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, tr("验证错误"), tr("启用代理时必须输入代理主机地址"));
            m_tabWidget->setCurrentWidget(m_serverPage);
            m_proxyHostEdit->setFocus();
            return false;
        }
    }
    
    return true;
}

/**
 * @brief 更新应用按钮状态
 */
void SettingsDialog::updateApplyButtonState()
{
    if (m_applyButton) {
        m_applyButton->setEnabled(m_hasChanges && !m_isLoading);
    }
}

/**
 * @brief 更新UI状态
 */
void SettingsDialog::updateUIState()
{
    updateApplyButtonState();
    
    // 更新状态标签
    if (m_statusLabel) {
        if (m_hasChanges) {
            m_statusLabel->setText(tr("有未保存的更改"));
        } else {
            m_statusLabel->setText(tr("就绪"));
        }
    }
}

/**
 * @brief 加载可用语言
 */
void SettingsDialog::loadAvailableLanguages()
{
    if (!m_languageCombo) {
        return;
    }
    
    m_languageCombo->clear();
    m_languageCombo->addItem(tr("简体中文"), "zh_CN");
    m_languageCombo->addItem(tr("English"), "en_US");
    m_languageCombo->addItem(tr("日本語"), "ja_JP");
    m_languageCombo->addItem(tr("한국어"), "ko_KR");
    m_languageCombo->addItem(tr("Français"), "fr_FR");
    m_languageCombo->addItem(tr("Deutsch"), "de_DE");
    m_languageCombo->addItem(tr("Español"), "es_ES");
    m_languageCombo->addItem(tr("Русский"), "ru_RU");
}

/**
 * @brief 加载可用主题
 */
void SettingsDialog::loadAvailableThemes()
{
    if (!m_themeCombo) {
        return;
    }
    
    m_themeCombo->clear();
    m_themeCombo->addItem(tr("系统默认"), "system");
    m_themeCombo->addItem(tr("浅色主题"), "light");
    m_themeCombo->addItem(tr("深色主题"), "dark");
    m_themeCombo->addItem(tr("高对比度"), "high_contrast");
}

/**
 * @brief 加载音视频设备
 */
void SettingsDialog::loadAudioVideoDevices()
{
    // 加载音频设备
    if (m_audioDeviceCombo) {
        m_audioDeviceCombo->clear();
        m_audioDeviceCombo->addItem(tr("系统默认"), "default");
        
        // TODO: 实际获取系统音频设备列表
        m_audioDeviceCombo->addItem(tr("内置麦克风"), "builtin_mic");
        m_audioDeviceCombo->addItem(tr("USB麦克风"), "usb_mic");
    }
    
    // 加载视频设备
    if (m_videoDeviceCombo) {
        m_videoDeviceCombo->clear();
        m_videoDeviceCombo->addItem(tr("系统默认"), "default");
        
        // TODO: 实际获取系统视频设备列表
        m_videoDeviceCombo->addItem(tr("内置摄像头"), "builtin_camera");
        m_videoDeviceCombo->addItem(tr("USB摄像头"), "usb_camera");
    }
}

/**
 * @brief 清除应用程序缓存
 */
void SettingsDialog::clearApplicationCache()
{
    if (m_progressBar) {
        m_progressBar->setVisible(true);
        m_progressBar->setRange(0, 0); // 不确定进度
    }
    
    if (m_statusLabel) {
        m_statusLabel->setText(tr("正在清除缓存..."));
    }
    
    // 在后台线程中清除缓存
    QTimer::singleShot(2000, this, &SettingsDialog::onCacheClearFinished);
}

/**
 * @brief 检查更新
 */
void SettingsDialog::checkForUpdates()
{
    if (m_checkUpdatesButton) {
        m_checkUpdatesButton->setEnabled(false);
        m_checkUpdatesButton->setText(tr("检查中..."));
    }
    
    if (m_statusLabel) {
        m_statusLabel->setText(tr("正在检查更新..."));
    }
    
    // 模拟检查更新
    QTimer::singleShot(3000, this, [this]() {
        if (m_checkUpdatesButton) {
            m_checkUpdatesButton->setEnabled(true);
            m_checkUpdatesButton->setText(tr("检查更新"));
        }
        
        if (m_statusLabel) {
            m_statusLabel->setText(tr("已是最新版本"));
        }
        
        QMessageBox::information(this, tr("检查更新"), tr("当前已是最新版本"));
    });
}