#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QStringList>
#include <QRandomGenerator>

class NavigationBar;
class RecentListWidget;
class ConfigurationManager;

/**
 * @brief Main welcome window for the Jitsi Meet Qt application
 * 
 * This window provides the main interface for users to:
 * - Enter meeting room names or URLs
 * - Join meetings
 * - View recent meetings
 * - Access settings and other features
 * - See animated random room name suggestions
 */
class WelcomeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit WelcomeWindow(QWidget *parent = nullptr);
    ~WelcomeWindow();
    
    /**
     * @brief Set the URL text in the input field
     */
    void setUrlText(const QString& url);
    
    /**
     * @brief Show an error message to the user
     */
    void showError(const QString& message);
    
    /**
     * @brief Clear any error messages
     */
    void clearError();
    
    /**
     * @brief Get the current URL text
     */
    QString getUrlText() const;
    
    /**
     * @brief Set the configuration manager for recent items
     */
    void setConfigurationManager(ConfigurationManager* configManager);
    
    /**
     * @brief Load recent items from configuration
     */
    void loadRecentItems();
    
    /**
     * @brief Add a URL to recent items
     */
    void addToRecentItems(const QString& url);

public slots:
    /**
     * @brief Handle language change events
     */
    void retranslateUi();

signals:
    /**
     * @brief Emitted when user wants to join a conference
     */
    void joinConference(const QString& url);
    
    /**
     * @brief Emitted when user requests settings
     */
    void settingsRequested();
    
    /**
     * @brief Emitted when user requests about dialog
     */
    void aboutRequested();

private slots:
    void onJoinButtonClicked();
    void onUrlChanged(const QString& text);
    void onRecentItemClicked(const QString& url);
    void updateRandomRoomName();
    void animateRoomNameTyping();
    void onSettingsClicked();
    void onAboutClicked();
    void onRecentItemsChanged();

private:
    void setupUI();
    void setupConnections();
    void setupRandomRoomNames();
    void startRoomNameAnimation();
    void stopRoomNameAnimation();
    void validateInput();
    void updateJoinButtonState();
    QString generateRandomRoomName();
    bool isValidUrl(const QString& url) const;
    void showTypingAnimation(const QString& roomName);
    void applyStyles();
    
    // UI Components
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    NavigationBar* m_navigationBar;
    
    // Main content area
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    
    // Title and description
    QLabel* m_titleLabel;
    QLabel* m_descriptionLabel;
    
    // URL input section
    QWidget* m_inputWidget;
    QVBoxLayout* m_inputLayout;
    QLineEdit* m_urlLineEdit;
    QPushButton* m_joinButton;
    QLabel* m_errorLabel;
    
    // Recent meetings section
    QLabel* m_recentLabel;
    RecentListWidget* m_recentList;
    
    // Animation and random room names
    QTimer* m_roomNameTimer;
    QTimer* m_animationTimer;
    QStringList m_randomRoomNames;
    QString m_currentRoomName;
    QString m_typingText;
    int m_animationIndex;
    bool m_isUserTyping;
    bool m_animationActive;
    
    // Animation effects
    QGraphicsOpacityEffect* m_placeholderEffect;
    QPropertyAnimation* m_placeholderAnimation;
    
    // Configuration
    ConfigurationManager* m_configManager;
    
    // Constants
    static const int ROOM_NAME_UPDATE_INTERVAL = 10000; // 10 seconds
    static const int TYPING_ANIMATION_SPEED = 100; // milliseconds per character
    static const int MIN_URL_LENGTH = 3;
};

#endif // WELCOMEWINDOW_H