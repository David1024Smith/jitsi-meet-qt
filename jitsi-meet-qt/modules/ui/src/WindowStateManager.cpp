#include "WindowStateManager.h"
#include <QEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QDebug>

WindowStateManager::WindowStateManager(QObject *parent)
    : QObject(parent)
    , m_throttleInterval(100) // 100ms throttle by default
{
}

WindowStateManager::~WindowStateManager()
{
    // 清理所有注册的窗口
    for (auto it = m_windows.begin(); it != m_windows.end(); ++it) {
        cleanupWindow(it.key());
    }
}

bool WindowStateManager::registerWindow(QWidget* window, const QString& name)
{
    if (!window) {
        return false;
    }

    if (m_windows.contains(window)) {
        qDebug() << "Window already registered";
        return true;
    }

    QString windowName = name.isEmpty() ? generateWindowName(window) : name;
    
    // 检查名称冲突
    if (m_windowsByName.contains(windowName)) {
        qWarning() << "Window name already exists:" << windowName;
        return false;
    }

    WindowInfo info;
    info.name = windowName;
    info.widget = window;
    info.state = Normal;
    info.size = window->size();
    info.screenSize = calculateScreenSize(info.size);
    info.responsiveEnabled = false;
    info.resizeThrottle = new QTimer(this);
    info.resizeThrottle->setSingleShot(true);
    info.resizeThrottle->setInterval(m_throttleInterval);

    m_windows[window] = info;
    m_windowsByName[windowName] = window;

    setupWindow(window);

    emit windowRegistered(window, windowName);
    qDebug() << "Window registered:" << windowName;
    return true;
}

bool WindowStateManager::unregisterWindow(QWidget* window)
{
    if (!window || !m_windows.contains(window)) {
        return false;
    }

    const WindowInfo& info = m_windows[window];
    QString name = info.name;

    cleanupWindow(window);
    m_windowsByName.remove(name);
    m_windows.remove(window);

    emit windowUnregistered(window);
    qDebug() << "Window unregistered:" << name;
    return true;
}

QWidget* WindowStateManager::getWindow(const QString& name) const
{
    return m_windowsByName.value(name, nullptr);
}

QStringList WindowStateManager::registeredWindows() const
{
    return m_windowsByName.keys();
}

WindowStateManager::WindowState WindowStateManager::getWindowState(QWidget* window) const
{
    if (!window || !m_windows.contains(window)) {
        return Normal;
    }

    return m_windows[window].state;
}

bool WindowStateManager::setWindowState(QWidget* window, WindowState state)
{
    if (!window || !m_windows.contains(window)) {
        return false;
    }

    WindowInfo& info = m_windows[window];
    if (info.state == state) {
        return true;
    }

    WindowState oldState = info.state;
    info.state = state;

    // 应用窗口状态
    switch (state) {
    case Normal:
        window->showNormal();
        break;
    case Minimized:
        window->showMinimized();
        break;
    case Maximized:
        window->showMaximized();
        break;
    case FullScreen:
        window->showFullScreen();
        break;
    }

    emit windowStateChanged(window, state);
    qDebug() << "Window state changed from" << oldState << "to" << state;
    return true;
}

QSize WindowStateManager::getWindowSize(QWidget* window) const
{
    if (!window || !m_windows.contains(window)) {
        return QSize();
    }

    return m_windows[window].size;
}

bool WindowStateManager::setWindowSize(QWidget* window, const QSize& size)
{
    if (!window || !m_windows.contains(window) || !size.isValid()) {
        return false;
    }

    window->resize(size);
    return true;
}

WindowStateManager::ScreenSize WindowStateManager::getScreenSize(QWidget* window) const
{
    if (!window || !m_windows.contains(window)) {
        return Medium;
    }

    return m_windows[window].screenSize;
}

WindowStateManager::ScreenSize WindowStateManager::getScreenSize(const QSize& size) const
{
    return calculateScreenSize(size);
}

bool WindowStateManager::enableResponsiveDesign(QWidget* window, bool enabled)
{
    if (!window || !m_windows.contains(window)) {
        return false;
    }

    WindowInfo& info = m_windows[window];
    if (info.responsiveEnabled == enabled) {
        return true;
    }

    info.responsiveEnabled = enabled;

    if (enabled) {
        // 连接调整大小节流器
        connect(info.resizeThrottle, &QTimer::timeout,
                this, &WindowStateManager::onResizeThrottleTimeout);
        
        // 立即更新响应式布局
        updateResponsiveLayout(window);
    } else {
        // 断开连接
        disconnect(info.resizeThrottle, &QTimer::timeout,
                   this, &WindowStateManager::onResizeThrottleTimeout);
    }

    qDebug() << "Responsive design" << (enabled ? "enabled" : "disabled") << "for window";
    return true;
}

bool WindowStateManager::isResponsiveDesignEnabled(QWidget* window) const
{
    if (!window || !m_windows.contains(window)) {
        return false;
    }

    return m_windows[window].responsiveEnabled;
}

void WindowStateManager::updateResponsiveLayout(QWidget* window)
{
    if (!window || !m_windows.contains(window)) {
        return;
    }

    const WindowInfo& info = m_windows[window];
    if (!info.responsiveEnabled) {
        return;
    }

    // 这里可以触发布局管理器的响应式更新
    // 例如：layoutManager->adaptToSize(window, info.size);
    
    emit responsiveLayoutUpdated(window);
    qDebug() << "Responsive layout updated for window";
}

void WindowStateManager::updateAllResponsiveLayouts()
{
    for (auto it = m_windows.begin(); it != m_windows.end(); ++it) {
        QWidget* window = it.key();
        const WindowInfo& info = it.value();
        
        if (info.responsiveEnabled) {
            updateResponsiveLayout(window);
        }
    }
}

void WindowStateManager::setResizeThrottleInterval(int milliseconds)
{
    m_throttleInterval = qMax(50, milliseconds); // 最小50ms

    // 更新所有现有的节流器
    for (auto it = m_windows.begin(); it != m_windows.end(); ++it) {
        WindowInfo& info = it.value();
        if (info.resizeThrottle) {
            info.resizeThrottle->setInterval(m_throttleInterval);
        }
    }
}

int WindowStateManager::resizeThrottleInterval() const
{
    return m_throttleInterval;
}

bool WindowStateManager::eventFilter(QObject* watched, QEvent* event)
{
    QWidget* window = qobject_cast<QWidget*>(watched);
    if (!window || !m_windows.contains(window)) {
        return QObject::eventFilter(watched, event);
    }

    WindowInfo& info = m_windows[window];

    switch (event->type()) {
    case QEvent::Resize: {
        QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(event);
        QSize newSize = resizeEvent->size();
        
        if (info.size != newSize) {
            info.size = newSize;
            
            ScreenSize newScreenSize = calculateScreenSize(newSize);
            if (info.screenSize != newScreenSize) {
                info.screenSize = newScreenSize;
                emit screenSizeChanged(window, newScreenSize);
            }
            
            emit windowSizeChanged(window, newSize);
            
            // 如果启用了响应式设计，启动节流器
            if (info.responsiveEnabled && info.resizeThrottle) {
                info.resizeThrottle->start();
            }
        }
        break;
    }
    case QEvent::WindowStateChange: {
        updateWindowInfo(window);
        break;
    }
    default:
        break;
    }

    return QObject::eventFilter(watched, event);
}

void WindowStateManager::onResizeThrottleTimeout()
{
    QTimer* timer = qobject_cast<QTimer*>(sender());
    if (!timer) {
        return;
    }

    // 查找对应的窗口
    for (auto it = m_windows.begin(); it != m_windows.end(); ++it) {
        const WindowInfo& info = it.value();
        if (info.resizeThrottle == timer) {
            updateResponsiveLayout(it.key());
            break;
        }
    }
}

void WindowStateManager::setupWindow(QWidget* window)
{
    if (!window) {
        return;
    }

    // 安装事件过滤器
    window->installEventFilter(this);
    
    // 更新窗口信息
    updateWindowInfo(window);
}

void WindowStateManager::cleanupWindow(QWidget* window)
{
    if (!window) {
        return;
    }

    // 移除事件过滤器
    window->removeEventFilter(this);
    
    // 清理节流器
    if (m_windows.contains(window)) {
        const WindowInfo& info = m_windows[window];
        if (info.resizeThrottle) {
            info.resizeThrottle->deleteLater();
        }
    }
}

void WindowStateManager::updateWindowInfo(QWidget* window)
{
    if (!window || !m_windows.contains(window)) {
        return;
    }

    WindowInfo& info = m_windows[window];
    
    // 更新窗口状态
    WindowState newState = Normal;
    if (window->isMinimized()) {
        newState = Minimized;
    } else if (window->isMaximized()) {
        newState = Maximized;
    } else if (window->isFullScreen()) {
        newState = FullScreen;
    }
    
    if (info.state != newState) {
        info.state = newState;
        emit windowStateChanged(window, newState);
    }
    
    // 更新窗口大小
    QSize newSize = window->size();
    if (info.size != newSize) {
        info.size = newSize;
        
        ScreenSize newScreenSize = calculateScreenSize(newSize);
        if (info.screenSize != newScreenSize) {
            info.screenSize = newScreenSize;
            emit screenSizeChanged(window, newScreenSize);
        }
        
        emit windowSizeChanged(window, newSize);
    }
}

WindowStateManager::ScreenSize WindowStateManager::calculateScreenSize(const QSize& size) const
{
    int width = size.width();
    
    if (width < 600) {
        return ExtraSmall;
    } else if (width < 800) {
        return Small;
    } else if (width < 1200) {
        return Medium;
    } else if (width < 1600) {
        return Large;
    } else {
        return ExtraLarge;
    }
}

QString WindowStateManager::generateWindowName(QWidget* window) const
{
    if (!window) {
        return QString();
    }

    QString baseName = window->objectName();
    if (baseName.isEmpty()) {
        baseName = window->metaObject()->className();
    }
    
    // 确保名称唯一
    QString name = baseName;
    int counter = 1;
    while (m_windowsByName.contains(name)) {
        name = QString("%1_%2").arg(baseName).arg(counter++);
    }
    
    return name;
}