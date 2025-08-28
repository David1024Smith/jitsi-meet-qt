#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QGroupBox>
#include <QMessageBox>
#include <QDebug>

#include "ProtocolHandler.h"
#include "JitsiConstants.h"

class ProtocolHandlerDemo : public QMainWindow
{
    Q_OBJECT

public:
    ProtocolHandlerDemo(QWidget* parent = nullptr);
    ~ProtocolHandlerDemo();

private slots:
    void onRegisterProtocol();
    void onUnregisterProtocol();
    void onParseUrl();
    void onValidateUrl();
    void onTestProtocolUrl();
    void onProtocolUrlReceived(const QString& url);
    void onClearLog();

private:
    void setupUI();
    void logMessage(const QString& message);
    void updateStatus();

private:
    ProtocolHandler* m_protocolHandler;
    
    // UI组件
    QLineEdit* m_urlInput;
    QPushButton* m_registerButton;
    QPushButton* m_unregisterButton;
    QPushButton* m_parseButton;
    QPushButton* m_validateButton;
    QPushButton* m_testButton;
    QPushButton* m_clearButton;
    QLabel* m_statusLabel;
    QTextEdit* m_logOutput;
    
    bool m_protocolRegistered;
};

ProtocolHandlerDemo::ProtocolHandlerDemo(QWidget* parent)
    : QMainWindow(parent)
    , m_protocolHandler(nullptr)
    , m_protocolRegistered(false)
{
    setWindowTitle("Protocol Handler Demo - Jitsi Meet Qt");
    setMinimumSize(800, 600);
    
    // 创建协议处理器
    m_protocolHandler = new ProtocolHandler(this);
    
    // 连接信号
    connect(m_protocolHandler, &ProtocolHandler::protocolUrlReceived,
            this, &ProtocolHandlerDemo::onProtocolUrlReceived);
    
    setupUI();
    updateStatus();
    
    logMessage("Protocol Handler Demo initialized");
    logMessage(QString("Protocol scheme: %1").arg(JitsiConstants::PROTOCOL_SCHEME));
    logMessage(QString("Protocol prefix: %1").arg(JitsiConstants::PROTOCOL_PREFIX));
}

ProtocolHandlerDemo::~ProtocolHandlerDemo()
{
    if (m_protocolRegistered && m_protocolHandler) {
        m_protocolHandler->unregisterProtocol();
    }
}

void ProtocolHandlerDemo::setupUI()
{
    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    auto mainLayout = new QVBoxLayout(centralWidget);
    
    // 状态组
    auto statusGroup = new QGroupBox("Protocol Status", this);
    auto statusLayout = new QHBoxLayout(statusGroup);
    
    m_statusLabel = new QLabel("Not Registered", this);
    m_statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    statusLayout->addWidget(new QLabel("Status:", this));
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    
    mainLayout->addWidget(statusGroup);
    
    // 协议管理组
    auto protocolGroup = new QGroupBox("Protocol Management", this);
    auto protocolLayout = new QHBoxLayout(protocolGroup);
    
    m_registerButton = new QPushButton("Register Protocol", this);
    m_unregisterButton = new QPushButton("Unregister Protocol", this);
    
    connect(m_registerButton, &QPushButton::clicked, this, &ProtocolHandlerDemo::onRegisterProtocol);
    connect(m_unregisterButton, &QPushButton::clicked, this, &ProtocolHandlerDemo::onUnregisterProtocol);
    
    protocolLayout->addWidget(m_registerButton);
    protocolLayout->addWidget(m_unregisterButton);
    protocolLayout->addStretch();
    
    mainLayout->addWidget(protocolGroup);
    
    // URL测试组
    auto urlGroup = new QGroupBox("URL Testing", this);
    auto urlLayout = new QVBoxLayout(urlGroup);
    
    // URL输入
    auto inputLayout = new QHBoxLayout();
    inputLayout->addWidget(new QLabel("Test URL:", this));
    m_urlInput = new QLineEdit(this);
    m_urlInput->setPlaceholderText("Enter jitsi-meet:// URL to test...");
    m_urlInput->setText("jitsi-meet://test-room");
    inputLayout->addWidget(m_urlInput);
    
    urlLayout->addLayout(inputLayout);
    
    // 按钮行
    auto buttonLayout = new QHBoxLayout();
    m_parseButton = new QPushButton("Parse URL", this);
    m_validateButton = new QPushButton("Validate URL", this);
    m_testButton = new QPushButton("Test Protocol URL", this);
    
    connect(m_parseButton, &QPushButton::clicked, this, &ProtocolHandlerDemo::onParseUrl);
    connect(m_validateButton, &QPushButton::clicked, this, &ProtocolHandlerDemo::onValidateUrl);
    connect(m_testButton, &QPushButton::clicked, this, &ProtocolHandlerDemo::onTestProtocolUrl);
    
    buttonLayout->addWidget(m_parseButton);
    buttonLayout->addWidget(m_validateButton);
    buttonLayout->addWidget(m_testButton);
    buttonLayout->addStretch();
    
    urlLayout->addLayout(buttonLayout);
    
    mainLayout->addWidget(urlGroup);
    
    // 日志输出组
    auto logGroup = new QGroupBox("Log Output", this);
    auto logLayout = new QVBoxLayout(logGroup);
    
    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setMaximumBlockCount(1000); // 限制日志行数
    
    auto logButtonLayout = new QHBoxLayout();
    m_clearButton = new QPushButton("Clear Log", this);
    connect(m_clearButton, &QPushButton::clicked, this, &ProtocolHandlerDemo::onClearLog);
    logButtonLayout->addWidget(m_clearButton);
    logButtonLayout->addStretch();
    
    logLayout->addWidget(m_logOutput);
    logLayout->addLayout(logButtonLayout);
    
    mainLayout->addWidget(logGroup);
    
    // 设置布局比例
    mainLayout->setStretch(0, 0); // 状态组
    mainLayout->setStretch(1, 0); // 协议管理组
    mainLayout->setStretch(2, 0); // URL测试组
    mainLayout->setStretch(3, 1); // 日志输出组（占用剩余空间）
}

void ProtocolHandlerDemo::onRegisterProtocol()
{
    if (m_protocolRegistered) {
        logMessage("Protocol is already registered");
        return;
    }
    
    logMessage("Attempting to register protocol...");
    
    bool success = m_protocolHandler->registerProtocol();
    if (success) {
        m_protocolRegistered = true;
        logMessage("✓ Protocol registered successfully");
        QMessageBox::information(this, "Success", 
            "Protocol registered successfully!\n\n"
            "You can now test protocol URLs by:\n"
            "1. Opening Run dialog (Win+R)\n"
            "2. Entering: jitsi-meet://test-room\n"
            "3. Pressing Enter");
    } else {
        logMessage("✗ Failed to register protocol");
        QMessageBox::warning(this, "Error", 
            "Failed to register protocol.\n\n"
            "This might be due to:\n"
            "• Insufficient permissions\n"
            "• Platform not supported\n"
            "• Registry access issues");
    }
    
    updateStatus();
}

void ProtocolHandlerDemo::onUnregisterProtocol()
{
    if (!m_protocolRegistered) {
        logMessage("Protocol is not registered");
        return;
    }
    
    logMessage("Unregistering protocol...");
    m_protocolHandler->unregisterProtocol();
    m_protocolRegistered = false;
    logMessage("✓ Protocol unregistered");
    
    updateStatus();
}

void ProtocolHandlerDemo::onParseUrl()
{
    QString url = m_urlInput->text().trimmed();
    if (url.isEmpty()) {
        logMessage("Please enter a URL to parse");
        return;
    }
    
    logMessage(QString("Parsing URL: %1").arg(url));
    
    QString parsedUrl = m_protocolHandler->parseProtocolUrl(url);
    if (!parsedUrl.isEmpty()) {
        logMessage(QString("✓ Parsed result: %1").arg(parsedUrl));
    } else {
        logMessage("✗ Failed to parse URL (invalid format)");
    }
}

void ProtocolHandlerDemo::onValidateUrl()
{
    QString url = m_urlInput->text().trimmed();
    if (url.isEmpty()) {
        logMessage("Please enter a URL to validate");
        return;
    }
    
    logMessage(QString("Validating URL: %1").arg(url));
    
    bool isValid = m_protocolHandler->isValidProtocolUrl(url);
    if (isValid) {
        logMessage("✓ URL is valid");
    } else {
        logMessage("✗ URL is invalid");
    }
}

void ProtocolHandlerDemo::onTestProtocolUrl()
{
    QString url = m_urlInput->text().trimmed();
    if (url.isEmpty()) {
        logMessage("Please enter a URL to test");
        return;
    }
    
    logMessage(QString("Testing protocol URL: %1").arg(url));
    
    // 模拟协议URL接收
    if (m_protocolHandler->isValidProtocolUrl(url)) {
        emit m_protocolHandler->protocolUrlReceived(url);
    } else {
        logMessage("✗ Cannot test invalid URL");
    }
}

void ProtocolHandlerDemo::onProtocolUrlReceived(const QString& url)
{
    logMessage(QString("🔗 Protocol URL received: %1").arg(url));
    
    // 解析URL
    QString parsedUrl = m_protocolHandler->parseProtocolUrl(url);
    if (!parsedUrl.isEmpty()) {
        logMessage(QString("📋 Parsed conference URL: %1").arg(parsedUrl));
        
        // 显示信息对话框
        QMessageBox::information(this, "Protocol URL Received",
            QString("Received protocol URL:\n%1\n\nParsed conference URL:\n%2")
            .arg(url).arg(parsedUrl));
    } else {
        logMessage("✗ Failed to parse received URL");
    }
}

void ProtocolHandlerDemo::onClearLog()
{
    m_logOutput->clear();
    logMessage("Log cleared");
}

void ProtocolHandlerDemo::logMessage(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp).arg(message);
    
    m_logOutput->append(logEntry);
    
    // 自动滚动到底部
    QTextCursor cursor = m_logOutput->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logOutput->setTextCursor(cursor);
    
    // 同时输出到控制台
    qDebug() << logEntry;
}

void ProtocolHandlerDemo::updateStatus()
{
    if (m_protocolRegistered) {
        m_statusLabel->setText("Registered");
        m_statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
        m_registerButton->setEnabled(false);
        m_unregisterButton->setEnabled(true);
    } else {
        m_statusLabel->setText("Not Registered");
        m_statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
        m_registerButton->setEnabled(true);
        m_unregisterButton->setEnabled(false);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName(JitsiConstants::APP_NAME);
    app.setApplicationVersion(JitsiConstants::APP_VERSION);
    app.setOrganizationName(JitsiConstants::APP_ORGANIZATION);
    
    ProtocolHandlerDemo demo;
    demo.show();
    
    return app.exec();
}

#include "protocol_handler_demo.moc"