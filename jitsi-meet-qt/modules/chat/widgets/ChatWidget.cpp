#include "ChatWidget.h"
#include "MessageList.h"
#include "InputWidget.h"
#include "../include/ChatManager.h"
#include "../models/ChatMessage.h"
#include "../models/Participant.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QListWidget>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

class ChatWidget::Private
{
public:
    Private(ChatWidget* parent) : q(parent) {
        initializeDefaults();
    }
    
    ChatWidget* q;
    
    // Core components
    ChatManager* chatManager = nullptr;
    MessageList* messageList = nullptr;
    InputWidget* inputWidget = nullptr;
    
    // Layout components
    QVBoxLayout* mainLayout = nullptr;
    QHBoxLayout* contentLayout = nullptr;
    QSplitter* mainSplitter = nullptr;
    QSplitter* rightSplitter = nullptr;
    
    // UI components
    QToolBar* toolbar = nullptr;
    QStatusBar* statusBar = nullptr;
    QListWidget* participantList = nullptr;
    QLabel* statusLabel = nullptr;
    QLabel* participantCountLabel = nullptr;
    
    // Actions
    QAction* connectAction = nullptr;
    QAction* disconnectAction = nullptr;
    QAction* settingsAction = nullptr;
    QAction* emojiAction = nullptr;
    QAction* fileAction = nullptr;
    QAction* participantListAction = nullptr;
    QAction* fullScreenAction = nullptr;
    
    // State
    QString currentRoom;
    bool connected = false;
    int participantCount = 0;
    bool inputEnabled = true;
    QString theme = "default";
    DisplayMode displayMode = NormalMode;
    ToolbarPosition toolbarPosition = TopToolbar;
    bool participantListVisible = true;
    bool toolbarVisible = true;
    bool statusBarVisible = true;
    QString customStyleSheet;
    
    void initializeDefaults() {
        // Default values already set in member initialization
    }
};

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
    , d(std::make_unique<Private>(this))
{
    initializeUI();
    connectSignals();
    applyStyles();
    updateUIState();
    
    setAcceptDrops(true);
}

ChatWidget::~ChatWidget() = default;

void ChatWidget::setChatManager(ChatManager* manager)
{
    if (d->chatManager == manager) {
        return;
    }
    
    if (d->chatManager) {
        // Disconnect old manager signals
        disconnect(d->chatManager, nullptr, this, nullptr);
    }
    
    d->chatManager = manager;
    
    if (d->chatManager) {
        // Connect new manager signals
        connect(d->chatManager, &ChatManager::messageReceived,
                this, &ChatWidget::handleMessageReceived);
        connect(d->chatManager, &ChatManager::messageSent,
                this, &ChatWidget::handleMessageSent);
        connect(d->chatManager, &ChatManager::messageSendFailed,
                this, &ChatWidget::handleMessageSendFailed);
        connect(d->chatManager, &ChatManager::participantJoined,
                this, &ChatWidget::handleParticipantJoined);
        connect(d->chatManager, &ChatManager::participantLeft,
                this, &ChatWidget::handleParticipantLeft);
        connect(d->chatManager, &ChatManager::connectionChanged,
                this, &ChatWidget::handleConnectionChanged);
    }
    
    updateUIState();
}

ChatManager* ChatWidget::chatManager() const
{
    return d->chatManager;
}

QString ChatWidget::currentRoom() const
{
    return d->currentRoom;
}

void ChatWidget::setCurrentRoom(const QString& roomId)
{
    if (d->currentRoom != roomId) {
        d->currentRoom = roomId;
        emit currentRoomChanged(roomId);
        updateUIState();
    }
}

bool ChatWidget::isConnected() const
{
    return d->connected;
}

int ChatWidget::participantCount() const
{
    return d->participantCount;
}

bool ChatWidget::isInputEnabled() const
{
    return d->inputEnabled;
}

void ChatWidget::setInputEnabled(bool enabled)
{
    if (d->inputEnabled != enabled) {
        d->inputEnabled = enabled;
        if (d->inputWidget) {
            d->inputWidget->setEnabled(enabled);
        }
        emit inputEnabledChanged(enabled);
    }
}

QString ChatWidget::theme() const
{
    return d->theme;
}

void ChatWidget::setTheme(const QString& theme)
{
    if (d->theme != theme) {
        d->theme = theme;
        applyTheme(theme);
        emit themeChanged(theme);
    }
}

ChatWidget::DisplayMode ChatWidget::displayMode() const
{
    return d->displayMode;
}

void ChatWidget::setDisplayMode(DisplayMode mode)
{
    if (d->displayMode != mode) {
        d->displayMode = mode;
        updateUIState();
    }
}

ChatWidget::ToolbarPosition ChatWidget::toolbarPosition() const
{
    return d->toolbarPosition;
}

void ChatWidget::setToolbarPosition(ToolbarPosition position)
{
    if (d->toolbarPosition != position) {
        d->toolbarPosition = position;
        createToolbar(); // Recreate toolbar in new position
    }
}

MessageList* ChatWidget::messageList() const
{
    return d->messageList;
}

InputWidget* ChatWidget::inputWidget() const
{
    return d->inputWidget;
}

bool ChatWidget::isParticipantListVisible() const
{
    return d->participantListVisible;
}

void ChatWidget::setParticipantListVisible(bool visible)
{
    if (d->participantListVisible != visible) {
        d->participantListVisible = visible;
        if (d->participantList) {
            d->participantList->setVisible(visible);
        }
        updateUIState();
    }
}

bool ChatWidget::isToolbarVisible() const
{
    return d->toolbarVisible;
}

void ChatWidget::setToolbarVisible(bool visible)
{
    if (d->toolbarVisible != visible) {
        d->toolbarVisible = visible;
        if (d->toolbar) {
            d->toolbar->setVisible(visible);
        }
    }
}

bool ChatWidget::isStatusBarVisible() const
{
    return d->statusBarVisible;
}

void ChatWidget::setStatusBarVisible(bool visible)
{
    if (d->statusBarVisible != visible) {
        d->statusBarVisible = visible;
        if (d->statusBar) {
            d->statusBar->setVisible(visible);
        }
    }
}

void ChatWidget::addToolbarAction(QAction* action)
{
    if (d->toolbar && action) {
        d->toolbar->addAction(action);
    }
}

void ChatWidget::removeToolbarAction(QAction* action)
{
    if (d->toolbar && action) {
        d->toolbar->removeAction(action);
    }
}

QList<QAction*> ChatWidget::toolbarActions() const
{
    if (d->toolbar) {
        return d->toolbar->actions();
    }
    return QList<QAction*>();
}

void ChatWidget::setCustomStyleSheet(const QString& styleSheet)
{
    d->customStyleSheet = styleSheet;
    applyStyles();
}

QString ChatWidget::customStyleSheet() const
{
    return d->customStyleSheet;
}

// Public slots implementation
void ChatWidget::connectToChat(const QString& serverUrl)
{
    if (d->chatManager) {
        if (!serverUrl.isEmpty()) {
            d->chatManager->connectToService(serverUrl);
        } else {
            d->chatManager->connectToService("");
        }
    }
}

void ChatWidget::disconnectFromChat()
{
    if (d->chatManager) {
        d->chatManager->disconnect();
    }
}

void ChatWidget::joinRoom(const QString& roomId, const QString& password)
{
    if (d->chatManager) {
        d->chatManager->joinRoom(roomId, password);
        setCurrentRoom(roomId);
    }
}

void ChatWidget::leaveRoom()
{
    if (d->chatManager) {
        d->chatManager->leaveRoom(d->currentRoom);
        setCurrentRoom(QString());
    }
}

void ChatWidget::sendMessage(const QString& message)
{
    if (d->chatManager && !message.trimmed().isEmpty()) {
        d->chatManager->sendMessage(message);
        emit messageSent(message);
    }
}

void ChatWidget::sendFile(const QString& filePath)
{
    if (d->chatManager && validateFile(filePath)) {
        d->chatManager->sendFile(filePath);
        emit fileSent(filePath);
    }
}

void ChatWidget::clearChatDisplay()
{
    if (d->messageList) {
        d->messageList->clearMessages();
    }
}

void ChatWidget::scrollToBottom()
{
    if (d->messageList) {
        d->messageList->scrollToBottom();
    }
}

void ChatWidget::scrollToTop()
{
    if (d->messageList) {
        d->messageList->scrollToTop();
    }
}

void ChatWidget::scrollToMessage(const QString& messageId)
{
    if (d->messageList) {
        d->messageList->scrollToMessage(messageId);
    }
}

void ChatWidget::highlightMessage(const QString& messageId)
{
    if (d->messageList) {
        d->messageList->highlightMessage(messageId);
    }
}

void ChatWidget::searchMessages(const QString& query)
{
    if (d->messageList) {
        QStringList results = d->messageList->searchMessages(query);
        emit searchRequested(query);
    }
}

// Private methods implementation
void ChatWidget::initializeUI()
{
    // Create main layout
    d->mainLayout = new QVBoxLayout(this);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->mainLayout->setSpacing(0);
    
    // Create toolbar
    createToolbar();
    
    // Create main content area
    d->mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // Create message list
    d->messageList = new MessageList(this);
    
    // Create input widget
    d->inputWidget = new InputWidget(this);
    
    // Create chat area (message list + input)
    QWidget* chatArea = new QWidget();
    QVBoxLayout* chatLayout = new QVBoxLayout(chatArea);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->addWidget(d->messageList, 1);
    chatLayout->addWidget(d->inputWidget, 0);
    
    // Create participant list
    createParticipantList();
    
    // Add to splitter
    d->mainSplitter->addWidget(chatArea);
    d->mainSplitter->addWidget(d->participantList);
    d->mainSplitter->setStretchFactor(0, 3);
    d->mainSplitter->setStretchFactor(1, 1);
    
    // Add to main layout
    d->mainLayout->addWidget(d->mainSplitter, 1);
    
    // Create status bar
    createStatusBar();
}

void ChatWidget::createToolbar()
{
    if (d->toolbar) {
        d->mainLayout->removeWidget(d->toolbar);
        delete d->toolbar;
    }
    
    if (d->toolbarPosition == NoToolbar) {
        return;
    }
    
    d->toolbar = new QToolBar(this);
    
    // Create actions
    d->connectAction = new QAction(tr("Connect"), this);
    d->connectAction->setIcon(QIcon(":/icons/connect.png"));
    
    d->disconnectAction = new QAction(tr("Disconnect"), this);
    d->disconnectAction->setIcon(QIcon(":/icons/disconnect.png"));
    
    d->settingsAction = new QAction(tr("Settings"), this);
    d->settingsAction->setIcon(QIcon(":/icons/settings.png"));
    
    d->emojiAction = new QAction(tr("Emoji"), this);
    d->emojiAction->setIcon(QIcon(":/icons/emoji.png"));
    
    d->fileAction = new QAction(tr("File"), this);
    d->fileAction->setIcon(QIcon(":/icons/file.png"));
    
    d->participantListAction = new QAction(tr("Participants"), this);
    d->participantListAction->setIcon(QIcon(":/icons/participants.png"));
    d->participantListAction->setCheckable(true);
    d->participantListAction->setChecked(d->participantListVisible);
    
    d->fullScreenAction = new QAction(tr("Full Screen"), this);
    d->fullScreenAction->setIcon(QIcon(":/icons/fullscreen.png"));
    
    // Add actions to toolbar
    d->toolbar->addAction(d->connectAction);
    d->toolbar->addAction(d->disconnectAction);
    d->toolbar->addSeparator();
    d->toolbar->addAction(d->emojiAction);
    d->toolbar->addAction(d->fileAction);
    d->toolbar->addSeparator();
    d->toolbar->addAction(d->participantListAction);
    d->toolbar->addAction(d->fullScreenAction);
    d->toolbar->addSeparator();
    d->toolbar->addAction(d->settingsAction);
    
    // Add toolbar to layout based on position
    switch (d->toolbarPosition) {
        case TopToolbar:
            d->mainLayout->insertWidget(0, d->toolbar);
            break;
        case BottomToolbar:
            d->mainLayout->addWidget(d->toolbar);
            break;
        default:
            // For left/right toolbars, would need different layout structure
            d->mainLayout->insertWidget(0, d->toolbar);
            break;
    }
    
    d->toolbar->setVisible(d->toolbarVisible);
}

void ChatWidget::createStatusBar()
{
    d->statusBar = new QStatusBar(this);
    
    d->statusLabel = new QLabel(tr("Disconnected"));
    d->participantCountLabel = new QLabel(tr("0 participants"));
    
    d->statusBar->addWidget(d->statusLabel);
    d->statusBar->addPermanentWidget(d->participantCountLabel);
    
    d->mainLayout->addWidget(d->statusBar);
    d->statusBar->setVisible(d->statusBarVisible);
}

void ChatWidget::createParticipantList()
{
    d->participantList = new QListWidget(this);
    d->participantList->setMaximumWidth(200);
    d->participantList->setVisible(d->participantListVisible);
}

void ChatWidget::connectSignals()
{
    // Input widget signals
    if (d->inputWidget) {
        connect(d->inputWidget, &InputWidget::messageSent,
                this, &ChatWidget::sendMessage);
        connect(d->inputWidget, &InputWidget::filesSelected,
                this, [this](const QStringList& files) {
                    for (const QString& file : files) {
                        sendFile(file);
                    }
                });
    }
    
    // Toolbar action signals
    if (d->connectAction) {
        connect(d->connectAction, &QAction::triggered,
                this, [this]() { connectToChat(); });
    }
    if (d->disconnectAction) {
        connect(d->disconnectAction, &QAction::triggered,
                this, &ChatWidget::disconnectFromChat);
    }
    if (d->settingsAction) {
        connect(d->settingsAction, &QAction::triggered,
                this, &ChatWidget::settingsRequested);
    }
    if (d->participantListAction) {
        connect(d->participantListAction, &QAction::toggled,
                this, [this](bool checked) { setParticipantListVisible(checked); });
    }
}

void ChatWidget::applyStyles()
{
    QString styleSheet = d->customStyleSheet;
    
    if (styleSheet.isEmpty()) {
        // Default styles based on theme
        if (d->theme == "dark") {
            styleSheet = R"(
                ChatWidget {
                    background-color: #2b2b2b;
                    color: #ffffff;
                }
                QToolBar {
                    background-color: #3c3c3c;
                    border: none;
                }
                QStatusBar {
                    background-color: #3c3c3c;
                    color: #ffffff;
                }
            )";
        } else {
            styleSheet = R"(
                ChatWidget {
                    background-color: #ffffff;
                    color: #000000;
                }
                QToolBar {
                    background-color: #f0f0f0;
                    border: 1px solid #d0d0d0;
                }
                QStatusBar {
                    background-color: #f0f0f0;
                    border-top: 1px solid #d0d0d0;
                }
            )";
        }
    }
    
    setStyleSheet(styleSheet);
}

void ChatWidget::updateUIState()
{
    // Update toolbar actions
    if (d->connectAction) {
        d->connectAction->setEnabled(!d->connected);
    }
    if (d->disconnectAction) {
        d->disconnectAction->setEnabled(d->connected);
    }
    
    // Update input widget
    if (d->inputWidget) {
        d->inputWidget->setEnabled(d->connected && d->inputEnabled);
    }
    
    // Update status bar
    updateStatusBar();
    
    // Update participant list action
    if (d->participantListAction) {
        d->participantListAction->setChecked(d->participantListVisible);
    }
}

void ChatWidget::updateStatusBar()
{
    if (d->statusLabel) {
        QString status = d->connected ? tr("Connected") : tr("Disconnected");
        if (d->connected && !d->currentRoom.isEmpty()) {
            status += QString(" - Room: %1").arg(d->currentRoom);
        }
        d->statusLabel->setText(status);
    }
    
    if (d->participantCountLabel) {
        d->participantCountLabel->setText(tr("%1 participants").arg(d->participantCount));
    }
}

// Event handlers
void ChatWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // Handle resize logic if needed
}

void ChatWidget::closeEvent(QCloseEvent* event)
{
    // Disconnect from chat when closing
    disconnectFromChat();
    QWidget::closeEvent(event);
}

void ChatWidget::keyPressEvent(QKeyEvent* event)
{
    // Handle keyboard shortcuts
    if (event->key() == Qt::Key_Escape) {
        if (isFullScreen()) {
            showNormal();
        }
    }
    QWidget::keyPressEvent(event);
}

void ChatWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void ChatWidget::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void ChatWidget::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QStringList filePaths;
        for (const QUrl& url : mimeData->urls()) {
            if (url.isLocalFile()) {
                filePaths << url.toLocalFile();
            }
        }
        handleFileDrop(filePaths);
        event->acceptProposedAction();
    }
}

// Private slot handlers
void ChatWidget::handleMessageReceived(ChatMessage* message)
{
    if (d->messageList && message) {
        d->messageList->addMessage(message);
    }
}

void ChatWidget::handleMessageSent(const QString& messageId)
{
    // Handle message sent confirmation
    qDebug() << "Message sent:" << messageId;
}

void ChatWidget::handleMessageSendFailed(const QString& messageId, const QString& error)
{
    // Handle message send failure
    qWarning() << "Message send failed:" << messageId << error;
    emit errorOccurred(tr("Failed to send message: %1").arg(error));
}

void ChatWidget::handleParticipantJoined(Participant* participant)
{
    if (participant && d->participantList) {
        d->participantList->addItem(participant->displayName());
        d->participantCount++;
        emit participantCountChanged(d->participantCount);
        updateStatusBar();
    }
}

void ChatWidget::handleParticipantLeft(const QString& participantId)
{
    // Remove from participant list
    if (d->participantList) {
        for (int i = 0; i < d->participantList->count(); ++i) {
            QListWidgetItem* item = d->participantList->item(i);
            if (item && item->data(Qt::UserRole).toString() == participantId) {
                delete d->participantList->takeItem(i);
                d->participantCount--;
                emit participantCountChanged(d->participantCount);
                updateStatusBar();
                break;
            }
        }
    }
}

void ChatWidget::handleConnectionChanged(bool connected)
{
    d->connected = connected;
    emit connectionChanged(connected);
    updateUIState();
}

void ChatWidget::handleFileDrop(const QStringList& filePaths)
{
    for (const QString& filePath : filePaths) {
        if (validateFile(filePath)) {
            sendFile(filePath);
        }
    }
}

bool ChatWidget::validateFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }
    
    // Add file validation logic here
    // Check file size, type, etc.
    
    return true;
}

void ChatWidget::applyTheme(const QString& themeName)
{
    d->theme = themeName;
    applyStyles();
}