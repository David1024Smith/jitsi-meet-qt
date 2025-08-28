#pragma once

#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>
#include <memory>
#include <optional>
#include <string_view>

/**
 * @brief Main application class implementing singleton pattern
 * 
 * This class serves as the entry point for the Jitsi Meet Qt application.
 * It ensures only one instance runs at a time and handles protocol URLs.
 * Uses modern C++17 features for improved performance and safety.
 */
class MainApplication : public QApplication {
    Q_OBJECT

public:
    /**
     * @brief Construct the main application
     * @param argc Command line argument count
     * @param argv Command line arguments
     */
    MainApplication(int argc, char *argv[]);
    
    /**
     * @brief Destructor
     */
    ~MainApplication() override;

    /**
     * @brief Get the singleton instance
     * @return Pointer to the application instance, nullptr if not created
     */
    [[nodiscard]] static MainApplication* instance() noexcept;

    /**
     * @brief Handle protocol URL from external source
     * @param url The jitsi-meet:// protocol URL
     */
    void handleProtocolUrl(std::string_view url);

    /**
     * @brief Set the window manager for protocol URL handling
     * @param windowManager Pointer to the window manager
     */
    void setWindowManager(class WindowManager* windowManager);

    /**
     * @brief Get the translation manager
     * @return Pointer to the translation manager
     */
    [[nodiscard]] class TranslationManager* translationManager() const noexcept { return m_translationManager; }

    /**
     * @brief Check if this is the primary instance
     * @return true if this is the primary instance, false otherwise
     */
    [[nodiscard]] bool isPrimaryInstance() const noexcept { return m_isPrimaryInstance; }

    /**
     * @brief Get the application title
     * @return Application title string
     */
    [[nodiscard]] static constexpr std::string_view applicationTitle() noexcept {
        return "Jitsi Meet";
    }

    /**
     * @brief Get the minimum window size
     * @return Minimum window size as QSize
     */
    [[nodiscard]] static constexpr QSize minimumWindowSize() noexcept {
        return QSize(800, 600);
    }

signals:
    /**
     * @brief Emitted when a protocol URL is received
     * @param url The protocol URL
     */
    void protocolUrlReceived(const QString& url);

    /**
     * @brief Emitted when a second instance attempts to start
     * @param arguments Command line arguments from the second instance
     */
    void secondInstanceDetected(const QString& arguments);

private slots:
    /**
     * @brief Handle new connection from second instance
     */
    void onNewConnection();

    /**
     * @brief Handle data from second instance
     */
    void onSecondInstanceData();

    /**
     * @brief Handle protocol URL received from ProtocolHandler
     * @param url The protocol URL
     */
    void onProtocolUrlReceived(const QString& url);

private:
    /**
     * @brief Setup single instance mechanism
     * @return true if setup successful, false otherwise
     */
    bool setupSingleInstance();

    /**
     * @brief Initialize application components
     */
    void initializeApplication();

    /**
     * @brief Initialize all manager components
     */
    void initializeManagers();

    /**
     * @brief Setup component connections
     */
    void setupComponentConnections();

    /**
     * @brief Initialize user interface
     */
    void initializeUserInterface();

    /**
     * @brief Initialize translation manager
     */
    void initializeTranslationManager();

    /**
     * @brief Initialize protocol handler
     */
    void initializeProtocolHandler();

    /**
     * @brief Parse command line arguments
     * @return Optional protocol URL if found
     */
    [[nodiscard]] std::optional<QString> parseCommandLineArguments() const;

    /**
     * @brief Validate protocol URL format
     * @param url URL to validate
     * @return true if valid jitsi-meet:// URL
     */
    [[nodiscard]] static bool isValidProtocolUrl(std::string_view url) noexcept;

    /**
     * @brief Send message to primary instance
     * @param message Message to send
     * @return true if message sent successfully
     */
    bool sendToPrimaryInstance(const QString& message);

    // Static instance pointer for singleton pattern
    static MainApplication* s_instance;

    // Single instance management
    std::unique_ptr<QLocalServer> m_localServer;
    std::unique_ptr<QLocalSocket> m_localSocket;
    bool m_isPrimaryInstance{false};
    
    // Application components
    class ProtocolHandler* m_protocolHandler;
    class WindowManager* m_windowManager;
    class TranslationManager* m_translationManager;
    class ConfigurationManager* m_configurationManager;
    class ConferenceManager* m_conferenceManager;
    class MediaManager* m_mediaManager;
    class ChatManager* m_chatManager;
    class ScreenShareManager* m_screenShareManager;
    class AuthenticationManager* m_authenticationManager;
    class ErrorRecoveryManager* m_errorRecoveryManager;
    class ThemeManager* m_themeManager;
    class PerformanceManager* m_performanceManager;
    
    // Application state
    QString m_serverName;
    static constexpr std::string_view SERVER_NAME = "JitsiMeetQt_SingleInstance";
    static constexpr int CONNECTION_TIMEOUT_MS = 1000;
};