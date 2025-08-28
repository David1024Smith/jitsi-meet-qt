#include "WindowManager.h"
#include "WindowStateManager.h"
#include "WelcomeWindow.h"
#include "ConferenceWindow.h"
#include "SettingsDialog.h"
#include "ConfigurationManager.h"
#include "TranslationManager.h"
#include "JitsiConstants.h"

#include <QDebug>
#include <QApplication>
#include <QDateTime>

WindowManager::WindowManager(QObject* parent)
    : QObject(parent)
    , m_welcomeWindow(nullptr)
    , m_conferenceWindow(nullptr)
    , m_settingsDialog(nullptr)
    , m_currentWindowType(WelcomeWindow)
    , m_previousWindowType(WelcomeWindow)
    , m_stateManager(nullptr)
    , m_configManager(nullptr)
    , m_translationManager(nullptr)
    , m_cleanupTimer(nullptr)
    , m_autoCleanup(true)
    , m_cleanupInterval(300000) // 5 minutes
    , m_windowTimeout(600000)   // 10 minutes
{
    // 初始化窗口状态
    m_windowStates[WelcomeWindow] = WindowHidden;
    m_windowStates[ConferenceWindow] = WindowHidden;
    m_windowStates[SettingsDialog] = WindowHidden;
    
    // 创建窗口状态管理器（需要配置管理器）
    // m_stateManager将在setConfigurationManager中创建
    
    // 设置清理定时器
    if (m_autoCleanup) {
        m_cleanupTimer = new QTimer(this);
        m_cleanupTimer->setInterval(m_cleanupInterval);
        connect(m_cleanupTimer, &QTimer::timeout, this, &WindowManager::onCleanupTimer);
        m_cleanupTimer->start();
    }
    
    qDebug() << "WindowManager created with auto cleanup enabled:" << m_autoCleanup;
}

WindowManager::~WindowManager()
{
    qDebug() << "WindowManager destructor called";
    
    // 停止清理定时器
    if (m_cleanupTimer) {
        m_cleanupTimer->stop();
    }
    
    // 保存所有窗口状态
    saveAllWindowStates();
    
    // 关闭所有窗口
    closeAllWindows();
    
    // 清理窗口实例
    if (m_welcomeWindow) {
        disconnectWindowSignals(WelcomeWindow, m_welcomeWindow);
        m_welcomeWindow->deleteLater();
        m_welcomeWindow = nullptr;
    }
    if (m_conferenceWindow) {
        disconnectWindowSignals(ConferenceWindow, m_conferenceWindow);
        m_conferenceWindow->deleteLater();
        m_conferenceWindow = nullptr;
    }
    if (m_settingsDialog) {
        disconnectWindowSignals(SettingsDialog, m_settingsDialog);
        m_settingsDialog->deleteLater();
        m_settingsDialog = nullptr;
    }
    
    qDebug() << "WindowManager destroyed";
}

void WindowManager::setConfigurationManager(ConfigurationManager* configManager)
{
    m_configManager = configManager;
    
    // 创建窗口状态管理器
    if (!m_stateManager && m_configManager) {
        m_stateManager = new WindowStateManager(m_configManager, this);
        qDebug() << "WindowStateManager created";
    }
}

void WindowManager::setTranslationManager(TranslationManager* translationManager)
{
    m_translationManager = translationManager;
    
    if (m_translationManager) {
        // 连接语言改变信号到所有现有窗口
        connect(m_translationManager, &TranslationManager::languageChanged,
                this, &WindowManager::onLanguageChanged);
        qDebug() << "TranslationManager connected to WindowManager";
    }
}

void WindowManager::showWindow(WindowType type, const QVariantMap& data)
{
    qDebug() << "Showing window type:" << getWindowTypeName(type) << "with data keys:" << data.keys();
    
    // 记录访问时间
    m_lastAccessTime[type] = QDateTime::currentMSecsSinceEpoch();
    
    // 保存数据
    if (!data.isEmpty()) {
        m_windowData[type] = data;
    }
    
    // 隐藏当前窗口（除非是对话框）
    if (m_currentWindowType != type && type != SettingsDialog) {
        hideWindow(m_currentWindowType);
        m_previousWindowType = m_currentWindowType;
    }
    
    QWidget* window = nullptr;
    
    switch (type) {
    case WelcomeWindow:
        createWelcomeWindow();
        window = m_welcomeWindow;
        
        // 应用数据到欢迎窗口
        applyWindowData(type, data);
        
        // 恢复窗口状态
        if (m_stateManager) {
            m_stateManager->restoreWindowState(m_welcomeWindow);
        }
        
        m_welcomeWindow->show();
        m_welcomeWindow->raise();
        m_welcomeWindow->activateWindow();
        break;
        
    case ConferenceWindow:
        createConferenceWindow();
        window = m_conferenceWindow;
        
        // 应用数据到会议窗口
        applyWindowData(type, data);
        
        // 恢复窗口状态
        if (m_stateManager) {
            m_stateManager->restoreWindowState(m_conferenceWindow);
        }
        
        m_conferenceWindow->show();
        m_conferenceWindow->raise();
        m_conferenceWindow->activateWindow();
        break;
        
    case SettingsDialog:
        createSettingsDialog();
        window = m_settingsDialog;
        
        // 应用数据到设置对话框
        applyWindowData(type, data);
        
        m_settingsDialog->show();
        m_settingsDialog->raise();
        m_settingsDialog->activateWindow();
        
        // 对话框不改变当前窗口类型
        updateWindowState(type, WindowVisible);
        return;
    }
    
    // 更新状态
    if (type != SettingsDialog) {
        m_currentWindowType = type;
        updateWindowState(type, WindowVisible);
        emit windowChanged(type);
        
        // 发送数据传递信号
        if (!data.isEmpty() && m_previousWindowType != type) {
            emit dataTransferred(m_previousWindowType, type, data);
        }
    }
    
    qDebug() << "Window" << getWindowTypeName(type) << "shown successfully";
}

void WindowManager::closeWindow(WindowType type)
{
    qDebug() << "Closing window type:" << getWindowTypeName(type);
    
    switch (type) {
    case WelcomeWindow:
        if (m_welcomeWindow) {
            // 保存窗口状态
            if (m_stateManager) {
                m_stateManager->saveWindowState(m_welcomeWindow);
            }
            m_welcomeWindow->close();
            updateWindowState(type, WindowHidden);
        }
        break;
        
    case ConferenceWindow:
        if (m_conferenceWindow) {
            // 保存窗口状态
            if (m_stateManager) {
                m_stateManager->saveWindowState(m_conferenceWindow);
            }
            m_conferenceWindow->close();
            updateWindowState(type, WindowHidden);
        }
        break;
        
    case SettingsDialog:
        if (m_settingsDialog) {
            m_settingsDialog->close();
            updateWindowState(type, WindowHidden);
        }
        break;
    }
}

void WindowManager::hideWindow(WindowType type)
{
    qDebug() << "Hiding window type:" << getWindowTypeName(type);
    
    switch (type) {
    case WelcomeWindow:
        if (m_welcomeWindow && m_welcomeWindow->isVisible()) {
            // 保存窗口状态
            if (m_stateManager) {
                m_stateManager->saveWindowState(m_welcomeWindow);
            }
            m_welcomeWindow->hide();
            updateWindowState(type, WindowHidden);
        }
        break;
        
    case ConferenceWindow:
        if (m_conferenceWindow && m_conferenceWindow->isVisible()) {
            // 保存窗口状态
            if (m_stateManager) {
                m_stateManager->saveWindowState(m_conferenceWindow);
            }
            m_conferenceWindow->hide();
            updateWindowState(type, WindowHidden);
        }
        break;
        
    case SettingsDialog:
        if (m_settingsDialog && m_settingsDialog->isVisible()) {
            m_settingsDialog->hide();
            updateWindowState(type, WindowHidden);
        }
        break;
    }
}

QMainWindow* WindowManager::currentWindow() const
{
    switch (m_currentWindowType) {
    case WelcomeWindow:
        return m_welcomeWindow;
    case ConferenceWindow:
        return m_conferenceWindow;
    default:
        return nullptr;
    }
}

QWidget* WindowManager::getWindow(WindowType type) const
{
    switch (type) {
    case WelcomeWindow:
        return m_welcomeWindow;
    case ConferenceWindow:
        return m_conferenceWindow;
    case SettingsDialog:
        return m_settingsDialog;
    default:
        return nullptr;
    }
}

WindowManager::WindowType WindowManager::currentWindowType() const
{
    return m_currentWindowType;
}

WindowManager::WindowState WindowManager::getWindowState(WindowType type) const
{
    return m_windowStates.value(type, WindowHidden);
}

bool WindowManager::hasWindow(WindowType type) const
{
    switch (type) {
    case WelcomeWindow:
        return m_welcomeWindow != nullptr;
    case ConferenceWindow:
        return m_conferenceWindow != nullptr;
    case SettingsDialog:
        return m_settingsDialog != nullptr;
    default:
        return false;
    }
}

bool WindowManager::isWindowVisible(WindowType type) const
{
    QWidget* window = getWindow(type);
    return window && window->isVisible();
}

void WindowManager::onJoinConference(const QString& url)
{
    qDebug() << "Join conference requested:" << url;
    QVariantMap data;
    data["url"] = url;
    showWindow(ConferenceWindow, data);
}

void WindowManager::onBackToWelcome()
{
    qDebug() << "Back to welcome requested";
    showWindow(WelcomeWindow);
}

void WindowManager::onSettingsRequested()
{
    qDebug() << "Settings requested";
    showWindow(SettingsDialog);
}

void WindowManager::onSettingsDialogClosed()
{
    qDebug() << "Settings dialog closed";
    closeWindow(SettingsDialog);
}

void WindowManager::createWelcomeWindow()
{
    if (!m_welcomeWindow) {
        qDebug() << "Creating WelcomeWindow";
        
        m_welcomeWindow = new ::WelcomeWindow();
        
        // 设置配置管理器
        if (m_configManager) {
            m_welcomeWindow->setConfigurationManager(m_configManager);
            m_welcomeWindow->loadRecentItems();
        }
        
        // 连接窗口信号
        connectWindowSignals(WelcomeWindow, m_welcomeWindow);
        
        // 连接翻译信号
        if (m_translationManager) {
            connect(m_translationManager, &TranslationManager::languageChanged,
                    m_welcomeWindow, &::WelcomeWindow::retranslateUi);
        }
        
        // 记录创建时间
        m_lastAccessTime[WelcomeWindow] = QDateTime::currentMSecsSinceEpoch();
        
        emit windowCreated(WelcomeWindow);
    }
}

void WindowManager::createConferenceWindow()
{
    if (!m_conferenceWindow) {
        qDebug() << "Creating ConferenceWindow";
        
        m_conferenceWindow = new ::ConferenceWindow();
        
        // 配置管理器将在需要时由ConferenceWindow内部使用
        
        // 连接窗口信号
        connectWindowSignals(ConferenceWindow, m_conferenceWindow);
        
        // 连接翻译信号
        if (m_translationManager) {
            connect(m_translationManager, &TranslationManager::languageChanged,
                    m_conferenceWindow, &::ConferenceWindow::retranslateUi);
        }
        
        // 记录创建时间
        m_lastAccessTime[ConferenceWindow] = QDateTime::currentMSecsSinceEpoch();
        
        emit windowCreated(ConferenceWindow);
    }
}

void WindowManager::createSettingsDialog()
{
    if (!m_settingsDialog) {
        qDebug() << "Creating SettingsDialog";
        
        m_settingsDialog = new ::SettingsDialog(m_configManager, m_translationManager, nullptr);
        
        // 连接窗口信号
        connectWindowSignals(SettingsDialog, m_settingsDialog);
        
        // 记录创建时间
        m_lastAccessTime[SettingsDialog] = QDateTime::currentMSecsSinceEpoch();
        
        emit windowCreated(SettingsDialog);
    }
}

bool WindowManager::sendDataToWindow(WindowType type, const QVariantMap& data)
{
    qDebug() << "Sending data to window type:" << getWindowTypeName(type) << "data keys:" << data.keys();
    
    if (!hasWindow(type)) {
        qWarning() << "Cannot send data to non-existent window:" << getWindowTypeName(type);
        return false;
    }
    
    // 保存数据
    m_windowData[type] = data;
    
    // 如果窗口可见，立即应用数据
    if (isWindowVisible(type)) {
        applyWindowData(type, data);
    }
    
    return true;
}

void WindowManager::syncWindowState(WindowType type)
{
    QWidget* window = getWindow(type);
    if (!window) {
        return;
    }
    
    WindowState newState = WindowHidden;
    
    if (window->isVisible()) {
        if (window->isMaximized()) {
            newState = WindowMaximized;
        } else if (window->isMinimized()) {
            newState = WindowMinimized;
        } else {
            newState = WindowVisible;
        }
    }
    
    if (m_windowStates[type] != newState) {
        updateWindowState(type, newState);
    }
}

void WindowManager::saveAllWindowStates()
{
    qDebug() << "Saving all window states";
    
    if (!m_stateManager) {
        return;
    }
    
    if (m_welcomeWindow) {
        m_stateManager->saveWindowState(m_welcomeWindow);
    }
    if (m_conferenceWindow) {
        m_stateManager->saveWindowState(m_conferenceWindow);
    }
    // 设置对话框通常不需要保存状态
}

void WindowManager::restoreAllWindowStates()
{
    qDebug() << "Restoring all window states";
    
    if (!m_stateManager) {
        return;
    }
    
    if (m_welcomeWindow) {
        m_stateManager->restoreWindowState(m_welcomeWindow);
    }
    if (m_conferenceWindow) {
        m_stateManager->restoreWindowState(m_conferenceWindow);
    }
}

void WindowManager::cleanupUnusedWindows()
{
    qDebug() << "Cleaning up unused windows";
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    // 检查每个窗口是否需要清理
    QList<WindowType> typesToCleanup;
    
    for (auto it = m_lastAccessTime.begin(); it != m_lastAccessTime.end(); ++it) {
        WindowType type = it.key();
        qint64 lastAccess = it.value();
        
        if (type != m_currentWindowType && // 不清理当前窗口
            (currentTime - lastAccess) > m_windowTimeout && // 超时
            shouldCleanupWindow(type)) {
            typesToCleanup.append(type);
        }
    }
    
    // 执行清理
    for (WindowType type : typesToCleanup) {
        qDebug() << "Cleaning up window:" << getWindowTypeName(type);
        
        switch (type) {
        case WelcomeWindow:
            if (m_welcomeWindow) {
                disconnectWindowSignals(type, m_welcomeWindow);
                m_welcomeWindow->deleteLater();
                m_welcomeWindow = nullptr;
                emit windowDestroyed(type);
            }
            break;
        case ConferenceWindow:
            if (m_conferenceWindow) {
                disconnectWindowSignals(type, m_conferenceWindow);
                m_conferenceWindow->deleteLater();
                m_conferenceWindow = nullptr;
                emit windowDestroyed(type);
            }
            break;
        case SettingsDialog:
            if (m_settingsDialog) {
                disconnectWindowSignals(type, m_settingsDialog);
                m_settingsDialog->deleteLater();
                m_settingsDialog = nullptr;
                emit windowDestroyed(type);
            }
            break;
        }
        
        // 清理相关数据
        m_windowData.remove(type);
        m_lastAccessTime.remove(type);
        updateWindowState(type, WindowHidden);
    }
}

void WindowManager::closeAllWindows()
{
    qDebug() << "Closing all windows";
    
    closeWindow(WelcomeWindow);
    closeWindow(ConferenceWindow);
    closeWindow(SettingsDialog);
}

void WindowManager::onWindowClosed()
{
    QWidget* sender = qobject_cast<QWidget*>(this->sender());
    if (!sender) {
        return;
    }
    
    // 确定是哪个窗口被关闭
    WindowType closedType = WelcomeWindow; // 默认值
    
    if (sender == m_welcomeWindow) {
        closedType = WelcomeWindow;
    } else if (sender == m_conferenceWindow) {
        closedType = ConferenceWindow;
    } else if (sender == m_settingsDialog) {
        closedType = SettingsDialog;
    }
    
    updateWindowState(closedType, WindowHidden);
    
    qDebug() << "Window closed:" << getWindowTypeName(closedType);
}

void WindowManager::onWindowStateChanged()
{
    QWidget* sender = qobject_cast<QWidget*>(this->sender());
    if (!sender) {
        return;
    }
    
    // 确定是哪个窗口状态改变
    WindowType changedType = WelcomeWindow; // 默认值
    
    if (sender == m_welcomeWindow) {
        changedType = WelcomeWindow;
    } else if (sender == m_conferenceWindow) {
        changedType = ConferenceWindow;
    } else if (sender == m_settingsDialog) {
        changedType = SettingsDialog;
    }
    
    syncWindowState(changedType);
}

void WindowManager::onCleanupTimer()
{
    if (m_autoCleanup) {
        cleanupUnusedWindows();
    }
}

void WindowManager::onConferenceJoined(const QString& url)
{
    qDebug() << "Conference joined:" << url;
    
    // 更新最近访问时间
    m_lastAccessTime[ConferenceWindow] = QDateTime::currentMSecsSinceEpoch();
    
    // 保存会议URL到配置
    if (m_configManager) {
        m_configManager->addRecentUrl(url);
    }
}

void WindowManager::connectWindowSignals(WindowType type, QWidget* window)
{
    if (!window) {
        return;
    }
    
    qDebug() << "Connecting signals for window:" << getWindowTypeName(type);
    
    // 连接通用窗口信号
    connect(window, &QWidget::destroyed, this, &WindowManager::onWindowClosed);
    
    // 连接特定窗口信号
    switch (type) {
    case WelcomeWindow:
        if (auto welcomeWindow = qobject_cast<::WelcomeWindow*>(window)) {
            connect(welcomeWindow, &::WelcomeWindow::joinConference,
                    this, &WindowManager::onJoinConference);
            connect(welcomeWindow, &::WelcomeWindow::settingsRequested,
                    this, &WindowManager::onSettingsRequested);
        }
        break;
        
    case ConferenceWindow:
        if (auto conferenceWindow = qobject_cast<::ConferenceWindow*>(window)) {
            connect(conferenceWindow, &::ConferenceWindow::backToWelcome,
                    this, &WindowManager::onBackToWelcome);
            connect(conferenceWindow, &::ConferenceWindow::conferenceJoined,
                    this, &WindowManager::onConferenceJoined);
        }
        break;
        
    case SettingsDialog:
        if (auto settingsDialog = qobject_cast<QDialog*>(window)) {
            connect(settingsDialog, &QDialog::finished, 
                    this, &WindowManager::onSettingsDialogClosed);
        }
        break;
    }
}

void WindowManager::disconnectWindowSignals(WindowType type, QWidget* window)
{
    if (!window) {
        return;
    }
    
    qDebug() << "Disconnecting signals for window:" << getWindowTypeName(type);
    
    // 断开所有信号连接
    disconnect(window, nullptr, this, nullptr);
}

void WindowManager::applyWindowData(WindowType type, const QVariantMap& data)
{
    if (data.isEmpty()) {
        return;
    }
    
    qDebug() << "Applying data to window:" << getWindowTypeName(type) << "data:" << data;
    
    switch (type) {
    case WelcomeWindow:
        if (m_welcomeWindow) {
            if (data.contains("url")) {
                m_welcomeWindow->setUrlText(data.value("url").toString());
            }
            if (data.contains("error")) {
                m_welcomeWindow->showError(data.value("error").toString());
            }
        }
        break;
        
    case ConferenceWindow:
        if (m_conferenceWindow) {
            if (data.contains("url")) {
                QString url = data.value("url").toString();
                m_conferenceWindow->joinConference(url);
            }
        }
        break;
        
    case SettingsDialog:
        // 设置对话框的数据应用将在后续任务中实现
        break;
    }
}

void WindowManager::updateWindowState(WindowType type, WindowState state)
{
    WindowState oldState = m_windowStates.value(type, WindowHidden);
    
    if (oldState != state) {
        m_windowStates[type] = state;
        emit windowStateChanged(type, state);
        
        qDebug() << "Window state changed:" << getWindowTypeName(type) 
                 << "from" << oldState << "to" << state;
    }
}

QString WindowManager::getWindowTypeName(WindowType type) const
{
    switch (type) {
    case WelcomeWindow:
        return "WelcomeWindow";
    case ConferenceWindow:
        return "ConferenceWindow";
    case SettingsDialog:
        return "SettingsDialog";
    default:
        return "Unknown";
    }
}

bool WindowManager::shouldCleanupWindow(WindowType type) const
{
    // 不清理当前窗口和设置对话框
    if (type == m_currentWindowType || type == SettingsDialog) {
        return false;
    }
    
    // 检查窗口是否可见
    if (isWindowVisible(type)) {
        return false;
    }
    
    return true;
}

void WindowManager::onLanguageChanged(const QString& language)
{
    qDebug() << "Language changed to:" << language;
    
    // All UI components should automatically update through their connected slots
    // This method can be used for any additional language change handling
}