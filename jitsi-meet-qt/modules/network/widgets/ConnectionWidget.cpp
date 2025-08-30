#include "ConnectionWidget.h"
#include "../interfaces/INetworkManager.h"
#include "../config/NetworkConfig.h"
#include <QShowEvent>
#include <QHideEvent>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include <QStyle>
#include <QValidator>
#include <QRegularExpressionValidator>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QDebug>

class ConnectionWidget::Private
{
public:
    // 网络管理器和配置
    INetworkManager* networkManager;
    NetworkConfig* networkConfig;
    
    // UI设置
    bool readOnly;
    bool autoConnect;
    
    // UI组件
    QVBoxLayout* mainLayout;
    
    // 服务器配置组
    QGroupBox* serverConfigGroup;
    QFormLayout* serverConfigLayout;
    QLineEdit* serverUrlEdit;
    QSpinBox* serverPortSpinBox;
    QLineEdit* serverDomainEdit;
    
    // 连接选项组
    QGroupBox* connectionOptionsGroup;
    QFormLayout* connectionOptionsLayout;
    QSpinBox* connectionTimeoutSpinBox;
    QCheckBox* autoReconnectCheckBox;
    QSpinBox* reconnectIntervalSpinBox;
    QSpinBox* maxReconnectAttemptsSpinBox;
    QCheckBox* webRTCEnabledCheckBox;
    QCheckBox* webSocketEnabledCheckBox;
    QCheckBox* httpsOnlyCheckBox;
    
    // 控制按钮
    QWidget* controlButtonsWidget;
    QHBoxLayout* controlButtonsLayout;
    QPushButton* connectButton;
    QPushButton* disconnectButton;
    QPushButton* testButton;
    QPushButton* applyButton;
    QPushButton* resetButton;
    
    // 状态显示
    QWidget* statusWidget;
    QHBoxLayout* statusLayout;
    QLabel* statusLabel;
    QProgressBar* connectionProgressBar;
    
    // 预设管理
    QComboBox* presetComboBox;
    QPushButton* savePresetButton;
    QPushButton* deletePresetButton;
    
    // 连接测试
    QNetworkAccessManager* testNetworkManager;
    QTimer* testTimer;
    QTime testStartTime;
    
    // 当前状态
    int currentConnectionState;
    bool configurationChanged;
    
    Private() {
        networkManager = nullptr;
        networkConfig = nullptr;
        readOnly = false;
        autoConnect = false;
        
        mainLayout = nullptr;
        serverConfigGroup = nullptr;
        connectionOptionsGroup = nullptr;
        controlButtonsWidget = nullptr;
        statusWidget = nullptr;
        
        testNetworkManager = nullptr;
        testTimer = nullptr;
        
        currentConnectionState = 0; // Disconnected
        configurationChanged = false;
    }
};

ConnectionWidget::ConnectionWidget(QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    initializeUI();
    
    // 创建网络测试管理器
    d->testNetworkManager = new QNetworkAccessManager(this);
    
    // 创建测试超时定时器
    d->testTimer = new QTimer(this);
    d->testTimer->setSingleShot(true);
    d->testTimer->setInterval(10000); // 10秒超时
    connect(d->testTimer, &QTimer::timeout, this, [this]() {
        handleConnectionTestResult(false, -1);
    });
    
    // 应用样式
    applyStyles();
    setupTooltips();
    
    // 加载预设列表
    loadPresetList();
}

ConnectionWidget::~ConnectionWidget()
{
    delete d;
}

void ConnectionWidget::setNetworkManager(INetworkManager* manager)
{
    if (d->networkManager == manager) {
        return;
    }
    
    // 断开旧连接
    if (d->networkManager) {
        disconnect(d->networkManager, nullptr, this, nullptr);
    }
    
    d->networkManager = manager;
    
    // 建立新连接
    if (d->networkManager) {
        connect(d->networkManager, SIGNAL(connectionStateChanged(int)),
                this, SLOT(handleConnectionStateChanged(int)));
        
        // 更新UI状态
        updateUIState();
    }
}

INetworkManager* ConnectionWidget::networkManager() const
{
    return d->networkManager;
}

void ConnectionWidget::setNetworkConfig(NetworkConfig* config)
{
    if (d->networkConfig == config) {
        return;
    }
    
    // 断开旧连接
    if (d->networkConfig) {
        disconnect(d->networkConfig, nullptr, this, nullptr);
    }
    
    d->networkConfig = config;
    
    // 建立新连接
    if (d->networkConfig) {
        connect(d->networkConfig, &NetworkConfig::configurationChanged,
                this, &ConnectionWidget::handleConfigurationChanged);
        
        // 从配置更新UI
        updateUIFromConfig();
    }
}

NetworkConfig* ConnectionWidget::networkConfig() const
{
    return d->networkConfig;
}

void ConnectionWidget::setReadOnly(bool readOnly)
{
    if (d->readOnly == readOnly) {
        return;
    }
    
    d->readOnly = readOnly;
    updateUIState();
    emit readOnlyChanged(readOnly);
}

bool ConnectionWidget::isReadOnly() const
{
    return d->readOnly;
}

void ConnectionWidget::setAutoConnect(bool enabled)
{
    if (d->autoConnect == enabled) {
        return;
    }
    
    d->autoConnect = enabled;
    emit autoConnectChanged(enabled);
}

bool ConnectionWidget::autoConnect() const
{
    return d->autoConnect;
}

QString ConnectionWidget::serverUrl() const
{
    return d->serverUrlEdit ? d->serverUrlEdit->text() : QString();
}

void ConnectionWidget::setServerUrl(const QString& url)
{
    if (d->serverUrlEdit) {
        d->serverUrlEdit->setText(url);
    }
}

int ConnectionWidget::serverPort() const
{
    return d->serverPortSpinBox ? d->serverPortSpinBox->value() : 443;
}

void ConnectionWidget::setServerPort(int port)
{
    if (d->serverPortSpinBox) {
        d->serverPortSpinBox->setValue(port);
    }
}

void ConnectionWidget::connectToServer()
{
    if (!validateInput()) {
        return;
    }
    
    // 更新配置
    updateConfigFromUI();
    
    if (d->networkManager) {
        QString url = serverUrl();
        emit connectionRequested(url);
        
        // 更新UI状态
        d->connectionProgressBar->setVisible(true);
        d->connectionProgressBar->setRange(0, 0); // 不确定进度
        d->statusLabel->setText("Connecting...");
    }
}

void ConnectionWidget::disconnectFromServer()
{
    if (d->networkManager) {
        emit disconnectionRequested();
        
        // 更新UI状态
        d->connectionProgressBar->setVisible(false);
        d->statusLabel->setText("Disconnecting...");
    }
}

void ConnectionWidget::reconnect()
{
    disconnectFromServer();
    
    // 延迟重连
    QTimer::singleShot(1000, this, &ConnectionWidget::connectToServer);
}

void ConnectionWidget::testConnection()
{
    if (!validateInput()) {
        return;
    }
    
    QString url = serverUrl();
    if (url.isEmpty()) {
        return;
    }
    
    // 开始连接测试
    d->testButton->setEnabled(false);
    d->testButton->setText("Testing...");
    d->statusLabel->setText("Testing connection...");
    
    // 记录开始时间
    d->testStartTime = QTime::currentTime();
    
    // 发送测试请求
    QNetworkRequest request(QUrl(url));
    request.setRawHeader("User-Agent", "Jitsi-Meet-Qt Connection Test");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, 
                        QNetworkRequest::NoLessSafeRedirectPolicy);
    
    QNetworkReply* reply = d->testNetworkManager->head(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        int latency = d->testStartTime.msecsTo(QTime::currentTime());
        bool success = (reply->error() == QNetworkReply::NoError);
        
        reply->deleteLater();
        d->testTimer->stop();
        
        handleConnectionTestResult(success, success ? latency : -1);
    });
    
    // 启动超时定时器
    d->testTimer->start();
}

void ConnectionWidget::applyConfiguration()
{
    if (!validateInput()) {
        return;
    }
    
    updateConfigFromUI();
    
    if (d->networkConfig) {
        d->networkConfig->applyChanges();
    }
    
    d->configurationChanged = false;
    emit configurationChanged();
    
    d->statusLabel->setText("Configuration applied");
}

void ConnectionWidget::resetConfiguration()
{
    if (d->networkConfig) {
        d->networkConfig->resetToDefaults();
        updateUIFromConfig();
    }
    
    d->configurationChanged = false;
    d->statusLabel->setText("Configuration reset to defaults");
}

void ConnectionWidget::loadPreset(const QString& presetName)
{
    if (presetName.isEmpty()) {
        return;
    }
    
    QSettings settings;
    settings.beginGroup("ConnectionPresets");
    
    if (settings.contains(presetName)) {
        QVariantMap presetData = settings.value(presetName).toMap();
        
        if (d->networkConfig) {
            d->networkConfig->fromVariantMap(presetData);
            updateUIFromConfig();
        }
        
        d->statusLabel->setText(QString("Loaded preset: %1").arg(presetName));
    }
    
    settings.endGroup();
}

void ConnectionWidget::savePreset(const QString& presetName)
{
    if (presetName.isEmpty()) {
        return;
    }
    
    updateConfigFromUI();
    
    if (d->networkConfig) {
        QSettings settings;
        settings.beginGroup("ConnectionPresets");
        settings.setValue(presetName, d->networkConfig->toVariantMap());
        settings.endGroup();
        
        // 更新预设列表
        loadPresetList();
        
        d->statusLabel->setText(QString("Saved preset: %1").arg(presetName));
    }
}

void ConnectionWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    
    // 自动连接
    if (d->autoConnect && d->currentConnectionState == 0) {
        QTimer::singleShot(500, this, &ConnectionWidget::connectToServer);
    }
}

void ConnectionWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
}

void ConnectionWidget::handleConnectButtonClicked()
{
    connectToServer();
}

void ConnectionWidget::handleDisconnectButtonClicked()
{
    disconnectFromServer();
}

void ConnectionWidget::handleTestButtonClicked()
{
    testConnection();
}

void ConnectionWidget::handleServerUrlChanged()
{
    d->configurationChanged = true;
    updateUIState();
}

void ConnectionWidget::handleServerPortChanged()
{
    d->configurationChanged = true;
    updateUIState();
}

void ConnectionWidget::handleConnectionStateChanged(int state)
{
    d->currentConnectionState = state;
    updateConnectionButtons(state);
    
    switch (state) {
    case 0: // Disconnected
        d->connectionProgressBar->setVisible(false);
        d->statusLabel->setText("Disconnected");
        break;
    case 1: // Connecting
        d->connectionProgressBar->setVisible(true);
        d->connectionProgressBar->setRange(0, 0);
        d->statusLabel->setText("Connecting...");
        break;
    case 2: // Connected
        d->connectionProgressBar->setVisible(false);
        d->statusLabel->setText("Connected");
        break;
    case 3: // Error
        d->connectionProgressBar->setVisible(false);
        d->statusLabel->setText("Connection Error");
        break;
    }
}

void ConnectionWidget::handleConfigurationChanged()
{
    d->configurationChanged = true;
    updateUIState();
}

void ConnectionWidget::handlePresetChanged(const QString& presetName)
{
    if (!presetName.isEmpty() && presetName != "Custom") {
        loadPreset(presetName);
    }
}

void ConnectionWidget::handleConnectionTestResult(bool success, int latency)
{
    d->testButton->setEnabled(true);
    d->testButton->setText("Test");
    
    if (success) {
        d->statusLabel->setText(QString("Connection test successful (Latency: %1ms)").arg(latency));
    } else {
        d->statusLabel->setText("Connection test failed");
    }
    
    emit connectionTestCompleted(success, latency);
}

void ConnectionWidget::initializeUI()
{
    d->mainLayout = new QVBoxLayout(this);
    
    // 创建各个组件组
    d->serverConfigGroup = createServerConfigGroup();
    d->connectionOptionsGroup = createConnectionOptionsGroup();
    d->controlButtonsWidget = createControlButtonsGroup();
    d->statusWidget = createStatusGroup();
    
    // 添加到主布局
    d->mainLayout->addWidget(d->serverConfigGroup);
    d->mainLayout->addWidget(d->connectionOptionsGroup);
    d->mainLayout->addWidget(d->controlButtonsWidget);
    d->mainLayout->addWidget(d->statusWidget);
    d->mainLayout->addStretch();
    
    // 连接信号
    connect(d->serverUrlEdit, &QLineEdit::textChanged,
            this, &ConnectionWidget::handleServerUrlChanged);
    connect(d->serverPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ConnectionWidget::handleServerPortChanged);
    
    connect(d->connectButton, &QPushButton::clicked,
            this, &ConnectionWidget::handleConnectButtonClicked);
    connect(d->disconnectButton, &QPushButton::clicked,
            this, &ConnectionWidget::handleDisconnectButtonClicked);
    connect(d->testButton, &QPushButton::clicked,
            this, &ConnectionWidget::handleTestButtonClicked);
    connect(d->applyButton, &QPushButton::clicked,
            this, &ConnectionWidget::applyConfiguration);
    connect(d->resetButton, &QPushButton::clicked,
            this, &ConnectionWidget::resetConfiguration);
    
    connect(d->presetComboBox, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &ConnectionWidget::handlePresetChanged);
    
    updateUIState();
}

QGroupBox* ConnectionWidget::createServerConfigGroup()
{
    QGroupBox* group = new QGroupBox("Server Configuration");
    d->serverConfigLayout = new QFormLayout(group);
    
    // 服务器URL
    d->serverUrlEdit = new QLineEdit();
    d->serverUrlEdit->setPlaceholderText("https://meet.jit.si");
    QRegularExpressionValidator* urlValidator = new QRegularExpressionValidator(
        QRegularExpression("^https?://[\\w\\.-]+(:\\d+)?(/.*)?$"), this);
    d->serverUrlEdit->setValidator(urlValidator);
    
    // 服务器端口
    d->serverPortSpinBox = new QSpinBox();
    d->serverPortSpinBox->setRange(1, 65535);
    d->serverPortSpinBox->setValue(443);
    
    // 服务器域名
    d->serverDomainEdit = new QLineEdit();
    d->serverDomainEdit->setPlaceholderText("meet.jit.si");
    
    // 预设选择
    d->presetComboBox = new QComboBox();
    d->savePresetButton = new QPushButton("Save");
    d->deletePresetButton = new QPushButton("Delete");
    
    QHBoxLayout* presetLayout = new QHBoxLayout();
    presetLayout->addWidget(d->presetComboBox);
    presetLayout->addWidget(d->savePresetButton);
    presetLayout->addWidget(d->deletePresetButton);
    
    d->serverConfigLayout->addRow("Server URL:", d->serverUrlEdit);
    d->serverConfigLayout->addRow("Port:", d->serverPortSpinBox);
    d->serverConfigLayout->addRow("Domain:", d->serverDomainEdit);
    d->serverConfigLayout->addRow("Presets:", presetLayout);
    
    return group;
}

QGroupBox* ConnectionWidget::createConnectionOptionsGroup()
{
    QGroupBox* group = new QGroupBox("Connection Options");
    d->connectionOptionsLayout = new QFormLayout(group);
    
    // 连接超时
    d->connectionTimeoutSpinBox = new QSpinBox();
    d->connectionTimeoutSpinBox->setRange(5000, 120000);
    d->connectionTimeoutSpinBox->setValue(30000);
    d->connectionTimeoutSpinBox->setSuffix(" ms");
    
    // 自动重连
    d->autoReconnectCheckBox = new QCheckBox("Enable auto-reconnect");
    d->autoReconnectCheckBox->setChecked(true);
    
    // 重连间隔
    d->reconnectIntervalSpinBox = new QSpinBox();
    d->reconnectIntervalSpinBox->setRange(1000, 60000);
    d->reconnectIntervalSpinBox->setValue(5000);
    d->reconnectIntervalSpinBox->setSuffix(" ms");
    
    // 最大重连次数
    d->maxReconnectAttemptsSpinBox = new QSpinBox();
    d->maxReconnectAttemptsSpinBox->setRange(1, 10);
    d->maxReconnectAttemptsSpinBox->setValue(3);
    
    // 协议选项
    d->webRTCEnabledCheckBox = new QCheckBox("Enable WebRTC");
    d->webRTCEnabledCheckBox->setChecked(true);
    
    d->webSocketEnabledCheckBox = new QCheckBox("Enable WebSocket");
    d->webSocketEnabledCheckBox->setChecked(true);
    
    d->httpsOnlyCheckBox = new QCheckBox("HTTPS Only");
    d->httpsOnlyCheckBox->setChecked(true);
    
    d->connectionOptionsLayout->addRow("Connection Timeout:", d->connectionTimeoutSpinBox);
    d->connectionOptionsLayout->addRow("", d->autoReconnectCheckBox);
    d->connectionOptionsLayout->addRow("Reconnect Interval:", d->reconnectIntervalSpinBox);
    d->connectionOptionsLayout->addRow("Max Reconnect Attempts:", d->maxReconnectAttemptsSpinBox);
    d->connectionOptionsLayout->addRow("", d->webRTCEnabledCheckBox);
    d->connectionOptionsLayout->addRow("", d->webSocketEnabledCheckBox);
    d->connectionOptionsLayout->addRow("", d->httpsOnlyCheckBox);
    
    return group;
}

QWidget* ConnectionWidget::createControlButtonsGroup()
{
    QWidget* widget = new QWidget();
    d->controlButtonsLayout = new QHBoxLayout(widget);
    
    d->connectButton = new QPushButton("Connect");
    d->disconnectButton = new QPushButton("Disconnect");
    d->testButton = new QPushButton("Test");
    d->applyButton = new QPushButton("Apply");
    d->resetButton = new QPushButton("Reset");
    
    d->connectButton->setDefault(true);
    d->disconnectButton->setEnabled(false);
    
    d->controlButtonsLayout->addWidget(d->connectButton);
    d->controlButtonsLayout->addWidget(d->disconnectButton);
    d->controlButtonsLayout->addStretch();
    d->controlButtonsLayout->addWidget(d->testButton);
    d->controlButtonsLayout->addWidget(d->applyButton);
    d->controlButtonsLayout->addWidget(d->resetButton);
    
    return widget;
}

QWidget* ConnectionWidget::createStatusGroup()
{
    QWidget* widget = new QWidget();
    d->statusLayout = new QHBoxLayout(widget);
    
    d->statusLabel = new QLabel("Ready");
    d->connectionProgressBar = new QProgressBar();
    d->connectionProgressBar->setVisible(false);
    d->connectionProgressBar->setMaximumHeight(16);
    
    d->statusLayout->addWidget(d->statusLabel);
    d->statusLayout->addStretch();
    d->statusLayout->addWidget(d->connectionProgressBar);
    
    return widget;
}

void ConnectionWidget::updateUIState()
{
    bool enabled = !d->readOnly;
    
    // 服务器配置
    d->serverUrlEdit->setEnabled(enabled);
    d->serverPortSpinBox->setEnabled(enabled);
    d->serverDomainEdit->setEnabled(enabled);
    
    // 连接选项
    d->connectionTimeoutSpinBox->setEnabled(enabled);
    d->autoReconnectCheckBox->setEnabled(enabled);
    d->reconnectIntervalSpinBox->setEnabled(enabled);
    d->maxReconnectAttemptsSpinBox->setEnabled(enabled);
    d->webRTCEnabledCheckBox->setEnabled(enabled);
    d->webSocketEnabledCheckBox->setEnabled(enabled);
    d->httpsOnlyCheckBox->setEnabled(enabled);
    
    // 按钮状态
    d->applyButton->setEnabled(enabled && d->configurationChanged);
    d->resetButton->setEnabled(enabled);
    
    updateConnectionButtons(d->currentConnectionState);
}

void ConnectionWidget::updateConnectionButtons(int state)
{
    bool canConnect = (state == 0); // Disconnected
    bool canDisconnect = (state == 1 || state == 2); // Connecting or Connected
    
    d->connectButton->setEnabled(canConnect && !d->readOnly);
    d->disconnectButton->setEnabled(canDisconnect);
    d->testButton->setEnabled(!d->readOnly && state != 1); // Not connecting
}

bool ConnectionWidget::validateInput()
{
    QString url = d->serverUrlEdit->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Server URL is required.");
        d->serverUrlEdit->setFocus();
        return false;
    }
    
    QUrl qurl(url);
    if (!qurl.isValid() || qurl.scheme().isEmpty() || qurl.host().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Invalid server URL format.");
        d->serverUrlEdit->setFocus();
        return false;
    }
    
    return true;
}

void ConnectionWidget::updateConfigFromUI()
{
    if (!d->networkConfig) {
        return;
    }
    
    // 服务器配置
    d->networkConfig->setServerUrl(d->serverUrlEdit->text().trimmed());
    d->networkConfig->setServerPort(d->serverPortSpinBox->value());
    d->networkConfig->setServerDomain(d->serverDomainEdit->text().trimmed());
    
    // 连接配置
    d->networkConfig->setConnectionTimeout(d->connectionTimeoutSpinBox->value());
    d->networkConfig->setAutoReconnect(d->autoReconnectCheckBox->isChecked());
    d->networkConfig->setReconnectInterval(d->reconnectIntervalSpinBox->value());
    d->networkConfig->setMaxReconnectAttempts(d->maxReconnectAttemptsSpinBox->value());
    
    // 协议配置
    d->networkConfig->setWebRTCEnabled(d->webRTCEnabledCheckBox->isChecked());
    d->networkConfig->setWebSocketEnabled(d->webSocketEnabledCheckBox->isChecked());
    d->networkConfig->setHttpsOnly(d->httpsOnlyCheckBox->isChecked());
}

void ConnectionWidget::updateUIFromConfig()
{
    if (!d->networkConfig) {
        return;
    }
    
    // 服务器配置
    d->serverUrlEdit->setText(d->networkConfig->serverUrl());
    d->serverPortSpinBox->setValue(d->networkConfig->serverPort());
    d->serverDomainEdit->setText(d->networkConfig->serverDomain());
    
    // 连接配置
    d->connectionTimeoutSpinBox->setValue(d->networkConfig->connectionTimeout());
    d->autoReconnectCheckBox->setChecked(d->networkConfig->autoReconnect());
    d->reconnectIntervalSpinBox->setValue(d->networkConfig->reconnectInterval());
    d->maxReconnectAttemptsSpinBox->setValue(d->networkConfig->maxReconnectAttempts());
    
    // 协议配置
    d->webRTCEnabledCheckBox->setChecked(d->networkConfig->webRTCEnabled());
    d->webSocketEnabledCheckBox->setChecked(d->networkConfig->webSocketEnabled());
    d->httpsOnlyCheckBox->setChecked(d->networkConfig->httpsOnly());
    
    d->configurationChanged = false;
}

void ConnectionWidget::loadPresetList()
{
    d->presetComboBox->clear();
    d->presetComboBox->addItem("Custom");
    
    QSettings settings;
    settings.beginGroup("ConnectionPresets");
    QStringList presets = settings.childKeys();
    d->presetComboBox->addItems(presets);
    settings.endGroup();
}

void ConnectionWidget::savePresetList()
{
    // 预设列表由QSettings自动管理
}

void ConnectionWidget::applyStyles()
{
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #d0d0d0;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 5px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        
        QPushButton {
            padding: 6px 12px;
            border: 1px solid #d0d0d0;
            border-radius: 3px;
            background-color: #f8f8f8;
        }
        
        QPushButton:hover {
            background-color: #e8e8e8;
        }
        
        QPushButton:pressed {
            background-color: #d8d8d8;
        }
        
        QPushButton:disabled {
            color: #888888;
            background-color: #f0f0f0;
        }
        
        QPushButton[default="true"] {
            background-color: #007acc;
            color: white;
            border-color: #005a9e;
        }
        
        QPushButton[default="true"]:hover {
            background-color: #005a9e;
        }
        
        QLineEdit, QSpinBox, QComboBox {
            padding: 4px;
            border: 1px solid #d0d0d0;
            border-radius: 3px;
        }
        
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
            border-color: #007acc;
        }
        
        QProgressBar {
            border: 1px solid #d0d0d0;
            border-radius: 3px;
            text-align: center;
        }
        
        QProgressBar::chunk {
            background-color: #007acc;
            border-radius: 2px;
        }
    )");
}

void ConnectionWidget::setupTooltips()
{
    d->serverUrlEdit->setToolTip("Enter the Jitsi Meet server URL (e.g., https://meet.jit.si)");
    d->serverPortSpinBox->setToolTip("Server port number (default: 443 for HTTPS)");
    d->serverDomainEdit->setToolTip("Server domain name");
    
    d->connectionTimeoutSpinBox->setToolTip("Connection timeout in milliseconds");
    d->autoReconnectCheckBox->setToolTip("Automatically reconnect on connection loss");
    d->reconnectIntervalSpinBox->setToolTip("Interval between reconnection attempts");
    d->maxReconnectAttemptsSpinBox->setToolTip("Maximum number of reconnection attempts");
    
    d->webRTCEnabledCheckBox->setToolTip("Enable WebRTC for peer-to-peer communication");
    d->webSocketEnabledCheckBox->setToolTip("Enable WebSocket for real-time communication");
    d->httpsOnlyCheckBox->setToolTip("Use only secure HTTPS connections");
    
    d->connectButton->setToolTip("Connect to the server");
    d->disconnectButton->setToolTip("Disconnect from the server");
    d->testButton->setToolTip("Test connection to the server");
    d->applyButton->setToolTip("Apply configuration changes");
    d->resetButton->setToolTip("Reset configuration to defaults");
}