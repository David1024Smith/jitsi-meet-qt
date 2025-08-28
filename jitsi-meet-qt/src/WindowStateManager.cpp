#include "WindowStateManager.h"
#include "ConfigurationManager.h"
#include "JitsiConstants.h"

#include <QApplication>
#include <QScreen>
#include <QWidget>
#include <QDebug>
#include <QGuiApplication>
#include <algorithm>

WindowStateManager::WindowStateManager(ConfigurationManager* configManager, QObject* parent)
    : QObject(parent)
    , m_configManager(configManager)
    , m_rememberWindowState(true)
{
    if (!m_configManager) {
        qWarning() << "WindowStateManager: ConfigurationManager is null";
        return;
    }

    // 从配置中加载窗口状态记忆设置
    m_rememberWindowState = m_configManager->currentConfiguration().rememberWindowState;

    // 监听屏幕配置变化
    if (auto* guiApp = qobject_cast<QGuiApplication*>(QGuiApplication::instance())) {
        connect(guiApp, &QGuiApplication::screenAdded,
                this, [this](QScreen*) { onScreenConfigurationChanged(); });
        connect(guiApp, &QGuiApplication::screenRemoved,
                this, [this](QScreen*) { onScreenConfigurationChanged(); });
    }

    qDebug() << "WindowStateManager initialized, remember state:" << m_rememberWindowState;
}

WindowStateManager::~WindowStateManager()
{
    qDebug() << "WindowStateManager destroyed";
}

bool WindowStateManager::saveWindowState(QWidget* widget)
{
    if (!widget || !m_configManager || !m_rememberWindowState) {
        qDebug() << "WindowStateManager::saveWindowState: Invalid parameters or disabled";
        return false;
    }

    WindowState state = getCurrentWindowState(widget);
    
    if (!state.valid) {
        qWarning() << "WindowStateManager::saveWindowState: Invalid window state";
        return false;
    }

    // 保存到配置管理器
    m_configManager->setWindowGeometry(state.geometry);
    m_configManager->setWindowMaximized(state.maximized);

    emit windowStateSaved(state);
    
    qDebug() << "Window state saved:" 
             << "geometry=" << state.geometry
             << "maximized=" << state.maximized
             << "screen=" << state.screenName;
    
    return true;
}

bool WindowStateManager::restoreWindowState(QWidget* widget)
{
    if (!widget || !m_configManager || !m_rememberWindowState) {
        qDebug() << "WindowStateManager::restoreWindowState: Invalid parameters or disabled";
        return false;
    }

    // 从配置管理器加载状态
    WindowState state;
    state.geometry = m_configManager->windowGeometry();
    state.maximized = m_configManager->isWindowMaximized();
    state.valid = true;

    // 验证窗口状态
    WindowState validatedState = validateWindowState(state);
    
    if (validatedState != state) {
        qDebug() << "Window state was corrected during validation";
        emit windowStateValidationFailed(state, validatedState);
    }

    // 应用窗口状态
    if (validatedState.maximized) {
        widget->showMaximized();
    } else {
        widget->setGeometry(validatedState.geometry);
        widget->showNormal();
    }

    emit windowStateRestored(validatedState);
    
    qDebug() << "Window state restored:" 
             << "geometry=" << validatedState.geometry
             << "maximized=" << validatedState.maximized
             << "screen=" << validatedState.screenName;
    
    return true;
}

WindowStateManager::WindowState WindowStateManager::getCurrentWindowState(QWidget* widget) const
{
    WindowState state;
    
    if (!widget) {
        qWarning() << "WindowStateManager::getCurrentWindowState: Widget is null";
        return state;
    }

    state.maximized = widget->isMaximized();
    
    if (state.maximized) {
        // 如果窗口最大化，使用正常几何信息
        state.geometry = widget->normalGeometry();
    } else {
        state.geometry = widget->geometry();
    }

    // 获取窗口所在的屏幕
    QScreen* screen = widget->screen();
    if (screen) {
        state.screenName = getScreenName(screen);
    }

    state.valid = !state.geometry.isEmpty() && 
                  state.geometry.width() >= JitsiConstants::MIN_WINDOW_WIDTH &&
                  state.geometry.height() >= JitsiConstants::MIN_WINDOW_HEIGHT;

    return state;
}

WindowStateManager::WindowState WindowStateManager::validateWindowState(const WindowState& state) const
{
    WindowState validatedState = state;

    if (!state.valid || state.geometry.isEmpty()) {
        qDebug() << "Using default window state due to invalid input";
        return getDefaultWindowState();
    }

    // 验证并调整几何信息
    validatedState.geometry = validateGeometry(state.geometry);
    
    // 检查窗口是否在可见区域内
    if (!isWindowVisible(validatedState.geometry)) {
        qDebug() << "Window not visible, adjusting to screen";
        QScreen* bestScreen = getBestScreen(validatedState.geometry);
        validatedState.geometry = adjustToScreen(validatedState.geometry, bestScreen);
        
        if (bestScreen) {
            validatedState.screenName = getScreenName(bestScreen);
        }
    }

    validatedState.valid = true;
    return validatedState;
}

bool WindowStateManager::isWindowVisible(const QRect& geometry) const
{
    if (geometry.isEmpty()) {
        return false;
    }

    // 检查窗口是否与任何屏幕有重叠
    const auto screens = QGuiApplication::screens();
    for (QScreen* screen : screens) {
        if (screen && screen->availableGeometry().intersects(geometry)) {
            // 确保至少有一部分窗口标题栏可见
            QRect titleBarArea(geometry.x(), geometry.y(), geometry.width(), 30);
            if (screen->availableGeometry().intersects(titleBarArea)) {
                return true;
            }
        }
    }

    return false;
}

QScreen* WindowStateManager::getBestScreen(const QRect& geometry) const
{
    const auto screens = QGuiApplication::screens();
    
    if (screens.isEmpty()) {
        return nullptr;
    }

    QScreen* bestScreen = nullptr;
    int maxOverlap = 0;

    // 找到与窗口重叠面积最大的屏幕
    for (QScreen* screen : screens) {
        if (!screen) continue;
        
        int overlap = calculateOverlapArea(geometry, screen);
        if (overlap > maxOverlap) {
            maxOverlap = overlap;
            bestScreen = screen;
        }
    }

    // 如果没有重叠，返回主屏幕
    if (!bestScreen) {
        bestScreen = QGuiApplication::primaryScreen();
    }

    return bestScreen;
}

QRect WindowStateManager::adjustToScreen(const QRect& geometry, QScreen* screen) const
{
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    
    if (!screen) {
        qWarning() << "No screen available for adjustment";
        return geometry;
    }

    QRect adjustedGeometry = geometry;
    QRect screenGeometry = screen->availableGeometry();

    // 确保窗口大小有效
    QSize validSize = ensureValidSize(adjustedGeometry.size());
    adjustedGeometry.setSize(validSize);

    // 确保窗口不超出屏幕大小
    if (adjustedGeometry.width() > screenGeometry.width()) {
        adjustedGeometry.setWidth(screenGeometry.width());
    }
    if (adjustedGeometry.height() > screenGeometry.height()) {
        adjustedGeometry.setHeight(screenGeometry.height());
    }

    // 调整位置确保窗口在屏幕范围内
    adjustedGeometry = ensureOnScreen(adjustedGeometry, screen);

    return adjustedGeometry;
}

WindowStateManager::WindowState WindowStateManager::getDefaultWindowState() const
{
    WindowState defaultState;
    defaultState.geometry = QRect(100, 100, 
                                 JitsiConstants::DEFAULT_WINDOW_WIDTH, 
                                 JitsiConstants::DEFAULT_WINDOW_HEIGHT);
    defaultState.maximized = false;
    defaultState.valid = true;

    // 调整到主屏幕
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        defaultState.geometry = adjustToScreen(defaultState.geometry, primaryScreen);
        defaultState.screenName = getScreenName(primaryScreen);
    }

    return defaultState;
}

bool WindowStateManager::isRememberWindowStateEnabled() const
{
    return m_rememberWindowState;
}

void WindowStateManager::setRememberWindowStateEnabled(bool enabled)
{
    if (m_rememberWindowState != enabled) {
        m_rememberWindowState = enabled;
        
        if (m_configManager) {
            // 更新配置
            ApplicationSettings config = m_configManager->currentConfiguration();
            config.rememberWindowState = enabled;
            m_configManager->saveConfiguration(config);
        }
        
        qDebug() << "Remember window state set to:" << enabled;
    }
}

void WindowStateManager::onScreenConfigurationChanged()
{
    qDebug() << "Screen configuration changed, available screens:" 
             << QGuiApplication::screens().size();
}

QRect WindowStateManager::validateGeometry(const QRect& geometry) const
{
    QRect validGeometry = geometry;

    // 确保窗口大小不小于最小值
    QSize validSize = ensureValidSize(validGeometry.size());
    validGeometry.setSize(validSize);

    return validGeometry;
}

QString WindowStateManager::getScreenName(QScreen* screen) const
{
    if (!screen) {
        return QString();
    }

    QString name = screen->name();
    if (name.isEmpty()) {
        // 使用屏幕几何信息作为标识
        QRect geometry = screen->geometry();
        name = QString("%1x%2+%3+%4")
               .arg(geometry.width())
               .arg(geometry.height())
               .arg(geometry.x())
               .arg(geometry.y());
    }

    return name;
}

QScreen* WindowStateManager::findScreenByName(const QString& screenName) const
{
    if (screenName.isEmpty()) {
        return nullptr;
    }

    const auto screens = QGuiApplication::screens();
    for (QScreen* screen : screens) {
        if (screen && getScreenName(screen) == screenName) {
            return screen;
        }
    }

    return nullptr;
}

int WindowStateManager::calculateOverlapArea(const QRect& geometry, QScreen* screen) const
{
    if (!screen) {
        return 0;
    }

    QRect intersection = geometry.intersected(screen->availableGeometry());
    return intersection.width() * intersection.height();
}

QSize WindowStateManager::ensureValidSize(const QSize& size) const
{
    QSize validSize = size;

    if (validSize.width() < JitsiConstants::MIN_WINDOW_WIDTH) {
        validSize.setWidth(JitsiConstants::MIN_WINDOW_WIDTH);
    }
    if (validSize.height() < JitsiConstants::MIN_WINDOW_HEIGHT) {
        validSize.setHeight(JitsiConstants::MIN_WINDOW_HEIGHT);
    }

    return validSize;
}

QRect WindowStateManager::ensureOnScreen(const QRect& geometry, QScreen* screen) const
{
    if (!screen) {
        return geometry;
    }

    QRect adjustedGeometry = geometry;
    QRect screenGeometry = screen->availableGeometry();

    // 调整位置确保窗口可见
    if (adjustedGeometry.x() < screenGeometry.x()) {
        adjustedGeometry.moveLeft(screenGeometry.x());
    }
    if (adjustedGeometry.y() < screenGeometry.y()) {
        adjustedGeometry.moveTop(screenGeometry.y());
    }

    // 确保窗口不完全超出屏幕右边和下边
    if (adjustedGeometry.left() >= screenGeometry.right()) {
        adjustedGeometry.moveLeft(screenGeometry.right() - adjustedGeometry.width());
    }
    if (adjustedGeometry.top() >= screenGeometry.bottom()) {
        adjustedGeometry.moveTop(screenGeometry.bottom() - adjustedGeometry.height());
    }

    // 如果窗口仍然超出屏幕，调整到屏幕边界
    if (adjustedGeometry.right() > screenGeometry.right()) {
        adjustedGeometry.moveRight(screenGeometry.right());
    }
    if (adjustedGeometry.bottom() > screenGeometry.bottom()) {
        adjustedGeometry.moveBottom(screenGeometry.bottom());
    }

    return adjustedGeometry;
}