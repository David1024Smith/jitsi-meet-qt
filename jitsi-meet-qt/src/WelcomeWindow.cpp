#include "WelcomeWindow.h"
#include "NavigationBar.h"
#include "RecentListWidget.h"
#include "ConfigurationManager.h"
#include "models/RecentItem.h"
#include "JitsiConstants.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QRandomGenerator>
#include <QUrl>
#include <QRegularExpression>
#include <QMessageBox>
#include <QFont>
#include <QSizePolicy>
#include <QSpacerItem>

WelcomeWindow::WelcomeWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_navigationBar(nullptr)
    , m_contentWidget(nullptr)
    , m_contentLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_descriptionLabel(nullptr)
    , m_inputWidget(nullptr)
    , m_inputLayout(nullptr)
    , m_urlLineEdit(nullptr)
    , m_joinButton(nullptr)
    , m_errorLabel(nullptr)
    , m_recentLabel(nullptr)
    , m_recentList(nullptr)
    , m_roomNameTimer(nullptr)
    , m_animationTimer(nullptr)
    , m_animationIndex(0)
    , m_isUserTyping(false)
    , m_animationActive(false)
    , m_placeholderEffect(nullptr)
    , m_placeholderAnimation(nullptr)
    , m_configManager(nullptr)
{
    setupUI();
    setupConnections();
    setupRandomRoomNames();
    startRoomNameAnimation();
}

WelcomeWindow::~WelcomeWindow()
{
    stopRoomNameAnimation();
}

void WelcomeWindow::setupUI()
{
    // Set window properties
    setWindowTitle(tr("welcome_title"));
    setMinimumSize(800, 600);
    resize(1000, 700);
    
    // Create central widget
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // Create main layout
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create navigation bar
    m_navigationBar = new NavigationBar(this);
    m_navigationBar->setButtonConfiguration(NavigationBar::SettingsButton | NavigationBar::AboutButton);
    m_navigationBar->setTitle(tr("welcome_title"));
    m_mainLayout->addWidget(m_navigationBar);
    
    // Create content widget
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(40, 40, 40, 40);
    m_contentLayout->setSpacing(30);
    
    // Add spacer at top
    m_contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    
    // Create title and description
    m_titleLabel = new QLabel(tr("welcome_subtitle"));
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_contentLayout->addWidget(m_titleLabel);
    
    m_descriptionLabel = new QLabel(tr("welcome_subtitle"));
    QFont descFont = m_descriptionLabel->font();
    descFont.setPointSize(12);
    m_descriptionLabel->setFont(descFont);
    m_descriptionLabel->setAlignment(Qt::AlignCenter);
    m_descriptionLabel->setStyleSheet("color: #666666;");
    m_contentLayout->addWidget(m_descriptionLabel);
    
    // Create input section
    m_inputWidget = new QWidget();
    m_inputLayout = new QVBoxLayout(m_inputWidget);
    m_inputLayout->setContentsMargins(0, 0, 0, 0);
    m_inputLayout->setSpacing(10);
    
    // URL input field
    m_urlLineEdit = new QLineEdit();
    m_urlLineEdit->setPlaceholderText(tr("room_name_placeholder"));
    m_urlLineEdit->setMinimumHeight(50);
    QFont inputFont = m_urlLineEdit->font();
    inputFont.setPointSize(14);
    m_urlLineEdit->setFont(inputFont);
    m_inputLayout->addWidget(m_urlLineEdit);
    
    // Join button
    m_joinButton = new QPushButton(tr("join_button"));
    m_joinButton->setMinimumHeight(50);
    m_joinButton->setEnabled(false);
    QFont buttonFont = m_joinButton->font();
    buttonFont.setPointSize(14);
    buttonFont.setBold(true);
    m_joinButton->setFont(buttonFont);
    m_inputLayout->addWidget(m_joinButton);
    
    // Error label
    m_errorLabel = new QLabel();
    m_errorLabel->setStyleSheet("color: #d32f2f; font-weight: bold;");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->hide();
    m_inputLayout->addWidget(m_errorLabel);
    
    m_contentLayout->addWidget(m_inputWidget);
    
    // Add some spacing
    m_contentLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));
    
    // Recent meetings section
    m_recentLabel = new QLabel(tr("recent_meetings"));
    QFont recentFont = m_recentLabel->font();
    recentFont.setPointSize(16);
    recentFont.setBold(true);
    m_recentLabel->setFont(recentFont);
    m_contentLayout->addWidget(m_recentLabel);
    
    // Recent list widget
    m_recentList = new RecentListWidget();
    m_recentList->setMaxItems(5);
    m_recentList->setMaximumHeight(200);
    m_contentLayout->addWidget(m_recentList);
    
    // Add spacer at bottom
    m_contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    
    m_mainLayout->addWidget(m_contentWidget);
    
    applyStyles();
}

void WelcomeWindow::setupConnections()
{
    // Navigation bar connections
    connect(m_navigationBar, &NavigationBar::settingsClicked, this, &WelcomeWindow::onSettingsClicked);
    connect(m_navigationBar, &NavigationBar::aboutClicked, this, &WelcomeWindow::onAboutClicked);
    
    // Input connections
    connect(m_urlLineEdit, &QLineEdit::textChanged, this, &WelcomeWindow::onUrlChanged);
    connect(m_urlLineEdit, &QLineEdit::returnPressed, this, &WelcomeWindow::onJoinButtonClicked);
    connect(m_joinButton, &QPushButton::clicked, this, &WelcomeWindow::onJoinButtonClicked);
    
    // Recent list connections
    connect(m_recentList, &RecentListWidget::itemClicked, this, &WelcomeWindow::onRecentItemClicked);
    connect(m_recentList, &RecentListWidget::itemDoubleClicked, this, &WelcomeWindow::onJoinButtonClicked);
    
    // Animation timers
    m_roomNameTimer = new QTimer(this);
    connect(m_roomNameTimer, &QTimer::timeout, this, &WelcomeWindow::updateRandomRoomName);
    
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &WelcomeWindow::animateRoomNameTyping);
}

void WelcomeWindow::setupRandomRoomNames()
{
    // Generate a list of random room names
    QStringList adjectives = {
        tr("Amazing"), tr("Brilliant"), tr("Creative"), tr("Dynamic"), tr("Excellent"),
        tr("Fantastic"), tr("Great"), tr("Happy"), tr("Innovative"), tr("Joyful"),
        tr("Kind"), tr("Lively"), tr("Magnificent"), tr("Nice"), tr("Outstanding"),
        tr("Perfect"), tr("Quick"), tr("Remarkable"), tr("Super"), tr("Terrific"),
        tr("Unique"), tr("Vibrant"), tr("Wonderful"), tr("Exciting"), tr("Youthful"),
        tr("Zealous")
    };
    
    QStringList nouns = {
        tr("Meeting"), tr("Conference"), tr("Discussion"), tr("Session"), tr("Gathering"),
        tr("Workshop"), tr("Seminar"), tr("Presentation"), tr("Briefing"), tr("Summit"),
        tr("Forum"), tr("Assembly"), tr("Consultation"), tr("Dialogue"), tr("Exchange"),
        tr("Interview"), tr("Negotiation"), tr("Review"), tr("Symposium"), tr("Talk"),
        tr("Webinar"), tr("Collaboration"), tr("Brainstorm"), tr("Huddle"), tr("Standup")
    };
    
    // Generate combinations
    for (const QString& adj : adjectives) {
        for (const QString& noun : nouns) {
            m_randomRoomNames.append(QString("%1%2").arg(adj).arg(noun));
        }
    }
}

void WelcomeWindow::startRoomNameAnimation()
{
    if (!m_animationActive) {
        m_animationActive = true;
        updateRandomRoomName();
        m_roomNameTimer->start(ROOM_NAME_UPDATE_INTERVAL);
    }
}

void WelcomeWindow::stopRoomNameAnimation()
{
    if (m_animationActive) {
        m_animationActive = false;
        m_roomNameTimer->stop();
        m_animationTimer->stop();
    }
}

void WelcomeWindow::updateRandomRoomName()
{
    if (m_isUserTyping || m_urlLineEdit->text().length() > 0) {
        return;
    }
    
    m_currentRoomName = generateRandomRoomName();
    showTypingAnimation(m_currentRoomName);
}

void WelcomeWindow::showTypingAnimation(const QString& roomName)
{
    m_typingText = "";
    m_animationIndex = 0;
    m_currentRoomName = roomName;
    
    m_animationTimer->start(TYPING_ANIMATION_SPEED);
}

void WelcomeWindow::animateRoomNameTyping()
{
    if (m_animationIndex < m_currentRoomName.length()) {
        m_typingText += m_currentRoomName.at(m_animationIndex);
        m_urlLineEdit->setPlaceholderText(m_typingText);
        m_animationIndex++;
    } else {
        m_animationTimer->stop();
        // Keep the full room name for a while before next update
    }
}

QString WelcomeWindow::generateRandomRoomName()
{
    if (m_randomRoomNames.isEmpty()) {
        return tr("MyMeeting");
    }
    
    int index = QRandomGenerator::global()->bounded(m_randomRoomNames.size());
    return m_randomRoomNames.at(index);
}

void WelcomeWindow::onJoinButtonClicked()
{
    QString url = m_urlLineEdit->text().trimmed();
    
    if (url.isEmpty()) {
        // Use the current placeholder text as room name
        url = m_urlLineEdit->placeholderText();
    }
    
    if (isValidUrl(url)) {
        clearError();
        
        // Add to recent items before joining
        addToRecentItems(url);
        
        emit joinConference(url);
    } else {
        showError(tr("invalid_url_error"));
    }
}

void WelcomeWindow::onUrlChanged(const QString& text)
{
    m_isUserTyping = !text.isEmpty();
    
    if (m_isUserTyping) {
        stopRoomNameAnimation();
    } else {
        startRoomNameAnimation();
    }
    
    validateInput();
    clearError();
}

void WelcomeWindow::onRecentItemClicked(const QString& url)
{
    setUrlText(url);
}

void WelcomeWindow::onSettingsClicked()
{
    emit settingsRequested();
}

void WelcomeWindow::onAboutClicked()
{
    emit aboutRequested();
}

void WelcomeWindow::onRecentItemsChanged()
{
    loadRecentItems();
}

void WelcomeWindow::validateInput()
{
    updateJoinButtonState();
}

void WelcomeWindow::updateJoinButtonState()
{
    QString text = m_urlLineEdit->text().trimmed();
    bool hasText = !text.isEmpty();
    bool hasPlaceholder = !m_urlLineEdit->placeholderText().isEmpty();
    
    // Enable button if there's text or a placeholder (random room name)
    m_joinButton->setEnabled(hasText || hasPlaceholder);
}

bool WelcomeWindow::isValidUrl(const QString& url) const
{
    if (url.length() < MIN_URL_LENGTH) {
        return false;
    }
    
    // Check if it's a full URL
    QUrl qurl(url);
    if (qurl.isValid() && !qurl.scheme().isEmpty()) {
        return qurl.scheme() == "http" || qurl.scheme() == "https";
    }
    
    // Check if it's a valid room name (alphanumeric with some special chars)
    QRegularExpression roomNameRegex("^[a-zA-Z0-9._-]+$");
    return roomNameRegex.match(url).hasMatch();
}

void WelcomeWindow::setUrlText(const QString& url)
{
    m_urlLineEdit->setText(url);
    m_urlLineEdit->setFocus();
}

void WelcomeWindow::showError(const QString& message)
{
    m_errorLabel->setText(message);
    m_errorLabel->show();
}

void WelcomeWindow::clearError()
{
    m_errorLabel->hide();
    m_errorLabel->clear();
}

QString WelcomeWindow::getUrlText() const
{
    QString text = m_urlLineEdit->text().trimmed();
    if (text.isEmpty()) {
        return m_urlLineEdit->placeholderText();
    }
    return text;
}

void WelcomeWindow::applyStyles()
{
    // Apply modern styling
    setStyleSheet(R"(
        WelcomeWindow {
            background-color: #f5f5f5;
        }
        
        QLineEdit {
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 12px 16px;
            font-size: 14px;
            background-color: white;
        }
        
        QLineEdit:focus {
            border-color: #1976d2;
            outline: none;
        }
        
        QPushButton {
            background-color: #1976d2;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        
        QPushButton:hover {
            background-color: #1565c0;
        }
        
        QPushButton:pressed {
            background-color: #0d47a1;
        }
        
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
        }
        
        QLabel {
            color: #333333;
        }
    )");
}

void WelcomeWindow::setConfigurationManager(ConfigurationManager* configManager)
{
    if (m_configManager) {
        // Disconnect from previous configuration manager
        disconnect(m_configManager, &ConfigurationManager::recentItemsChanged,
                  this, &WelcomeWindow::onRecentItemsChanged);
    }
    
    m_configManager = configManager;
    
    if (m_configManager) {
        // Connect to configuration manager signals
        connect(m_configManager, &ConfigurationManager::recentItemsChanged,
                this, &WelcomeWindow::onRecentItemsChanged);
        
        // Load recent items
        loadRecentItems();
        
        // Set max items from configuration
        m_recentList->setMaxItems(m_configManager->maxRecentItems());
    }
}

void WelcomeWindow::loadRecentItems()
{
    if (!m_configManager || !m_recentList) {
        return;
    }
    
    QList<RecentItem> items = m_configManager->recentItems();
    m_recentList->setRecentItems(items);
}

void WelcomeWindow::addToRecentItems(const QString& url)
{
    if (!m_configManager || url.isEmpty()) {
        return;
    }
    
    // Create a RecentItem from the URL
    QString fullUrl = url;
    
    // If it's not a full URL, construct one with the default server
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        QString serverUrl = m_configManager->serverUrl();
        if (!serverUrl.endsWith("/")) {
            serverUrl += "/";
        }
        fullUrl = serverUrl + url;
    }
    
    RecentItem item(fullUrl);
    m_configManager->addRecentItem(item);
}

void WelcomeWindow::retranslateUi()
{
    // Update window title
    setWindowTitle(tr("welcome_title"));
    
    // Update navigation bar
    if (m_navigationBar) {
        m_navigationBar->setTitle(tr("welcome_title"));
        m_navigationBar->retranslateUi();
    }
    
    // Update labels
    if (m_titleLabel) {
        m_titleLabel->setText(tr("welcome_subtitle"));
    }
    
    if (m_descriptionLabel) {
        m_descriptionLabel->setText(tr("welcome_subtitle"));
    }
    
    if (m_recentLabel) {
        m_recentLabel->setText(tr("recent_meetings"));
    }
    
    // Update input field
    if (m_urlLineEdit) {
        m_urlLineEdit->setPlaceholderText(tr("room_name_placeholder"));
    }
    
    // Update button
    if (m_joinButton) {
        m_joinButton->setText(tr("join_button"));
    }
    
    // Update recent list widget
    if (m_recentList) {
        m_recentList->retranslateUi();
    }
    
    // Regenerate random room names with new translations
    setupRandomRoomNames();
}