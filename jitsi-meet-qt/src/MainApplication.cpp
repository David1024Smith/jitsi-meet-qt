#include "MainApplication.h"
#include "ProtocolHandler.h"
#include "WindowManager.h"
#include "TranslationManager.h"
#include "ConfigurationManager.h"
#include "ConferenceManager.h"
#include "MediaManager.h"
#include "ChatManager.h"
#include "ScreenShareManager.h"
#include "AuthenticationManager.h"
#include "ErrorRecoveryManager.h"
#include "ThemeManager.h"
#include "PerformanceManager.h"
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <algorithm>
#include <string>

// Static member initialization
MainApplication* MainApplication::s_instance = nullptr;

MainApplication::MainApplication(int argc, char *argv[])
    : QApplication(argc, argv)
    , m_serverName(QString::fromUtf8(SERVER_NAME.data(), static_cast<int>(SERVER_NAME.size())))
    , m_protocolHandler(nullptr)
    , m_windowManager(nullptr)
    , m_translationManager(nullptr)
    , m_configurationManager(nullptr)
    , m_conferenceManager(nullptr)
    , m_mediaManager(nullptr)
    , m_chatManager(nullptr)
    , m_screenShareManager(nullptr)
    , m_authenticationManager(nullptr)
    , m_errorRecoveryManager(nullptr)
    , m_themeManager(nullptr)
    , m_performanceManager(nullptr)
{
    // Set singleton instance
    s_instance = this;
    
    // Set application properties
    setApplicationName(QString::fromUtf8(applicationTitle().data(), 
                                       static_cast<int>(applicationTitle().size())));
    setApplicationVersion("1.0.0");
    setOrganizationName("Jitsi Meet Qt");
    setOrganizationDomain("jitsi.org");
    
    // Setup single instance mechanism
    if (!setupSingleInstance()) {
        // This is a second instance, try to communicate with primary
        if (auto protocolUrl = parseCommandLineArguments(); protocolUrl.has_value()) {
            sendToPrimaryInstance(protocolUrl.value());
        } else {
            sendToPrimaryInstance("activate");
        }
        
        // Exit second instance
        QTimer::singleShot(100, this, &QApplication::quit);
        return;
    }
    
    // Initialize primary instance
    initializeApplication();
    
    // Handle command line arguments for primary instance
    if (auto protocolUrl = parseCommandLineArguments(); protocolUrl.has_value()) {
        QTimer::singleShot(100, [this, url = protocolUrl.value()]() {
            handleProtocolUrl(url.toStdString());
        });
    }
}

MainApplication::~MainApplication() {
    qDebug() << "MainApplication destructor called";
    
    if (m_localServer) {
        m_localServer->close();
        QLocalServer::removeServer(m_serverName);
    }
    
    // Clean up managers in reverse order of creation
    if (m_performanceManager) {
        delete m_performanceManager;
        m_performanceManager = nullptr;
    }
    
    if (m_themeManager) {
        delete m_themeManager;
        m_themeManager = nullptr;
    }
    
    if (m_errorRecoveryManager) {
        delete m_errorRecoveryManager;
        m_errorRecoveryManager = nullptr;
    }
    
    if (m_authenticationManager) {
        delete m_authenticationManager;
        m_authenticationManager = nullptr;
    }
    
    if (m_screenShareManager) {
        delete m_screenShareManager;
        m_screenShareManager = nullptr;
    }
    
    if (m_chatManager) {
        delete m_chatManager;
        m_chatManager = nullptr;
    }
    
    if (m_mediaManager) {
        delete m_mediaManager;
        m_mediaManager = nullptr;
    }
    
    if (m_conferenceManager) {
        delete m_conferenceManager;
        m_conferenceManager = nullptr;
    }
    
    if (m_windowManager) {
        delete m_windowManager;
        m_windowManager = nullptr;
    }
    
    if (m_configurationManager) {
        delete m_configurationManager;
        m_configurationManager = nullptr;
    }
    
    if (m_translationManager) {
        delete m_translationManager;
        m_translationManager = nullptr;
    }
    
    if (m_protocolHandler) {
        m_protocolHandler->unregisterProtocol();
        delete m_protocolHandler;
        m_protocolHandler = nullptr;
    }
    
    s_instance = nullptr;
    qDebug() << "MainApplication destroyed";
}

MainApplication* MainApplication::instance() noexcept {
    return s_instance;
}

void MainApplication::handleProtocolUrl(std::string_view url) {
    if (!isValidProtocolUrl(url)) {
        qWarning() << "Invalid protocol URL:" << QString::fromUtf8(url.data(), 
                                                                  static_cast<int>(url.size()));
        return;
    }
    
    const QString qUrl = QString::fromUtf8(url.data(), static_cast<int>(url.size()));
    emit protocolUrlReceived(qUrl);
}

bool MainApplication::setupSingleInstance() {
    // Try to connect to existing instance first
    m_localSocket = std::make_unique<QLocalSocket>();
    m_localSocket->connectToServer(m_serverName);
    
    if (m_localSocket->waitForConnected(CONNECTION_TIMEOUT_MS)) {
        // Another instance is already running
        m_isPrimaryInstance = false;
        return false;
    }
    
    // No existing instance, create server
    m_localServer = std::make_unique<QLocalServer>();
    
    // Remove any stale server
    QLocalServer::removeServer(m_serverName);
    
    if (!m_localServer->listen(m_serverName)) {
        qCritical() << "Failed to create local server:" << m_localServer->errorString();
        return false;
    }
    
    // Connect server signals
    connect(m_localServer.get(), &QLocalServer::newConnection,
            this, &MainApplication::onNewConnection);
    
    m_isPrimaryInstance = true;
    return true;
}

void MainApplication::initializeApplication() {
    qDebug() << "Initializing Jitsi Meet Qt application...";
    
    // Set application icon and window properties
    setQuitOnLastWindowClosed(true);
    
    // High DPI scaling is enabled by default in Qt 6
    
    // Initialize all manager components
    initializeManagers();
    
    // Setup component connections
    setupComponentConnections();
    
    // Initialize user interface
    initializeUserInterface();
    
    qDebug() << "Jitsi Meet Qt application initialized successfully";
    qDebug() << "Qt version:" << qVersion();
    qDebug() << "C++ standard:" << __cplusplus;
}

std::optional<QString> MainApplication::parseCommandLineArguments() const {
    QCommandLineParser parser;
    parser.setApplicationDescription("Jitsi Meet Qt Desktop Application");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Add protocol URL option
    QCommandLineOption urlOption(QStringList() << "u" << "url",
                                "Join conference from URL",
                                "url");
    parser.addOption(urlOption);
    
    // Process arguments
    parser.process(*this);
    
    // Check for protocol URL
    if (parser.isSet(urlOption)) {
        return parser.value(urlOption);
    }
    
    // Check positional arguments for protocol URL
    const auto args = parser.positionalArguments();
    for (const auto& arg : args) {
        if (isValidProtocolUrl(arg.toStdString())) {
            return arg;
        }
    }
    
    return std::nullopt;
}

bool MainApplication::isValidProtocolUrl(std::string_view url) noexcept {
    constexpr std::string_view PROTOCOL_PREFIX = "jitsi-meet://";
    
    if (url.size() <= PROTOCOL_PREFIX.size()) {
        return false;
    }
    
    return url.substr(0, PROTOCOL_PREFIX.size()) == PROTOCOL_PREFIX;
}

bool MainApplication::sendToPrimaryInstance(const QString& message) {
    if (!m_localSocket || m_localSocket->state() != QLocalSocket::ConnectedState) {
        return false;
    }
    
    const QByteArray data = message.toUtf8();
    const qint64 written = m_localSocket->write(data);
    
    if (written != data.size()) {
        qWarning() << "Failed to send complete message to primary instance";
        return false;
    }
    
    return m_localSocket->waitForBytesWritten(CONNECTION_TIMEOUT_MS);
}

void MainApplication::onNewConnection() {
    if (!m_localServer) {
        return;
    }
    
    auto* socket = m_localServer->nextPendingConnection();
    if (!socket) {
        return;
    }
    
    connect(socket, &QLocalSocket::readyRead, [this, socket]() {
        const QByteArray data = socket->readAll();
        const QString message = QString::fromUtf8(data);
        
        if (message == "activate") {
            emit secondInstanceDetected(QString());
        } else if (isValidProtocolUrl(message.toStdString())) {
            emit protocolUrlReceived(message);
        }
        
        socket->disconnectFromServer();
    });
    
    connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
}

void MainApplication::onSecondInstanceData() {
    auto* socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) {
        return;
    }
    
    const QByteArray data = socket->readAll();
    const QString message = QString::fromUtf8(data);
    
    emit secondInstanceDetected(message);
}

void MainApplication::initializeProtocolHandler() {
    if (!m_protocolHandler) {
        m_protocolHandler = new ProtocolHandler(this);
        
        // Connect protocol handler signals
        connect(m_protocolHandler, &ProtocolHandler::protocolUrlReceived,
                this, &MainApplication::onProtocolUrlReceived);
        
        // Register the protocol
        bool registered = m_protocolHandler->registerProtocol();
        if (registered) {
            qDebug() << "Protocol handler registered successfully";
        } else {
            qWarning() << "Failed to register protocol handler";
        }
    }
}

void MainApplication::setWindowManager(WindowManager* windowManager) {
    if (m_windowManager == windowManager) {
        return; // Already set
    }
    
    m_windowManager = windowManager;
    
    if (m_windowManager) {
        qDebug() << "WindowManager set in MainApplication";
    }
}

void MainApplication::onProtocolUrlReceived(const QString& url) {
    qDebug() << "Protocol URL received:" << url;
    
    if (m_protocolHandler) {
        QString parsedUrl = m_protocolHandler->parseProtocolUrl(url);
        if (!parsedUrl.isEmpty()) {
            emit protocolUrlReceived(parsedUrl);
        } else {
            qWarning() << "Failed to parse protocol URL:" << url;
        }
    }
}

void MainApplication::initializeTranslationManager() {
    // Create translation manager
    m_translationManager = new TranslationManager(this);
    
    // Initialize translation system
    if (m_translationManager->initialize()) {
        qDebug() << "TranslationManager initialized successfully";
        qDebug() << "Current language:" << m_translationManager->currentLanguageCode();
        qDebug() << "System language:" << static_cast<int>(m_translationManager->systemLanguage());
        
        // Log available languages
        auto availableLanguages = m_translationManager->availableLanguages();
        qDebug() << "Available languages:" << availableLanguages.size();
        for (const auto& langInfo : availableLanguages) {
            qDebug() << "  -" << langInfo.code << "(" << langInfo.nativeName << ")";
        }
    } else {
        qWarning() << "Failed to initialize TranslationManager";
    }
}

void MainApplication::initializeManagers() {
    qDebug() << "Initializing manager components...";
    
    // 1. Initialize configuration manager first (other components depend on it)
    m_configurationManager = new ConfigurationManager(this);
    qDebug() << "ConfigurationManager initialized";
    
    // 2. Initialize translation manager
    initializeTranslationManager();
    
    // 3. Initialize theme manager
    m_themeManager = new ThemeManager(this);
    qDebug() << "ThemeManager initialized";
    
    // 4. Initialize performance manager
    m_performanceManager = new PerformanceManager(this);
    qDebug() << "PerformanceManager initialized";
    
    // 5. Initialize error recovery manager
    m_errorRecoveryManager = new ErrorRecoveryManager(this);
    qDebug() << "ErrorRecoveryManager initialized";
    
    // 6. Initialize authentication manager
    m_authenticationManager = new AuthenticationManager(this);
    qDebug() << "AuthenticationManager initialized";
    
    // 7. Initialize media manager
    m_mediaManager = new MediaManager(this);
    qDebug() << "MediaManager initialized";
    
    // 8. Initialize conference manager
    m_conferenceManager = new ConferenceManager(this);
    qDebug() << "ConferenceManager initialized";
    
    // 9. Initialize chat manager
    m_chatManager = new ChatManager(this);
    if (m_conferenceManager && m_conferenceManager->xmppClient()) {
        m_chatManager->setXMPPClient(m_conferenceManager->xmppClient());
    }
    qDebug() << "ChatManager initialized";
    
    // 10. Initialize screen share manager
    m_screenShareManager = new ScreenShareManager(this);
    qDebug() << "ScreenShareManager initialized";
    
    // 11. Initialize window manager
    m_windowManager = new WindowManager(this);
    if (m_configurationManager) {
        m_windowManager->setConfigurationManager(m_configurationManager);
    }
    if (m_translationManager) {
        m_windowManager->setTranslationManager(m_translationManager);
    }
    qDebug() << "WindowManager initialized";
    
    // 12. Initialize protocol handler
    initializeProtocolHandler();
    
    qDebug() << "All manager components initialized successfully";
}

void MainApplication::setupComponentConnections() {
    qDebug() << "Setting up component connections...";
    
    if (!m_windowManager || !m_configurationManager || !m_conferenceManager) {
        qWarning() << "Cannot setup connections: required components not initialized";
        return;
    }
    
    // Connect protocol URL handling to window manager
    connect(this, &MainApplication::protocolUrlReceived,
            m_windowManager, &WindowManager::onJoinConference);
    
    // Connect second instance detection to window manager
    connect(this, &MainApplication::secondInstanceDetected,
            [this](const QString& arguments) {
                qDebug() << "Second instance detected, bringing window to front";
                if (m_windowManager) {
                    // Show welcome window and bring to front
                    m_windowManager->showWindow(WindowManager::WelcomeWindow);
                    if (auto* currentWindow = m_windowManager->currentWindow()) {
                        currentWindow->raise();
                        currentWindow->activateWindow();
                    }
                }
            });
    
    // Connect conference manager to other components
    if (m_conferenceManager && m_mediaManager) {
        // Connect media manager to conference manager
        connect(m_mediaManager, &MediaManager::localVideoStarted,
                m_conferenceManager, [this]() {
                    qDebug() << "Local video started, updating conference state";
                });
        
        connect(m_mediaManager, &MediaManager::localAudioStarted,
                m_conferenceManager, [this]() {
                    qDebug() << "Local audio started, updating conference state";
                });
    }
    
    // Connect chat manager to conference manager
    if (m_chatManager && m_conferenceManager) {
        connect(m_conferenceManager, &ConferenceManager::conferenceJoined,
                m_chatManager, [this](const ConferenceManager::ConferenceInfo& info) {
                    qDebug() << "Conference joined, chat manager ready";
                });
    }
    
    // Connect screen share manager to conference manager
    if (m_screenShareManager && m_conferenceManager) {
        connect(m_conferenceManager, &ConferenceManager::conferenceJoined,
                m_screenShareManager, [this](const ConferenceManager::ConferenceInfo& info) {
                    qDebug() << "Conference joined, screen share manager ready";
                });
    }
    
    // Connect error recovery manager to all components
    if (m_errorRecoveryManager) {
        if (m_conferenceManager) {
            connect(m_conferenceManager, &ConferenceManager::errorOccurred,
                    m_errorRecoveryManager, &ErrorRecoveryManager::handleError);
        }
        if (m_mediaManager) {
            // Media manager error handling will be connected when available
        }
    }
    
    // Connect configuration changes to relevant components
    if (m_configurationManager) {
        connect(m_configurationManager, &ConfigurationManager::languageChanged,
                [this](const QString& language) {
                    qDebug() << "Language changed to:" << language;
                    if (m_translationManager) {
                        m_translationManager->setLanguage(language);
                    }
                });
        
        connect(m_configurationManager, &ConfigurationManager::darkModeChanged,
                [this](bool darkMode) {
                    qDebug() << "Dark mode changed to:" << darkMode;
                    if (m_themeManager) {
                        auto theme = darkMode ? ThemeManager::Theme::Dark : ThemeManager::Theme::Light;
                        m_themeManager->setTheme(theme);
                    }
                });
    }
    
    qDebug() << "Component connections setup completed";
}

void MainApplication::initializeUserInterface() {
    qDebug() << "Initializing user interface...";
    
    if (!m_windowManager) {
        qWarning() << "Cannot initialize UI: WindowManager not available";
        return;
    }
    
    // Set window manager in main application for protocol handling
    setWindowManager(m_windowManager);
    
    // Apply theme settings
    if (m_themeManager && m_configurationManager) {
        bool darkMode = m_configurationManager->isDarkMode();
        auto theme = darkMode ? ThemeManager::Theme::Dark : ThemeManager::Theme::Light;
        m_themeManager->setTheme(theme);
        qDebug() << "Applied theme settings, dark mode:" << darkMode;
    }
    
    // Show welcome window as the initial interface
    QTimer::singleShot(100, [this]() {
        if (m_windowManager) {
            m_windowManager->showWindow(WindowManager::WelcomeWindow);
            qDebug() << "Welcome window displayed";
        }
    });
    
    qDebug() << "User interface initialized";
}

