#include "StatusBar.h"
#include "../themes/BaseTheme.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QDebug>

StatusBar::StatusBar(QWidget *parent)
    : QStatusBar(parent)
    , m_statusType(InfoStatus)
    , m_progressVisible(false)
    , m_progressValue(0)
    , m_progressMinimum(0)
    , m_progressMaximum(100)
    , m_connectionStatus(false)
    , m_networkQuality(0)
    , m_recordingStatus(false)
    , m_muteStatus(false)
    , m_statusLabel(nullptr)
    , m_statusIcon(nullptr)
    , m_progressBar(nullptr)
    , m_connectionIndicator(nullptr)
    , m_networkIndicator(nullptr)
    , m_recordingIndicator(nullptr)
    , m_muteIndicator(nullptr)
    , m_messageTimer(new QTimer(this))
    , m_progressTimer(new QTimer(this))
{
    setupStatusBar();
    setupWidgets();
}

StatusBar::~StatusBar() = default;

QString StatusBar::statusText() const
{
    return m_statusText;
}

void StatusBar::setStatusText(const QString& text)
{
    if (m_statusText != text) {
        m_statusText = text;
        updateStatusDisplay();
        emit statusTextChanged(text);
    }
}

void StatusBar::showMessage(const QString& message, int timeout)
{
    showMessage(message, InfoStatus, timeout);
}

void StatusBar::showMessage(const QString& message, StatusType type, int timeout)
{
    setStatusType(type);
    setStatusText(message);
    
    if (timeout > 0) {
        m_messageTimer->start(timeout);
    }
}

StatusBar::StatusType StatusBar::statusType() const
{
    return m_statusType;
}

void StatusBar::setStatusType(StatusType type)
{
    if (m_statusType != type) {
        m_statusType = type;
        updateStatusDisplay();
        emit statusTypeChanged(type);
    }
}

bool StatusBar::isProgressVisible() const
{
    return m_progressVisible;
}

void StatusBar::setProgressVisible(bool visible)
{
    if (m_progressVisible != visible) {
        m_progressVisible = visible;
        updateProgressDisplay();
        emit progressVisibleChanged(visible);
    }
}

int StatusBar::progressValue() const
{
    return m_progressValue;
}

void StatusBar::setProgressValue(int value)
{
    if (m_progressValue != value) {
        m_progressValue = qBound(m_progressMinimum, value, m_progressMaximum);
        if (m_progressBar) {
            m_progressBar->setValue(m_progressValue);
        }
        emit progressValueChanged(m_progressValue);
    }
}

void StatusBar::setProgressRange(int minimum, int maximum)
{
    m_progressMinimum = minimum;
    m_progressMaximum = maximum;
    if (m_progressBar) {
        m_progressBar->setRange(minimum, maximum);
    }
}

void StatusBar::showProgress(const QString& text)
{
    if (!text.isEmpty()) {
        setStatusText(text);
    }
    setProgressVisible(true);
}

void StatusBar::hideProgress()
{
    setProgressVisible(false);
}

void StatusBar::showConnectionStatus(bool connected)
{
    if (m_connectionStatus != connected) {
        m_connectionStatus = connected;
        updateIndicators();
        emit connectionStatusChanged(connected);
    }
}

void StatusBar::showNetworkQuality(int quality)
{
    quality = qBound(0, quality, 100);
    if (m_networkQuality != quality) {
        m_networkQuality = quality;
        updateIndicators();
        emit networkQualityChanged(quality);
    }
}

void StatusBar::showRecordingStatus(bool recording)
{
    if (m_recordingStatus != recording) {
        m_recordingStatus = recording;
        updateIndicators();
        emit recordingStatusChanged(recording);
    }
}

void StatusBar::showMuteStatus(bool muted)
{
    if (m_muteStatus != muted) {
        m_muteStatus = muted;
        updateIndicators();
        emit muteStatusChanged(muted);
    }
}

void StatusBar::applyTheme(std::shared_ptr<BaseTheme> theme)
{
    if (!theme) {
        return;
    }

    m_currentTheme = theme;
    onThemeChanged(theme);
    updateThemeColors();
    updateThemeFonts();
}

QVariantMap StatusBar::getConfiguration() const
{
    QVariantMap config = getDefaultConfiguration();
    config["statusText"] = m_statusText;
    config["statusType"] = static_cast<int>(m_statusType);
    config["progressVisible"] = m_progressVisible;
    config["progressValue"] = m_progressValue;
    config["connectionStatus"] = m_connectionStatus;
    config["networkQuality"] = m_networkQuality;
    config["recordingStatus"] = m_recordingStatus;
    config["muteStatus"] = m_muteStatus;
    return config;
}

void StatusBar::setConfiguration(const QVariantMap& config)
{
    if (!validateConfiguration(config)) {
        qWarning() << "Invalid configuration for StatusBar";
        return;
    }

    if (config.contains("statusText")) {
        setStatusText(config["statusText"].toString());
    }
    if (config.contains("statusType")) {
        setStatusType(static_cast<StatusType>(config["statusType"].toInt()));
    }
    if (config.contains("progressVisible")) {
        setProgressVisible(config["progressVisible"].toBool());
    }
    if (config.contains("progressValue")) {
        setProgressValue(config["progressValue"].toInt());
    }
    if (config.contains("connectionStatus")) {
        showConnectionStatus(config["connectionStatus"].toBool());
    }
    if (config.contains("networkQuality")) {
        showNetworkQuality(config["networkQuality"].toInt());
    }
    if (config.contains("recordingStatus")) {
        showRecordingStatus(config["recordingStatus"].toBool());
    }
    if (config.contains("muteStatus")) {
        showMuteStatus(config["muteStatus"].toBool());
    }
}

QString StatusBar::componentName() const
{
    return "StatusBar";
}

void StatusBar::onThemeChanged(std::shared_ptr<BaseTheme> theme)
{
    Q_UNUSED(theme)
    // 主题变化处理
}

QString StatusBar::getDefaultStyleSheet() const
{
    return "QStatusBar { background-color: #F8F9FA; border-top: 1px solid #DEE2E6; }";
}

void StatusBar::updateThemeColors()
{
    setStyleSheet(getDefaultStyleSheet());
    updateStatusDisplay();
    updateIndicators();
}

void StatusBar::updateThemeFonts()
{
    // 更新字体
}

QVariantMap StatusBar::getDefaultConfiguration() const
{
    QVariantMap config;
    config["statusText"] = QString();
    config["statusType"] = static_cast<int>(InfoStatus);
    config["progressVisible"] = false;
    config["progressValue"] = 0;
    config["connectionStatus"] = false;
    config["networkQuality"] = 0;
    config["recordingStatus"] = false;
    config["muteStatus"] = false;
    return config;
}

bool StatusBar::validateConfiguration(const QVariantMap& config) const
{
    if (config.contains("statusType")) {
        int type = config["statusType"].toInt();
        if (type < InfoStatus || type > BusyStatus) {
            return false;
        }
    }
    
    if (config.contains("networkQuality")) {
        int quality = config["networkQuality"].toInt();
        if (quality < 0 || quality > 100) {
            return false;
        }
    }
    
    return true;
}

void StatusBar::resizeEvent(QResizeEvent *event)
{
    QStatusBar::resizeEvent(event);
    arrangeWidgets();
}

void StatusBar::onMessageTimeout()
{
    setStatusText(QString());
    m_messageTimer->stop();
}

void StatusBar::onProgressUpdate()
{
    // 进度更新处理
}

void StatusBar::setupStatusBar()
{
    // 连接定时器
    connect(m_messageTimer, &QTimer::timeout, this, &StatusBar::onMessageTimeout);
    connect(m_progressTimer, &QTimer::timeout, this, &StatusBar::onProgressUpdate);
    
    // 设置状态栏属性
    setSizeGripEnabled(true);
}

void StatusBar::setupWidgets()
{
    // 状态文本和图标
    m_statusIcon = new QLabel(this);
    m_statusIcon->setFixedSize(16, 16);
    m_statusIcon->setScaledContents(true);
    
    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    m_progressBar->setRange(m_progressMinimum, m_progressMaximum);
    
    // 状态指示器
    m_connectionIndicator = new QLabel(this);
    m_connectionIndicator->setFixedSize(16, 16);
    m_connectionIndicator->setToolTip("Connection Status");
    
    m_networkIndicator = new QLabel(this);
    m_networkIndicator->setFixedSize(16, 16);
    m_networkIndicator->setToolTip("Network Quality");
    
    m_recordingIndicator = new QLabel(this);
    m_recordingIndicator->setFixedSize(16, 16);
    m_recordingIndicator->setToolTip("Recording Status");
    
    m_muteIndicator = new QLabel(this);
    m_muteIndicator->setFixedSize(16, 16);
    m_muteIndicator->setToolTip("Mute Status");
    
    // 添加到状态栏
    addWidget(m_statusIcon);
    addWidget(m_statusLabel, 1);
    addWidget(m_progressBar);
    addPermanentWidget(m_connectionIndicator);
    addPermanentWidget(m_networkIndicator);
    addPermanentWidget(m_recordingIndicator);
    addPermanentWidget(m_muteIndicator);
    
    updateStatusDisplay();
    updateIndicators();
}

void StatusBar::updateStatusDisplay()
{
    if (m_statusLabel) {
        m_statusLabel->setText(m_statusText);
    }
    
    if (m_statusIcon) {
        QString iconText = getStatusIcon(m_statusType);
        m_statusIcon->setText(iconText);
        
        QString color = getStatusColor(m_statusType);
        m_statusIcon->setStyleSheet(QString("color: %1;").arg(color));
    }
}

void StatusBar::updateProgressDisplay()
{
    if (m_progressBar) {
        m_progressBar->setVisible(m_progressVisible);
    }
}

void StatusBar::updateIndicators()
{
    // 连接状态指示器
    if (m_connectionIndicator) {
        QString connectionText = m_connectionStatus ? "●" : "○";
        QString connectionColor = m_connectionStatus ? "#28A745" : "#DC3545";
        m_connectionIndicator->setText(connectionText);
        m_connectionIndicator->setStyleSheet(QString("color: %1; font-weight: bold;").arg(connectionColor));
    }
    
    // 网络质量指示器
    if (m_networkIndicator) {
        QString qualityText;
        QString qualityColor;
        
        if (m_networkQuality >= 80) {
            qualityText = "▲▲▲";
            qualityColor = "#28A745";
        } else if (m_networkQuality >= 60) {
            qualityText = "▲▲○";
            qualityColor = "#FFC107";
        } else if (m_networkQuality >= 40) {
            qualityText = "▲○○";
            qualityColor = "#FF6B35";
        } else {
            qualityText = "○○○";
            qualityColor = "#DC3545";
        }
        
        m_networkIndicator->setText(qualityText);
        m_networkIndicator->setStyleSheet(QString("color: %1; font-size: 8px;").arg(qualityColor));
    }
    
    // 录制状态指示器
    if (m_recordingIndicator) {
        QString recordingText = m_recordingStatus ? "●" : "";
        m_recordingIndicator->setText(recordingText);
        m_recordingIndicator->setStyleSheet("color: #DC3545; font-weight: bold;");
    }
    
    // 静音状态指示器
    if (m_muteIndicator) {
        QString muteText = m_muteStatus ? "🔇" : "🔊";
        m_muteIndicator->setText(muteText);
    }
}

QString StatusBar::getStatusIcon(StatusType type) const
{
    switch (type) {
        case SuccessStatus:
            return "✓";
        case WarningStatus:
            return "⚠";
        case ErrorStatus:
            return "✗";
        case BusyStatus:
            return "⟳";
        default: // InfoStatus
            return "ℹ";
    }
}

QString StatusBar::getStatusColor(StatusType type) const
{
    switch (type) {
        case SuccessStatus:
            return "#28A745";
        case WarningStatus:
            return "#FFC107";
        case ErrorStatus:
            return "#DC3545";
        case BusyStatus:
            return "#17A2B8";
        default: // InfoStatus
            return "#6C757D";
    }
}

void StatusBar::arrangeWidgets()
{
    // 重新排列组件
}