#include "ScreenShareManager.h"
#include "WebRTCEngine.h"
#include <QApplication>
// QDesktopWidget is deprecated in Qt 6, use QScreen instead
#include <QScreen>
#include <QWindow>
#include <QPixmap>
#include <QBuffer>
#include <QImageWriter>
#include <QDebug>

// Define static constants
const int ScreenShareManager::DEFAULT_FRAME_RATE;
const int ScreenShareManager::DEFAULT_BITRATE;
const int ScreenShareManager::PERFORMANCE_CHECK_INTERVAL;
const int ScreenShareManager::MIN_FRAME_RATE;
const int ScreenShareManager::MAX_FRAME_RATE;
#include <QMessageBox>
#include <QGridLayout>
#include <QTabWidget>
#include <QScrollArea>
#include <QGroupBox>

ScreenShareManager::ScreenShareManager(QObject *parent)
    : QObject(parent)
    , m_captureTimer(new QTimer(this))
    , m_isScreenSharing(false)
    , m_isWindowSharing(false)
    , m_localScreenShareWidget(nullptr)
    , m_webrtcEngine(nullptr)
    , m_performanceTimer(new QTimer(this))
    , m_frameCount(0)
    , m_lastFrameTime(0)
{
    initializeCapture();
    
    // 连接定时器信号
    connect(m_captureTimer, &QTimer::timeout, this, &ScreenShareManager::onCaptureTimer);
    connect(m_performanceTimer, &QTimer::timeout, this, &ScreenShareManager::adjustQualityBasedOnPerformance);
    
    // 监听屏幕变化
    if (auto guiApp = qobject_cast<QGuiApplication*>(QGuiApplication::instance())) {
        connect(guiApp, &QGuiApplication::screenAdded, this, &ScreenShareManager::onScreenChanged);
        connect(guiApp, &QGuiApplication::screenRemoved, this, &ScreenShareManager::onScreenChanged);
    }
    
    // 初始化屏幕和窗口列表
    refreshScreenList();
    refreshWindowList();
    
    // 启动性能监控
    m_performanceTimer->start(PERFORMANCE_CHECK_INTERVAL);
}

ScreenShareManager::~ScreenShareManager()
{
    cleanupCapture();
}

void ScreenShareManager::initializeCapture()
{
    // 创建本地屏幕共享显示组件
    m_localScreenShareWidget = new QVideoWidget();
    m_localScreenShareWidget->setMinimumSize(320, 240);
    m_localScreenShareWidget->hide();
    
    // 设置默认质量
    m_shareQuality = ShareQuality();
    
    qDebug() << "ScreenShareManager: Initialized";
}

void ScreenShareManager::cleanupCapture()
{
    stopScreenShare();
    
    if (m_localScreenShareWidget) {
        delete m_localScreenShareWidget;
        m_localScreenShareWidget = nullptr;
    }
    
    // 清理远程屏幕共享组件
    for (auto it = m_remoteScreenShareWidgets.begin(); it != m_remoteScreenShareWidgets.end(); ++it) {
        delete it.value();
    }
    m_remoteScreenShareWidgets.clear();
    
    qDebug() << "ScreenShareManager: Cleaned up";
}

QList<ScreenShareManager::ScreenInfo> ScreenShareManager::availableScreens() const
{
    return m_screens;
}

QList<ScreenShareManager::WindowInfo> ScreenShareManager::availableWindows() const
{
    return m_windows;
}

void ScreenShareManager::refreshScreenList()
{
    enumerateScreens();
    emit screenListChanged();
}

void ScreenShareManager::refreshWindowList()
{
    enumerateWindows();
    emit windowListChanged();
}

bool ScreenShareManager::startScreenShare(int screenId)
{
    if (m_isScreenSharing || m_isWindowSharing) {
        qWarning() << "ScreenShareManager: Already sharing";
        return false;
    }
    
    // 如果没有指定屏幕ID，显示选择对话框
    if (screenId == -1) {
        if (!showScreenSelectionDialog()) {
            return false;
        }
        return true; // 对话框会调用相应的开始方法
    }
    
    // 查找指定的屏幕
    ScreenInfo targetScreen;
    bool found = false;
    for (const auto& screen : m_screens) {
        if (screen.screenId == screenId) {
            targetScreen = screen;
            found = true;
            break;
        }
    }
    
    if (!found) {
        emit screenCaptureError(QString("Screen with ID %1 not found").arg(screenId));
        return false;
    }
    
    try {
        setupScreenCapture(screenId);
        m_currentScreen = targetScreen;
        m_isScreenSharing = true;
        
        // 开始捕获
        updateCaptureSettings();
        m_captureTimer->start(1000 / m_shareQuality.frameRate);
        
        emit screenShareStarted();
        qDebug() << "ScreenShareManager: Started screen sharing for screen" << screenId;
        return true;
        
    } catch (const std::exception& e) {
        emit screenCaptureError(QString("Failed to start screen sharing: %1").arg(e.what()));
        return false;
    }
}

bool ScreenShareManager::startWindowShare(qint64 windowId)
{
    if (m_isScreenSharing || m_isWindowSharing) {
        qWarning() << "ScreenShareManager: Already sharing";
        return false;
    }
    
    // 查找指定的窗口
    WindowInfo targetWindow;
    bool found = false;
    for (const auto& window : m_windows) {
        if (window.windowId == windowId) {
            targetWindow = window;
            found = true;
            break;
        }
    }
    
    if (!found) {
        emit windowCaptureError(QString("Window with ID %1 not found").arg(windowId));
        return false;
    }
    
    try {
        setupWindowCapture(windowId);
        m_currentWindow = targetWindow;
        m_isWindowSharing = true;
        
        // 开始捕获
        updateCaptureSettings();
        m_captureTimer->start(1000 / m_shareQuality.frameRate);
        
        emit windowShareStarted();
        qDebug() << "ScreenShareManager: Started window sharing for window" << windowId;
        return true;
        
    } catch (const std::exception& e) {
        emit windowCaptureError(QString("Failed to start window sharing: %1").arg(e.what()));
        return false;
    }
}

void ScreenShareManager::stopScreenShare()
{
    if (!m_isScreenSharing && !m_isWindowSharing) {
        return;
    }
    
    m_captureTimer->stop();
    
    if (m_isScreenSharing) {
        m_isScreenSharing = false;
        m_currentScreen = ScreenInfo();
        emit screenShareStopped();
        qDebug() << "ScreenShareManager: Stopped screen sharing";
    }
    
    if (m_isWindowSharing) {
        m_isWindowSharing = false;
        m_currentWindow = WindowInfo();
        emit windowShareStopped();
        qDebug() << "ScreenShareManager: Stopped window sharing";
    }
    
    // 隐藏本地预览
    if (m_localScreenShareWidget) {
        m_localScreenShareWidget->hide();
    }
}

bool ScreenShareManager::isScreenSharing() const
{
    return m_isScreenSharing;
}

bool ScreenShareManager::isWindowSharing() const
{
    return m_isWindowSharing;
}

ScreenShareManager::ScreenInfo ScreenShareManager::currentScreen() const
{
    return m_currentScreen;
}

ScreenShareManager::WindowInfo ScreenShareManager::currentWindow() const
{
    return m_currentWindow;
}

bool ScreenShareManager::showScreenSelectionDialog()
{
    refreshScreenList();
    refreshWindowList();
    
    ScreenSelectionDialog dialog(m_screens, m_windows);
    if (dialog.exec() == QDialog::Accepted) {
        if (dialog.isScreenSelected()) {
            return startScreenShare(dialog.selectedScreenId());
        } else if (dialog.isWindowSelected()) {
            return startWindowShare(dialog.selectedWindowId());
        }
    }
    
    return false;
}

void ScreenShareManager::setShareQuality(const ShareQuality& quality)
{
    m_shareQuality = quality;
    updateCaptureSettings();
    qDebug() << "ScreenShareManager: Updated quality settings";
}

ScreenShareManager::ShareQuality ScreenShareManager::shareQuality() const
{
    return m_shareQuality;
}

void ScreenShareManager::addRemoteScreenShare(const QString& participantId, QVideoWidget* widget)
{
    if (m_remoteScreenShareWidgets.contains(participantId)) {
        removeRemoteScreenShare(participantId);
    }
    
    m_remoteScreenShareWidgets[participantId] = widget;
    emit remoteScreenShareReceived(participantId, widget);
    qDebug() << "ScreenShareManager: Added remote screen share for" << participantId;
}

void ScreenShareManager::removeRemoteScreenShare(const QString& participantId)
{
    if (m_remoteScreenShareWidgets.contains(participantId)) {
        QVideoWidget* widget = m_remoteScreenShareWidgets.take(participantId);
        delete widget;
        emit remoteScreenShareRemoved(participantId);
        qDebug() << "ScreenShareManager: Removed remote screen share for" << participantId;
    }
}

QVideoWidget* ScreenShareManager::remoteScreenShareWidget(const QString& participantId) const
{
    return m_remoteScreenShareWidgets.value(participantId, nullptr);
}

QList<QString> ScreenShareManager::remoteScreenShareParticipants() const
{
    return m_remoteScreenShareWidgets.keys();
}

QVideoWidget* ScreenShareManager::localScreenShareWidget() const
{
    return m_localScreenShareWidget;
}

void ScreenShareManager::setWebRTCEngine(WebRTCEngine* engine)
{
    m_webrtcEngine = engine;
    qDebug() << "ScreenShareManager: WebRTC engine set";
}

WebRTCEngine* ScreenShareManager::webRTCEngine() const
{
    return m_webrtcEngine;
}

void ScreenShareManager::onCaptureTimer()
{
    captureCurrentFrame();
    m_frameCount++;
}

void ScreenShareManager::onScreenChanged()
{
    refreshScreenList();
}

void ScreenShareManager::onWindowChanged()
{
    refreshWindowList();
}

void ScreenShareManager::setupScreenCapture(int screenId)
{
    // 查找屏幕
    for (const auto& screenInfo : m_screens) {
        if (screenInfo.screenId == screenId) {
            m_currentScreen = screenInfo;
            break;
        }
    }
    
    qDebug() << "ScreenShareManager: Setup screen capture for screen" << screenId;
}

void ScreenShareManager::setupWindowCapture(qint64 windowId)
{
    // 查找窗口
    for (const auto& windowInfo : m_windows) {
        if (windowInfo.windowId == windowId) {
            m_currentWindow = windowInfo;
            break;
        }
    }
    
    qDebug() << "ScreenShareManager: Setup window capture for window" << windowId;
}

void ScreenShareManager::captureCurrentFrame()
{
    QPixmap frame;
    
    if (m_isScreenSharing) {
        frame = captureScreen(m_currentScreen);
    } else if (m_isWindowSharing) {
        frame = captureWindow(m_currentWindow);
    } else {
        return;
    }
    
    if (frame.isNull()) {
        return;
    }
    
    // 编码并发送帧
    QByteArray frameData = encodeFrame(frame);
    if (!frameData.isEmpty()) {
        sendFrameToWebRTC(frameData);
    }
    
    // 更新本地预览
    if (m_localScreenShareWidget) {
        // 这里需要将QPixmap转换为视频帧并显示
        // 简化实现：直接设置为背景
        m_localScreenShareWidget->show();
    }
}

QPixmap ScreenShareManager::captureScreen(const ScreenInfo& screen)
{
    if (!screen.screen) {
        return QPixmap();
    }
    
    try {
        QPixmap screenshot = screen.screen->grabWindow(0);
        
        // 如果需要缩放到指定分辨率
        if (m_shareQuality.resolution != screenshot.size()) {
            screenshot = screenshot.scaled(m_shareQuality.resolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        
        return screenshot;
        
    } catch (const std::exception& e) {
        emit screenCaptureError(QString("Failed to capture screen: %1").arg(e.what()));
        return QPixmap();
    }
}

QPixmap ScreenShareManager::captureWindow(const WindowInfo& window)
{
    // Windows平台的窗口捕获实现
    // 这里是简化实现，实际需要使用Windows API
    try {
        QScreen* screen = QGuiApplication::primaryScreen();
        if (screen) {
            QPixmap screenshot = screen->grabWindow(window.windowId, 
                                                  window.geometry.x(), 
                                                  window.geometry.y(),
                                                  window.geometry.width(), 
                                                  window.geometry.height());
            
            // 如果需要缩放到指定分辨率
            if (m_shareQuality.resolution != screenshot.size()) {
                screenshot = screenshot.scaled(m_shareQuality.resolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            
            return screenshot;
        }
        
    } catch (const std::exception& e) {
        emit windowCaptureError(QString("Failed to capture window: %1").arg(e.what()));
    }
    
    return QPixmap();
}

QByteArray ScreenShareManager::encodeFrame(const QPixmap& frame)
{
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    
    // 使用JPEG编码以减少数据量
    if (!frame.save(&buffer, "JPEG", 85)) { // 85%质量
        emit encodingError("Failed to encode frame to JPEG");
        return QByteArray();
    }
    
    return data;
}

void ScreenShareManager::sendFrameToWebRTC(const QByteArray& frameData)
{
    if (m_webrtcEngine) {
        // Convert byte array back to QPixmap for WebRTC transmission
        QPixmap frame;
        if (frame.loadFromData(frameData, "JPEG")) {
            m_webrtcEngine->sendScreenFrame(frame);
            qDebug() << "ScreenShareManager: Sent frame to WebRTC, size:" << frameData.size();
        } else {
            emit encodingError("Failed to convert frame data for WebRTC transmission");
        }
    }
}

void ScreenShareManager::processRemoteFrame(const QString& participantId, const QByteArray& frameData)
{
    QVideoWidget* widget = m_remoteScreenShareWidgets.value(participantId);
    if (widget) {
        // 解码并显示远程屏幕共享帧
        QPixmap frame;
        if (frame.loadFromData(frameData, "JPEG")) {
            // 这里需要将QPixmap转换为视频帧并显示到widget
            // 简化实现
            qDebug() << "ScreenShareManager: Processed remote frame from" << participantId;
        }
    }
}

void ScreenShareManager::enumerateScreens()
{
    m_screens.clear();
    
    QList<QScreen*> screens = QGuiApplication::screens();
    for (int i = 0; i < screens.size(); ++i) {
        QScreen* screen = screens[i];
        ScreenInfo info;
        info.screenId = i;
        info.name = screen->name();
        info.size = screen->size();
        info.geometry = screen->geometry();
        info.isPrimary = (screen == QGuiApplication::primaryScreen());
        info.screen = screen;
        
        m_screens.append(info);
    }
    
    qDebug() << "ScreenShareManager: Enumerated" << m_screens.size() << "screens";
}

void ScreenShareManager::enumerateWindows()
{
    m_windows.clear();
    
    // Windows平台的窗口枚举实现
    // 这里是简化实现，实际需要使用Windows API (EnumWindows)
    
    // 示例：添加一些虚拟窗口用于测试
    WindowInfo info1;
    info1.windowId = 1;
    info1.title = "Test Window 1";
    info1.processName = "test.exe";
    info1.geometry = QRect(100, 100, 800, 600);
    info1.isVisible = true;
    m_windows.append(info1);
    
    WindowInfo info2;
    info2.windowId = 2;
    info2.title = "Test Window 2";
    info2.processName = "app.exe";
    info2.geometry = QRect(200, 200, 1024, 768);
    info2.isVisible = true;
    m_windows.append(info2);
    
    qDebug() << "ScreenShareManager: Enumerated" << m_windows.size() << "windows";
}

void ScreenShareManager::adjustQualityBasedOnPerformance()
{
    if (!m_isScreenSharing && !m_isWindowSharing) {
        return;
    }
    
    if (!m_shareQuality.adaptiveQuality) {
        return;
    }
    
    // 计算当前帧率
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (m_lastFrameTime > 0) {
        qint64 timeDiff = currentTime - m_lastFrameTime;
        if (timeDiff > 0) {
            int actualFrameRate = (m_frameCount * 1000) / timeDiff;
            
            // 如果实际帧率低于目标帧率的80%，降低质量
            if (actualFrameRate < m_shareQuality.frameRate * 0.8) {
                if (m_shareQuality.frameRate > MIN_FRAME_RATE) {
                    m_shareQuality.frameRate = qMax(MIN_FRAME_RATE, m_shareQuality.frameRate - 2);
                    updateCaptureSettings();
                    qDebug() << "ScreenShareManager: Reduced frame rate to" << m_shareQuality.frameRate;
                }
            }
            // 如果实际帧率接近目标帧率，可以尝试提高质量
            else if (actualFrameRate >= m_shareQuality.frameRate * 0.95) {
                if (m_shareQuality.frameRate < MAX_FRAME_RATE) {
                    m_shareQuality.frameRate = qMin(MAX_FRAME_RATE, m_shareQuality.frameRate + 1);
                    updateCaptureSettings();
                    qDebug() << "ScreenShareManager: Increased frame rate to" << m_shareQuality.frameRate;
                }
            }
        }
    }
    
    m_lastFrameTime = currentTime;
    m_frameCount = 0;
}

void ScreenShareManager::updateCaptureSettings()
{
    if (m_captureTimer->isActive()) {
        m_captureTimer->setInterval(1000 / m_shareQuality.frameRate);
    }
}

// ScreenSelectionDialog 实现

ScreenSelectionDialog::ScreenSelectionDialog(const QList<ScreenShareManager::ScreenInfo>& screens,
                                           const QList<ScreenShareManager::WindowInfo>& windows,
                                           QWidget* parent)
    : QDialog(parent)
    , m_screens(screens)
    , m_windows(windows)
    , m_screenList(nullptr)
    , m_windowList(nullptr)
    , m_previewLabel(nullptr)
    , m_shareButton(nullptr)
    , m_cancelButton(nullptr)
    , m_selectedScreenId(-1)
    , m_selectedWindowId(0)
    , m_screenSelected(false)
    , m_windowSelected(false)
{
    setupUI();
    populateScreenList();
    populateWindowList();
}

ScreenSelectionDialog::~ScreenSelectionDialog()
{
}

int ScreenSelectionDialog::selectedScreenId() const
{
    return m_selectedScreenId;
}

qint64 ScreenSelectionDialog::selectedWindowId() const
{
    return m_selectedWindowId;
}

bool ScreenSelectionDialog::isScreenSelected() const
{
    return m_screenSelected;
}

bool ScreenSelectionDialog::isWindowSelected() const
{
    return m_windowSelected;
}

void ScreenSelectionDialog::setupUI()
{
    setWindowTitle("选择屏幕或窗口");
    setModal(true);
    resize(800, 600);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 创建标签页
    QTabWidget* tabWidget = new QTabWidget(this);
    
    // 屏幕选择标签页
    QWidget* screenTab = new QWidget();
    QVBoxLayout* screenLayout = new QVBoxLayout(screenTab);
    
    screenLayout->addWidget(new QLabel("选择要共享的屏幕:"));
    m_screenList = new QListWidget();
    m_screenList->setSelectionMode(QAbstractItemView::SingleSelection);
    screenLayout->addWidget(m_screenList);
    
    tabWidget->addTab(screenTab, "屏幕");
    
    // 窗口选择标签页
    QWidget* windowTab = new QWidget();
    QVBoxLayout* windowLayout = new QVBoxLayout(windowTab);
    
    windowLayout->addWidget(new QLabel("选择要共享的窗口:"));
    m_windowList = new QListWidget();
    m_windowList->setSelectionMode(QAbstractItemView::SingleSelection);
    windowLayout->addWidget(m_windowList);
    
    tabWidget->addTab(windowTab, "窗口");
    
    mainLayout->addWidget(tabWidget);
    
    // 预览区域
    QGroupBox* previewGroup = new QGroupBox("预览");
    QVBoxLayout* previewLayout = new QVBoxLayout(previewGroup);
    
    m_previewLabel = new QLabel();
    m_previewLabel->setMinimumSize(320, 240);
    m_previewLabel->setStyleSheet("QLabel { background-color: black; border: 1px solid #ccc; }");
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setText("选择屏幕或窗口以显示预览");
    
    previewLayout->addWidget(m_previewLabel);
    mainLayout->addWidget(previewGroup);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_shareButton = new QPushButton("开始共享");
    m_shareButton->setEnabled(false);
    m_cancelButton = new QPushButton("取消");
    
    buttonLayout->addWidget(m_shareButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_screenList, &QListWidget::itemClicked, this, &ScreenSelectionDialog::onScreenItemClicked);
    connect(m_windowList, &QListWidget::itemClicked, this, &ScreenSelectionDialog::onWindowItemClicked);
    connect(m_shareButton, &QPushButton::clicked, this, &ScreenSelectionDialog::onShareButtonClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ScreenSelectionDialog::onCancelButtonClicked);
}

void ScreenSelectionDialog::populateScreenList()
{
    m_screenList->clear();
    
    for (const auto& screen : m_screens) {
        QString text = QString("屏幕 %1: %2 (%3x%4)")
                      .arg(screen.screenId)
                      .arg(screen.name)
                      .arg(screen.size.width())
                      .arg(screen.size.height());
        
        if (screen.isPrimary) {
            text += " [主屏幕]";
        }
        
        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, screen.screenId);
        m_screenList->addItem(item);
    }
}

void ScreenSelectionDialog::populateWindowList()
{
    m_windowList->clear();
    
    for (const auto& window : m_windows) {
        if (!window.isVisible) {
            continue;
        }
        
        QString text = QString("%1 - %2 (%3x%4)")
                      .arg(window.title)
                      .arg(window.processName)
                      .arg(window.geometry.width())
                      .arg(window.geometry.height());
        
        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, static_cast<qulonglong>(window.windowId));
        m_windowList->addItem(item);
    }
}

void ScreenSelectionDialog::updatePreview()
{
    // 简化实现：显示选择信息
    QString previewText;
    
    if (m_screenSelected) {
        for (const auto& screen : m_screens) {
            if (screen.screenId == m_selectedScreenId) {
                previewText = QString("屏幕预览\n%1\n%2x%3")
                             .arg(screen.name)
                             .arg(screen.size.width())
                             .arg(screen.size.height());
                break;
            }
        }
    } else if (m_windowSelected) {
        for (const auto& window : m_windows) {
            if (window.windowId == m_selectedWindowId) {
                previewText = QString("窗口预览\n%1\n%2x%3")
                             .arg(window.title)
                             .arg(window.geometry.width())
                             .arg(window.geometry.height());
                break;
            }
        }
    } else {
        previewText = "选择屏幕或窗口以显示预览";
    }
    
    m_previewLabel->setText(previewText);
}

void ScreenSelectionDialog::onScreenItemClicked()
{
    QListWidgetItem* item = m_screenList->currentItem();
    if (item) {
        m_selectedScreenId = item->data(Qt::UserRole).toInt();
        m_screenSelected = true;
        m_windowSelected = false;
        
        // 清除窗口选择
        m_windowList->clearSelection();
        
        m_shareButton->setEnabled(true);
        updatePreview();
    }
}

void ScreenSelectionDialog::onWindowItemClicked()
{
    QListWidgetItem* item = m_windowList->currentItem();
    if (item) {
        m_selectedWindowId = item->data(Qt::UserRole).toULongLong();
        m_windowSelected = true;
        m_screenSelected = false;
        
        // 清除屏幕选择
        m_screenList->clearSelection();
        
        m_shareButton->setEnabled(true);
        updatePreview();
    }
}

void ScreenSelectionDialog::onShareButtonClicked()
{
    accept();
}

void ScreenSelectionDialog::onCancelButtonClicked()
{
    reject();
}

// MOC will be generated automatically