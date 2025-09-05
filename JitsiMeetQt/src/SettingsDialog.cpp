#include "SettingsDialog.h"
#include "ConfigurationManager.h"

#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include <QFont>
#include <QStyleOption>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

/**
 * @brief SettingsDialog构造函数
 * @param parent 父窗口
 */
SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_titleBar(nullptr)
    , m_titleLabel(nullptr)
    , m_closeButton(nullptr)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_contentLayout(nullptr)
    , m_serverFrame(nullptr)
    , m_serverUrlLabel(nullptr)
    , m_serverUrlEdit(nullptr)
    , m_serverTimeoutLabel(nullptr)
    , m_serverTimeoutSpin(nullptr)
    , m_switchFrame(nullptr)
    , m_alwaysOnTopCheck(nullptr)
    , m_disableAGCCheck(nullptr)
    , m_buttonArea(nullptr)
    , m_buttonLayout(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_configManager(nullptr)
{
    // 获取配置管理器实例
    m_configManager = ConfigurationManager::instance();
    
    // 初始化UI
    initializeUI();
    initializeConnections();
    
    // 加载设置
    loadSettings();
    
    // 设置窗口属性
    setWindowTitle(tr("设置"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);
    resize(400, 600);
    
    // 设置样式
    setStyleSheet(
        "QDialog {"
        "    background-color: #1e2328;"
        "    border-radius: 8px;"
        "}"
        "QLabel {"
        "    color: #ffffff;"
        "    font-size: 14px;"
        "}"
        "QLineEdit {"
        "    background-color: #2a2d32;"
        "    border: 1px solid #3a3d42;"
        "    border-radius: 4px;"
        "    padding: 8px;"
        "    color: #ffffff;"
        "    font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #4a9eff;"
        "}"
        "QSpinBox {"
        "    background-color: #2a2d32;"
        "    border: 1px solid #3a3d42;"
        "    border-radius: 4px;"
        "    padding: 8px;"
        "    color: #ffffff;"
        "    font-size: 14px;"
        "}"
        "QSpinBox:focus {"
        "    border-color: #4a9eff;"
        "}"
        "QCheckBox {"
        "    color: #ffffff;"
        "    font-size: 14px;"
        "    spacing: 8px;"
        "}"
        "QCheckBox::indicator {"
        "    width: 20px;"
        "    height: 20px;"
        "    border-radius: 10px;"
        "    background-color: #3a3d42;"
        "    border: 2px solid #5a5d62;"
        "}"
        "QCheckBox::indicator:checked {"
        "    background-color: #4a9eff;"
        "    border-color: #4a9eff;"
        "}"
        "QCheckBox::indicator:checked:before {"
        "    content: '';"
        "    width: 8px;"
        "    height: 8px;"
        "    border-radius: 4px;"
        "    background-color: #ffffff;"
        "    margin: 4px;"
        "}"
        "QPushButton {"
        "    background-color: #4a9eff;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    color: #ffffff;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #5aa9ff;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3a89df;"
        "}"
        "QPushButton#cancelButton {"
        "    background-color: #3a3d42;"
        "    color: #ffffff;"
        "}"
        "QPushButton#cancelButton:hover {"
        "    background-color: #4a4d52;"
        "}"
        "QPushButton#closeButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    color: #ffffff;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    padding: 5px;"
        "    border-radius: 3px;"
        "}"
        "QPushButton#closeButton:hover {"
        "    background-color: #ff4444;"
        "}"
        "QFrame {"
        "    background-color: transparent;"
        "    border: none;"
        "}"
        "QScrollArea {"
        "    background-color: transparent;"
        "    border: none;"
        "}"
        "QScrollBar:vertical {"
        "    background-color: #2a2d32;"
        "    width: 8px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background-color: #4a4d52;"
        "    border-radius: 4px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background-color: #5a5d62;"
        "}"
    );
}

/**
 * @brief SettingsDialog析构函数
 */
SettingsDialog::~SettingsDialog()
{
    // 析构函数
}

/**
 * @brief 确定按钮点击处理
 */
void SettingsDialog::onOkClicked()
{
    saveSettings();
    emit settingsApplied();
    accept();
}

/**
 * @brief 取消按钮点击处理
 */
void SettingsDialog::onCancelClicked()
{
    reject();
}

/**
 * @brief 服务器URL改变处理
 * @param text 服务器URL
 */
void SettingsDialog::onServerUrlChanged(const QString& text)
{
    Q_UNUSED(text)
    // 处理服务器URL改变
}

/**
 * @brief 服务器超时改变处理
 * @param value 超时值
 */
void SettingsDialog::onServerTimeoutChanged(int value)
{
    Q_UNUSED(value)
    // 处理服务器超时改变
}

/**
 * @brief 始终置顶选项改变处理
 * @param checked 是否选中
 */
void SettingsDialog::onAlwaysOnTopChanged(bool checked)
{
    Q_UNUSED(checked)
    // 处理始终置顶选项改变
}

/**
 * @brief 禁用自动增益控制选项改变处理
 * @param checked 是否选中
 */
void SettingsDialog::onDisableAGCChanged(bool checked)
{
    Q_UNUSED(checked)
    // 处理禁用自动增益控制选项改变
}

/**
 * @brief 初始化用户界面
 */
void SettingsDialog::initializeUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // 创建标题栏
    m_titleBar = createTitleBar();
    m_mainLayout->addWidget(m_titleBar);
    
    // 创建内容区域
    QWidget* contentArea = createContentArea();
    m_mainLayout->addWidget(contentArea, 1);
    
    // 创建按钮区域
    m_buttonArea = createButtonArea();
    m_mainLayout->addWidget(m_buttonArea);
}

/**
 * @brief 初始化连接
 */
void SettingsDialog::initializeConnections()
{
    // 连接按钮信号
    connect(m_closeButton, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    connect(m_okButton, &QPushButton::clicked, this, &SettingsDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    
    // 连接设置项信号
    connect(m_serverUrlEdit, &QLineEdit::textChanged, this, &SettingsDialog::onServerUrlChanged);
    connect(m_serverTimeoutSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onServerTimeoutChanged);
    connect(m_alwaysOnTopCheck, &QCheckBox::toggled, this, &SettingsDialog::onAlwaysOnTopChanged);
    connect(m_disableAGCCheck, &QCheckBox::toggled, this, &SettingsDialog::onDisableAGCChanged);
}

/**
 * @brief 加载设置
 */
void SettingsDialog::loadSettings()
{
    if (!m_configManager) {
        return;
    }
    
    // 加载服务器设置
    m_serverUrlEdit->setText(m_configManager->getDefaultServerUrl());
    m_serverTimeoutSpin->setValue(m_configManager->getServerTimeout() / 1000); // 转换为秒
    
    // 加载开关设置
    m_alwaysOnTopCheck->setChecked(m_configManager->getValue("always_on_top", false).toBool());
    m_disableAGCCheck->setChecked(m_configManager->getValue("disable_agc", false).toBool());
}

/**
 * @brief 保存设置
 */
void SettingsDialog::saveSettings()
{
    if (!m_configManager) {
        return;
    }
    
    // 保存服务器设置
    m_configManager->setDefaultServerUrl(m_serverUrlEdit->text());
    m_configManager->setServerTimeout(m_serverTimeoutSpin->value() * 1000); // 转换为毫秒
    
    // 保存开关设置
    m_configManager->setValue("always_on_top", m_alwaysOnTopCheck->isChecked());
    m_configManager->setValue("disable_agc", m_disableAGCCheck->isChecked());
    
    // 保存配置
    m_configManager->sync();
}

/**
 * @brief 创建标题栏
 * @return 标题栏部件
 */
QWidget* SettingsDialog::createTitleBar()
{
    QWidget* titleBar = new QWidget(this);
    titleBar->setFixedHeight(50);
    titleBar->setStyleSheet("background-color: #1e2328; border-bottom: 1px solid #3a3d42;");
    
    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 20, 0);
    
    // 标题标签
    m_titleLabel = new QLabel(tr("设置"), titleBar);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #ffffff;");
    titleLayout->addWidget(m_titleLabel);
    
    titleLayout->addStretch();
    
    // 关闭按钮
    m_closeButton = new QPushButton("×", titleBar);
    m_closeButton->setObjectName("closeButton");
    m_closeButton->setFixedSize(30, 30);
    titleLayout->addWidget(m_closeButton);
    
    return titleBar;
}

/**
 * @brief 创建内容区域
 * @return 内容区域部件
 */
QWidget* SettingsDialog::createContentArea()
{
    // 创建滚动区域
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // 创建内容部件
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(20, 20, 20, 20);
    m_contentLayout->setSpacing(20);
    
    // 创建服务器设置框架
    m_serverFrame = new QFrame(m_contentWidget);
    QVBoxLayout* serverLayout = new QVBoxLayout(m_serverFrame);
    serverLayout->setContentsMargins(0, 0, 0, 0);
    serverLayout->setSpacing(15);
    
    // 服务器网址
    m_serverUrlLabel = new QLabel(tr("服务器网址"), m_serverFrame);
    m_serverUrlLabel->setStyleSheet("color: #a0a0a0; font-size: 12px;");
    serverLayout->addWidget(m_serverUrlLabel);
    
    m_serverUrlEdit = new QLineEdit(m_serverFrame);
    m_serverUrlEdit->setPlaceholderText(tr("https://meet.jit.si"));
    serverLayout->addWidget(m_serverUrlEdit);
    
    // 服务器超时
    m_serverTimeoutLabel = new QLabel(tr("服务器超时 (秒)"), m_serverFrame);
    m_serverTimeoutLabel->setStyleSheet("color: #a0a0a0; font-size: 12px;");
    serverLayout->addWidget(m_serverTimeoutLabel);
    
    m_serverTimeoutSpin = new QSpinBox(m_serverFrame);
    m_serverTimeoutSpin->setRange(5, 300);
    m_serverTimeoutSpin->setValue(30);
    serverLayout->addWidget(m_serverTimeoutSpin);
    
    m_contentLayout->addWidget(m_serverFrame);
    
    // 创建开关设置框架
    m_switchFrame = new QFrame(m_contentWidget);
    QVBoxLayout* switchLayout = new QVBoxLayout(m_switchFrame);
    switchLayout->setContentsMargins(0, 0, 0, 0);
    switchLayout->setSpacing(20);
    
    // 始终置顶
    QWidget* alwaysOnTopWidget = new QWidget(m_switchFrame);
    QHBoxLayout* alwaysOnTopLayout = new QHBoxLayout(alwaysOnTopWidget);
    alwaysOnTopLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* alwaysOnTopLabel = new QLabel(tr("始终置顶"), alwaysOnTopWidget);
    alwaysOnTopLayout->addWidget(alwaysOnTopLabel);
    alwaysOnTopLayout->addStretch();
    
    m_alwaysOnTopCheck = new QCheckBox(alwaysOnTopWidget);
    alwaysOnTopLayout->addWidget(m_alwaysOnTopCheck);
    
    switchLayout->addWidget(alwaysOnTopWidget);
    
    // 禁用自动增益控制
    QWidget* disableAGCWidget = new QWidget(m_switchFrame);
    QHBoxLayout* disableAGCLayout = new QHBoxLayout(disableAGCWidget);
    disableAGCLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* disableAGCLabel = new QLabel(tr("禁用自动增益控制"), disableAGCWidget);
    disableAGCLayout->addWidget(disableAGCLabel);
    disableAGCLayout->addStretch();
    
    m_disableAGCCheck = new QCheckBox(disableAGCWidget);
    disableAGCLayout->addWidget(m_disableAGCCheck);
    
    switchLayout->addWidget(disableAGCWidget);
    
    m_contentLayout->addWidget(m_switchFrame);
    
    // 添加弹性空间
    m_contentLayout->addStretch();
    
    // 设置滚动区域的内容
    m_scrollArea->setWidget(m_contentWidget);
    
    return m_scrollArea;
}

/**
 * @brief 创建按钮区域
 * @return 按钮区域部件
 */
QWidget* SettingsDialog::createButtonArea()
{
    QWidget* buttonArea = new QWidget(this);
    buttonArea->setFixedHeight(80);
    buttonArea->setStyleSheet("background-color: #1e2328; border-top: 1px solid #3a3d42;");
    
    m_buttonLayout = new QHBoxLayout(buttonArea);
    m_buttonLayout->setContentsMargins(20, 20, 20, 20);
    m_buttonLayout->setSpacing(10);
    
    m_buttonLayout->addStretch();
    
    // 取消按钮
    m_cancelButton = new QPushButton(tr("取消"), buttonArea);
    m_cancelButton->setObjectName("cancelButton");
    m_cancelButton->setFixedSize(80, 36);
    m_buttonLayout->addWidget(m_cancelButton);
    
    // 确定按钮
    m_okButton = new QPushButton(tr("确定"), buttonArea);
    m_okButton->setFixedSize(80, 36);
    m_buttonLayout->addWidget(m_okButton);
    
    return buttonArea;
}