#include "ErrorDialog.h"
#include <QApplication>
#include <QStyle>
#include <QScreen>
#include <QShowEvent>
#include <QCloseEvent>
#include <QSpacerItem>
#include <QFont>

ErrorDialog::ErrorDialog(const JitsiError& error, QWidget* parent)
    : QDialog(parent)
    , m_error(error)
    , m_result(Cancel)
    , m_autoCloseTimeout(30)
    , m_remainingSeconds(30)
    , m_showDetails(false)
    , m_retryEnabled(true)
    , m_resetEnabled(false)
{
    setupUI();
    setupConnections();
    
    // 根据错误类型调整按钮状态
    updateButtonStates();
    
    // 设置窗口属性
    setWindowTitle("Jitsi Meet - 错误");
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setModal(true);
    resize(400, 200);
    
    // 居中显示
    if (parent) {
        move(parent->geometry().center() - rect().center());
    } else {
        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            move(screen->geometry().center() - rect().center());
        }
    }
}

ErrorDialog::~ErrorDialog()
{
    stopCountdown();
}

void ErrorDialog::setShowDetails(bool show)
{
    m_showDetails = show;
    m_detailsEdit->setVisible(show);
    m_detailsButton->setText(show ? "隐藏详情 <<" : "显示详情 >>");
    
    // 调整对话框大小
    if (show) {
        resize(500, 350);
    } else {
        resize(400, 200);
    }
}

void ErrorDialog::setAutoCloseTimeout(int seconds)
{
    m_autoCloseTimeout = seconds;
    m_remainingSeconds = seconds;
    
    if (seconds > 0) {
        m_countdownLabel->setVisible(true);
        m_countdownProgress->setVisible(true);
        m_countdownProgress->setMaximum(seconds);
    } else {
        m_countdownLabel->setVisible(false);
        m_countdownProgress->setVisible(false);
    }
}

void ErrorDialog::setRetryEnabled(bool enabled)
{
    m_retryEnabled = enabled;
    m_retryButton->setVisible(enabled);
}

void ErrorDialog::setResetEnabled(bool enabled)
{
    m_resetEnabled = enabled;
    m_resetButton->setVisible(enabled);
}

void ErrorDialog::setErrorIcon(const QPixmap& icon)
{
    m_iconLabel->setPixmap(icon);
}

void ErrorDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    
    if (m_autoCloseTimeout > 0) {
        startCountdown();
    }
}

void ErrorDialog::closeEvent(QCloseEvent* event)
{
    stopCountdown();
    QDialog::closeEvent(event);
}

void ErrorDialog::onRetryClicked()
{
    m_result = Retry;
    accept();
}

void ErrorDialog::onIgnoreClicked()
{
    m_result = Ignore;
    accept();
}

void ErrorDialog::onResetClicked()
{
    m_result = Reset;
    accept();
}

void ErrorDialog::onCancelClicked()
{
    m_result = Cancel;
    reject();
}

void ErrorDialog::onDetailsToggled()
{
    setShowDetails(!m_showDetails);
}

void ErrorDialog::onAutoCloseTimeout()
{
    m_result = Ignore;
    accept();
}

void ErrorDialog::updateCountdown()
{
    m_remainingSeconds--;
    
    if (m_remainingSeconds <= 0) {
        onAutoCloseTimeout();
        return;
    }
    
    m_countdownLabel->setText(QString("对话框将在 %1 秒后自动关闭").arg(m_remainingSeconds));
    m_countdownProgress->setValue(m_autoCloseTimeout - m_remainingSeconds);
}

void ErrorDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_topLayout = new QHBoxLayout();
    m_buttonLayout = new QHBoxLayout();
    
    // 创建图标标签
    m_iconLabel = new QLabel();
    m_iconLabel->setFixedSize(48, 48);
    m_iconLabel->setScaledContents(true);
    
    // 根据错误严重程度设置图标
    QStyle* style = QApplication::style();
    QPixmap icon;
    
    switch (m_error.severity()) {
    case ErrorSeverity::Info:
        icon = style->standardPixmap(QStyle::SP_MessageBoxInformation);
        break;
    case ErrorSeverity::Warning:
        icon = style->standardPixmap(QStyle::SP_MessageBoxWarning);
        break;
    case ErrorSeverity::Error:
    case ErrorSeverity::Critical:
        icon = style->standardPixmap(QStyle::SP_MessageBoxCritical);
        break;
    }
    
    m_iconLabel->setPixmap(icon);
    
    // 创建消息标签
    m_messageLabel = new QLabel();
    m_messageLabel->setText(m_error.toUserMessage());
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    // 设置消息字体
    QFont messageFont = m_messageLabel->font();
    messageFont.setPointSize(messageFont.pointSize() + 1);
    m_messageLabel->setFont(messageFont);
    
    // 顶部布局（图标 + 消息）
    m_topLayout->addWidget(m_iconLabel);
    m_topLayout->addWidget(m_messageLabel, 1);
    m_topLayout->setSpacing(15);
    
    // 创建详情文本框
    m_detailsEdit = new QTextEdit();
    m_detailsEdit->setPlainText(m_error.details());
    m_detailsEdit->setReadOnly(true);
    m_detailsEdit->setMaximumHeight(150);
    m_detailsEdit->setVisible(false);
    
    // 创建倒计时标签和进度条
    m_countdownLabel = new QLabel();
    m_countdownLabel->setAlignment(Qt::AlignCenter);
    m_countdownLabel->setVisible(false);
    
    m_countdownProgress = new QProgressBar();
    m_countdownProgress->setTextVisible(false);
    m_countdownProgress->setMaximumHeight(6);
    m_countdownProgress->setVisible(false);
    
    // 创建按钮
    m_retryButton = new QPushButton("重试");
    m_ignoreButton = new QPushButton("忽略");
    m_resetButton = new QPushButton("重置");
    m_cancelButton = new QPushButton("取消");
    m_detailsButton = new QPushButton("显示详情 >>");
    
    // 设置默认按钮
    m_retryButton->setDefault(true);
    
    // 按钮布局
    m_buttonLayout->addWidget(m_detailsButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_retryButton);
    m_buttonLayout->addWidget(m_ignoreButton);
    m_buttonLayout->addWidget(m_resetButton);
    m_buttonLayout->addWidget(m_cancelButton);
    
    // 主布局
    m_mainLayout->addLayout(m_topLayout);
    m_mainLayout->addWidget(m_detailsEdit);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_countdownLabel);
    m_mainLayout->addWidget(m_countdownProgress);
    m_mainLayout->addLayout(m_buttonLayout);
    
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(20, 20, 20, 15);
}

void ErrorDialog::setupConnections()
{
    connect(m_retryButton, &QPushButton::clicked, this, &ErrorDialog::onRetryClicked);
    connect(m_ignoreButton, &QPushButton::clicked, this, &ErrorDialog::onIgnoreClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &ErrorDialog::onResetClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ErrorDialog::onCancelClicked);
    connect(m_detailsButton, &QPushButton::clicked, this, &ErrorDialog::onDetailsToggled);
    
    // 创建定时器
    m_autoCloseTimer = new QTimer(this);
    m_autoCloseTimer->setSingleShot(true);
    connect(m_autoCloseTimer, &QTimer::timeout, this, &ErrorDialog::onAutoCloseTimeout);
    
    m_countdownTimer = new QTimer(this);
    connect(m_countdownTimer, &QTimer::timeout, this, &ErrorDialog::updateCountdown);
}

void ErrorDialog::updateButtonStates()
{
    // 根据错误类型和可恢复性调整按钮
    bool recoverable = m_error.isRecoverable();
    
    m_retryButton->setVisible(recoverable && m_retryEnabled);
    m_ignoreButton->setVisible(true);
    
    // 根据错误类型决定是否显示重置按钮
    switch (m_error.type()) {
    case ErrorType::ConfigurationError:
        m_resetEnabled = true;
        break;
    case ErrorType::WebEngineError:
        if (m_error.severity() != ErrorSeverity::Critical) {
            m_resetEnabled = true;
        }
        break;
    default:
        break;
    }
    
    m_resetButton->setVisible(m_resetEnabled);
    
    // 如果有详细信息，显示详情按钮
    m_detailsButton->setVisible(!m_error.details().isEmpty());
    
    // 设置按钮文本
    switch (m_error.type()) {
    case ErrorType::NetworkError:
        m_retryButton->setText("重新连接");
        m_ignoreButton->setText("离线模式");
        break;
    case ErrorType::InvalidUrl:
        m_retryButton->setText("重新输入");
        m_ignoreButton->setText("忽略");
        break;
    case ErrorType::WebEngineError:
        m_retryButton->setText("重新加载");
        m_resetButton->setText("重启组件");
        break;
    case ErrorType::ConfigurationError:
        m_resetButton->setText("重置设置");
        break;
    case ErrorType::WebRTCError:
        m_retryButton->setText("重新初始化");
        m_resetButton->setText("重置设备");
        break;
    case ErrorType::XMPPConnectionError:
        m_retryButton->setText("重新连接");
        m_ignoreButton->setText("离线模式");
        break;
    case ErrorType::AuthenticationError:
        m_retryButton->setText("重新验证");
        m_ignoreButton->setText("跳过验证");
        break;
    case ErrorType::MediaDeviceError:
        m_retryButton->setText("重新检测");
        m_resetButton->setText("重置权限");
        break;
    default:
        break;
    }
}

void ErrorDialog::startCountdown()
{
    if (m_autoCloseTimeout <= 0) {
        return;
    }
    
    m_remainingSeconds = m_autoCloseTimeout;
    m_countdownProgress->setValue(0);
    m_countdownProgress->setMaximum(m_autoCloseTimeout);
    
    updateCountdown();
    
    m_countdownTimer->start(1000); // 每秒更新
    m_autoCloseTimer->start(m_autoCloseTimeout * 1000);
}

void ErrorDialog::stopCountdown()
{
    if (m_countdownTimer) {
        m_countdownTimer->stop();
    }
    
    if (m_autoCloseTimer) {
        m_autoCloseTimer->stop();
    }
}