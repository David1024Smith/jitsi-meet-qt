#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include "AuthenticationManager.h"

/**
 * @brief AuthenticationManager演示应用程序
 * 
 * 这个演示程序展示了AuthenticationManager的各种功能：
 * - JWT token认证
 * - 密码认证
 * - 访客模式认证
 * - 房间权限检查
 * - 认证状态管理
 * - Token过期处理
 */
class AuthenticationDemo : public QMainWindow
{
    Q_OBJECT

public:
    AuthenticationDemo(QWidget* parent = nullptr);
    ~AuthenticationDemo();

private slots:
    void onAuthenticateClicked();
    void onJWTAuthClicked();
    void onPasswordAuthClicked();
    void onCheckPermissionsClicked();
    void onLogoutClicked();
    void onRefreshTokenClicked();
    
    // AuthenticationManager信号处理
    void onAuthStateChanged(AuthenticationManager::AuthState state);
    void onAuthenticationSucceeded(AuthenticationManager::AuthType type);
    void onAuthenticationFailed(const QString& error);
    void onPasswordRequired();
    void onJWTTokenRequired();
    void onRoomPermissionsUpdated(const AuthenticationManager::RoomPermissions& permissions);
    void onTokenExpiring(int expiresIn);
    void onTokenExpired();

private:
    void setupUI();
    void updateUI();
    void logMessage(const QString& message);
    QString createSampleJWTToken();
    
    AuthenticationManager* m_authManager;
    
    // UI组件
    QLineEdit* m_serverUrlEdit;
    QLineEdit* m_roomNameEdit;
    QLineEdit* m_displayNameEdit;
    QLineEdit* m_jwtTokenEdit;
    QLineEdit* m_passwordEdit;
    
    QPushButton* m_authenticateBtn;
    QPushButton* m_jwtAuthBtn;
    QPushButton* m_passwordAuthBtn;
    QPushButton* m_checkPermissionsBtn;
    QPushButton* m_logoutBtn;
    QPushButton* m_refreshTokenBtn;
    
    QLabel* m_statusLabel;
    QLabel* m_authTypeLabel;
    QLabel* m_userIdLabel;
    QTextEdit* m_logTextEdit;
    QTextEdit* m_permissionsTextEdit;
};

AuthenticationDemo::AuthenticationDemo(QWidget* parent)
    : QMainWindow(parent)
    , m_authManager(new AuthenticationManager(this))
{
    setWindowTitle("AuthenticationManager Demo");
    setMinimumSize(800, 600);
    
    setupUI();
    updateUI();
    
    // 连接AuthenticationManager信号
    connect(m_authManager, &AuthenticationManager::authStateChanged,
            this, &AuthenticationDemo::onAuthStateChanged);
    connect(m_authManager, &AuthenticationManager::authenticationSucceeded,
            this, &AuthenticationDemo::onAuthenticationSucceeded);
    connect(m_authManager, &AuthenticationManager::authenticationFailed,
            this, &AuthenticationDemo::onAuthenticationFailed);
    connect(m_authManager, &AuthenticationManager::passwordRequired,
            this, &AuthenticationDemo::onPasswordRequired);
    connect(m_authManager, &AuthenticationManager::jwtTokenRequired,
            this, &AuthenticationDemo::onJWTTokenRequired);
    connect(m_authManager, &AuthenticationManager::roomPermissionsUpdated,
            this, &AuthenticationDemo::onRoomPermissionsUpdated);
    connect(m_authManager, &AuthenticationManager::tokenExpiring,
            this, &AuthenticationDemo::onTokenExpiring);
    connect(m_authManager, &AuthenticationManager::tokenExpired,
            this, &AuthenticationDemo::onTokenExpired);
    
    logMessage("AuthenticationManager Demo started");
}

AuthenticationDemo::~AuthenticationDemo()
{
}

void AuthenticationDemo::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // 服务器配置组
    QGroupBox* serverGroup = new QGroupBox("Server Configuration");
    QVBoxLayout* serverLayout = new QVBoxLayout(serverGroup);
    
    QHBoxLayout* serverUrlLayout = new QHBoxLayout();
    serverUrlLayout->addWidget(new QLabel("Server URL:"));
    m_serverUrlEdit = new QLineEdit("https://meet.jit.si");
    serverUrlLayout->addWidget(m_serverUrlEdit);
    serverLayout->addLayout(serverUrlLayout);
    
    QHBoxLayout* roomLayout = new QHBoxLayout();
    roomLayout->addWidget(new QLabel("Room Name:"));
    m_roomNameEdit = new QLineEdit("test-room");
    roomLayout->addWidget(m_roomNameEdit);
    serverLayout->addLayout(roomLayout);
    
    QHBoxLayout* displayNameLayout = new QHBoxLayout();
    displayNameLayout->addWidget(new QLabel("Display Name:"));
    m_displayNameEdit = new QLineEdit("Demo User");
    displayNameLayout->addWidget(m_displayNameEdit);
    serverLayout->addLayout(displayNameLayout);
    
    mainLayout->addWidget(serverGroup);
    
    // 认证方法组
    QGroupBox* authGroup = new QGroupBox("Authentication Methods");
    QVBoxLayout* authLayout = new QVBoxLayout(authGroup);
    
    // 基本认证
    QHBoxLayout* basicAuthLayout = new QHBoxLayout();
    m_authenticateBtn = new QPushButton("Start Authentication");
    connect(m_authenticateBtn, &QPushButton::clicked, this, &AuthenticationDemo::onAuthenticateClicked);
    basicAuthLayout->addWidget(m_authenticateBtn);
    authLayout->addLayout(basicAuthLayout);
    
    // JWT认证
    QHBoxLayout* jwtLayout = new QHBoxLayout();
    jwtLayout->addWidget(new QLabel("JWT Token:"));
    m_jwtTokenEdit = new QLineEdit();
    m_jwtTokenEdit->setPlaceholderText("Enter JWT token or click 'Generate Sample'");
    jwtLayout->addWidget(m_jwtTokenEdit);
    
    QPushButton* generateJWTBtn = new QPushButton("Generate Sample");
    connect(generateJWTBtn, &QPushButton::clicked, [this]() {
        m_jwtTokenEdit->setText(createSampleJWTToken());
    });
    jwtLayout->addWidget(generateJWTBtn);
    
    m_jwtAuthBtn = new QPushButton("JWT Auth");
    connect(m_jwtAuthBtn, &QPushButton::clicked, this, &AuthenticationDemo::onJWTAuthClicked);
    jwtLayout->addWidget(m_jwtAuthBtn);
    authLayout->addLayout(jwtLayout);
    
    // 密码认证
    QHBoxLayout* passwordLayout = new QHBoxLayout();
    passwordLayout->addWidget(new QLabel("Password:"));
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Enter room password");
    passwordLayout->addWidget(m_passwordEdit);
    
    m_passwordAuthBtn = new QPushButton("Password Auth");
    connect(m_passwordAuthBtn, &QPushButton::clicked, this, &AuthenticationDemo::onPasswordAuthClicked);
    passwordLayout->addWidget(m_passwordAuthBtn);
    authLayout->addLayout(passwordLayout);
    
    mainLayout->addWidget(authGroup);
    
    // 操作按钮组
    QGroupBox* actionsGroup = new QGroupBox("Actions");
    QHBoxLayout* actionsLayout = new QHBoxLayout(actionsGroup);
    
    m_checkPermissionsBtn = new QPushButton("Check Permissions");
    connect(m_checkPermissionsBtn, &QPushButton::clicked, this, &AuthenticationDemo::onCheckPermissionsClicked);
    actionsLayout->addWidget(m_checkPermissionsBtn);
    
    m_refreshTokenBtn = new QPushButton("Refresh Token");
    connect(m_refreshTokenBtn, &QPushButton::clicked, this, &AuthenticationDemo::onRefreshTokenClicked);
    actionsLayout->addWidget(m_refreshTokenBtn);
    
    m_logoutBtn = new QPushButton("Logout");
    connect(m_logoutBtn, &QPushButton::clicked, this, &AuthenticationDemo::onLogoutClicked);
    actionsLayout->addWidget(m_logoutBtn);
    
    mainLayout->addWidget(actionsGroup);
    
    // 状态信息组
    QGroupBox* statusGroup = new QGroupBox("Status Information");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
    
    QHBoxLayout* statusInfoLayout = new QHBoxLayout();
    statusInfoLayout->addWidget(new QLabel("Auth State:"));
    m_statusLabel = new QLabel("Not Authenticated");
    statusInfoLayout->addWidget(m_statusLabel);
    statusInfoLayout->addStretch();
    
    statusInfoLayout->addWidget(new QLabel("Auth Type:"));
    m_authTypeLabel = new QLabel("None");
    statusInfoLayout->addWidget(m_authTypeLabel);
    statusInfoLayout->addStretch();
    
    statusInfoLayout->addWidget(new QLabel("User ID:"));
    m_userIdLabel = new QLabel("-");
    statusInfoLayout->addWidget(m_userIdLabel);
    statusLayout->addLayout(statusInfoLayout);
    
    mainLayout->addWidget(statusGroup);
    
    // 权限信息
    QGroupBox* permissionsGroup = new QGroupBox("Room Permissions");
    QVBoxLayout* permissionsLayout = new QVBoxLayout(permissionsGroup);
    
    m_permissionsTextEdit = new QTextEdit();
    m_permissionsTextEdit->setMaximumHeight(100);
    m_permissionsTextEdit->setReadOnly(true);
    permissionsLayout->addWidget(m_permissionsTextEdit);
    
    mainLayout->addWidget(permissionsGroup);
    
    // 日志输出
    QGroupBox* logGroup = new QGroupBox("Log Output");
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    
    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setMaximumHeight(150);
    m_logTextEdit->setReadOnly(true);
    logLayout->addWidget(m_logTextEdit);
    
    QPushButton* clearLogBtn = new QPushButton("Clear Log");
    connect(clearLogBtn, &QPushButton::clicked, m_logTextEdit, &QTextEdit::clear);
    logLayout->addWidget(clearLogBtn);
    
    mainLayout->addWidget(logGroup);
}

void AuthenticationDemo::updateUI()
{
    bool isAuthenticated = m_authManager->isAuthenticated();
    bool isAuthenticating = m_authManager->authState() == AuthenticationManager::Authenticating;
    
    // 更新按钮状态
    m_authenticateBtn->setEnabled(!isAuthenticated && !isAuthenticating);
    m_jwtAuthBtn->setEnabled(!isAuthenticated && !isAuthenticating);
    m_passwordAuthBtn->setEnabled(!isAuthenticated && !isAuthenticating);
    m_checkPermissionsBtn->setEnabled(isAuthenticated);
    m_refreshTokenBtn->setEnabled(isAuthenticated && m_authManager->authType() == AuthenticationManager::JWT);
    m_logoutBtn->setEnabled(isAuthenticated);
    
    // 更新状态标签
    QString stateText;
    switch (m_authManager->authState()) {
        case AuthenticationManager::NotAuthenticated:
            stateText = "Not Authenticated";
            break;
        case AuthenticationManager::Authenticating:
            stateText = "Authenticating...";
            break;
        case AuthenticationManager::Authenticated:
            stateText = "Authenticated";
            break;
        case AuthenticationManager::Failed:
            stateText = "Failed";
            break;
    }
    m_statusLabel->setText(stateText);
    
    QString typeText;
    switch (m_authManager->authType()) {
        case AuthenticationManager::None:
            typeText = "None";
            break;
        case AuthenticationManager::JWT:
            typeText = "JWT";
            break;
        case AuthenticationManager::Password:
            typeText = "Password";
            break;
        case AuthenticationManager::Guest:
            typeText = "Guest";
            break;
    }
    m_authTypeLabel->setText(typeText);
    
    m_userIdLabel->setText(m_authManager->userId().isEmpty() ? "-" : m_authManager->userId());
}

void AuthenticationDemo::logMessage(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logTextEdit->append(QString("[%1] %2").arg(timestamp, message));
}

QString AuthenticationDemo::createSampleJWTToken()
{
    // 创建示例JWT token用于测试
    QJsonObject header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";
    
    QJsonObject payload;
    payload["sub"] = "demo-user-123";
    payload["name"] = m_displayNameEdit->text();
    payload["iat"] = QDateTime::currentDateTime().toSecsSinceEpoch();
    payload["exp"] = QDateTime::currentDateTime().addSecs(3600).toSecsSinceEpoch();
    payload["room"] = m_roomNameEdit->text();
    
    QJsonDocument headerDoc(header);
    QJsonDocument payloadDoc(payload);
    
    QString headerB64 = headerDoc.toJson(QJsonDocument::Compact).toBase64();
    QString payloadB64 = payloadDoc.toJson(QJsonDocument::Compact).toBase64();
    QString signature = "demo-signature";
    
    // 替换为URL安全字符
    headerB64.replace('+', '-').replace('/', '_').remove('=');
    payloadB64.replace('+', '-').replace('/', '_').remove('=');
    
    return headerB64 + "." + payloadB64 + "." + signature;
}

void AuthenticationDemo::onAuthenticateClicked()
{
    QString serverUrl = m_serverUrlEdit->text().trimmed();
    QString roomName = m_roomNameEdit->text().trimmed();
    QString displayName = m_displayNameEdit->text().trimmed();
    
    if (serverUrl.isEmpty() || roomName.isEmpty() || displayName.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please fill in all required fields");
        return;
    }
    
    logMessage(QString("Starting authentication for room '%1' on server '%2'").arg(roomName, serverUrl));
    m_authManager->authenticate(serverUrl, roomName, displayName);
}

void AuthenticationDemo::onJWTAuthClicked()
{
    QString token = m_jwtTokenEdit->text().trimmed();
    
    if (token.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter a JWT token");
        return;
    }
    
    logMessage("Authenticating with JWT token");
    m_authManager->authenticateWithJWT(token);
}

void AuthenticationDemo::onPasswordAuthClicked()
{
    QString password = m_passwordEdit->text();
    
    if (password.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter a password");
        return;
    }
    
    logMessage("Authenticating with password");
    m_authManager->authenticateWithPassword(password);
}

void AuthenticationDemo::onCheckPermissionsClicked()
{
    QString roomName = m_roomNameEdit->text().trimmed();
    
    if (roomName.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter a room name");
        return;
    }
    
    logMessage(QString("Checking permissions for room '%1'").arg(roomName));
    m_authManager->checkRoomPermissions(roomName);
}

void AuthenticationDemo::onLogoutClicked()
{
    logMessage("Logging out");
    m_authManager->logout();
}

void AuthenticationDemo::onRefreshTokenClicked()
{
    logMessage("Refreshing authentication token");
    m_authManager->refreshAuthToken();
}

void AuthenticationDemo::onAuthStateChanged(AuthenticationManager::AuthState state)
{
    logMessage(QString("Authentication state changed to: %1").arg(static_cast<int>(state)));
    updateUI();
}

void AuthenticationDemo::onAuthenticationSucceeded(AuthenticationManager::AuthType type)
{
    QString typeStr;
    switch (type) {
        case AuthenticationManager::JWT: typeStr = "JWT"; break;
        case AuthenticationManager::Password: typeStr = "Password"; break;
        case AuthenticationManager::Guest: typeStr = "Guest"; break;
        default: typeStr = "Unknown"; break;
    }
    
    logMessage(QString("Authentication succeeded with type: %1").arg(typeStr));
    updateUI();
}

void AuthenticationDemo::onAuthenticationFailed(const QString& error)
{
    logMessage(QString("Authentication failed: %1").arg(error));
    QMessageBox::critical(this, "Authentication Failed", error);
    updateUI();
}

void AuthenticationDemo::onPasswordRequired()
{
    logMessage("Password authentication required");
    QMessageBox::information(this, "Authentication Required", "This room requires password authentication");
}

void AuthenticationDemo::onJWTTokenRequired()
{
    logMessage("JWT token authentication required");
    QMessageBox::information(this, "Authentication Required", "This room requires JWT token authentication");
}

void AuthenticationDemo::onRoomPermissionsUpdated(const AuthenticationManager::RoomPermissions& permissions)
{
    logMessage("Room permissions updated");
    
    QString permissionsText;
    permissionsText += QString("Can Join: %1\n").arg(permissions.canJoin ? "Yes" : "No");
    permissionsText += QString("Is Moderator: %1\n").arg(permissions.isModerator ? "Yes" : "No");
    permissionsText += QString("Can Record: %1\n").arg(permissions.canRecord ? "Yes" : "No");
    permissionsText += QString("Can Live Stream: %1\n").arg(permissions.canLiveStream ? "Yes" : "No");
    permissionsText += QString("Role: %1\n").arg(permissions.role);
    permissionsText += QString("Features: %1").arg(permissions.features.join(", "));
    
    m_permissionsTextEdit->setPlainText(permissionsText);
}

void AuthenticationDemo::onTokenExpiring(int expiresIn)
{
    logMessage(QString("Authentication token expiring in %1 seconds").arg(expiresIn));
    QMessageBox::warning(this, "Token Expiring", 
                        QString("Your authentication token will expire in %1 seconds").arg(expiresIn));
}

void AuthenticationDemo::onTokenExpired()
{
    logMessage("Authentication token has expired");
    QMessageBox::critical(this, "Token Expired", "Your authentication token has expired. Please re-authenticate.");
    updateUI();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    AuthenticationDemo demo;
    demo.show();
    
    return app.exec();
}

#include "authentication_manager_demo.moc"