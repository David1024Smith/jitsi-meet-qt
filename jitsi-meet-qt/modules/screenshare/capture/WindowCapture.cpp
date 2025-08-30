#include "WindowCapture.h"
#include <QApplication>
#include <QWindow>
#include <QPixmap>
#include <QTimer>
#include <QMutexLocker>
#include <QDebug>
#include <QBuffer>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#endif

class WindowCapture::Private
{
public:
    Private()
        : status(IScreenCapture::Inactive)
        , initialized(false)
        , captureMode(IScreenCapture::Window)
        , quality(IScreenCapture::MediumQuality)
        , frameRate(30)
        , captureCursor(false)
        , captureDelay(0)
        , compressionQuality(75)
        , includeWindowFrame(true)
        , targetWindow(nullptr)
        , captureTimer(nullptr)
        , windowId(0)
    {
    }

    IScreenCapture::CaptureStatus status;
    bool initialized;
    IScreenCapture::CaptureMode captureMode;
    IScreenCapture::CaptureQuality quality;
    int frameRate;
    bool captureCursor;
    int captureDelay;
    int compressionQuality;
    bool includeWindowFrame;
    
    QWindow* targetWindow;
    QTimer* captureTimer;
    WId windowId;
    QRect captureRegion;
    
    mutable QMutex mutex;
};

WindowCapture::WindowCapture(QObject *parent)
    : IScreenCapture(parent)
    , d(new Private)
{
    d->captureTimer = new QTimer(this);
    connect(d->captureTimer, &QTimer::timeout,
            this, &WindowCapture::onCaptureTimer);
}

WindowCapture::~WindowCapture()
{
    stopCapture();
    delete d;
}

bool WindowCapture::initialize()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->initialized) {
        return true;
    }
    
    try {
        // 初始化窗口捕获设置
        initializeCapture();
        
        d->initialized = true;
        updateStatus(Inactive);
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "WindowCapture initialization failed:" << e.what();
        return false;
    }
}

bool WindowCapture::startCapture()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        emitError("WindowCapture not initialized");
        return false;
    }
    
    if (d->status == Active) {
        return true;
    }
    
    updateStatus(Initializing);
    
    try {
        // 确保有目标窗口
        if (!d->targetWindow && d->windowId == 0) {
            throw std::runtime_error("No window selected for capture");
        }
        
        // 启动捕获定时器
        updateCaptureTimer();
        d->captureTimer->start();
        
        updateStatus(Active);
        emit captureStarted();
        return true;
        
    } catch (const std::exception& e) {
        emitError(QString("Failed to start window capture: %1").arg(e.what()));
        updateStatus(Error);
        return false;
    }
}

void WindowCapture::stopCapture()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status == Inactive) {
        return;
    }
    
    // 停止定时器
    if (d->captureTimer) {
        d->captureTimer->stop();
    }
    
    updateStatus(Inactive);
    emit captureStopped();
}

void WindowCapture::pauseCapture()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Active) {
        return;
    }
    
    if (d->captureTimer) {
        d->captureTimer->stop();
    }
    
    updateStatus(Paused);
    emit capturePaused();
}

void WindowCapture::resumeCapture()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Paused) {
        return;
    }
    
    if (d->captureTimer) {
        d->captureTimer->start();
    }
    
    updateStatus(Active);
    emit captureResumed();
}

IScreenCapture::CaptureStatus WindowCapture::status() const
{
    QMutexLocker locker(&d->mutex);
    return d->status;
}

bool WindowCapture::isCapturing() const
{
    QMutexLocker locker(&d->mutex);
    return d->status == Active;
}

bool WindowCapture::isInitialized() const
{
    QMutexLocker locker(&d->mutex);
    return d->initialized;
}

void WindowCapture::setCaptureMode(CaptureMode mode)
{
    QMutexLocker locker(&d->mutex);
    // 窗口捕获只支持Window模式
    if (mode == Window) {
        d->captureMode = mode;
    }
}

IScreenCapture::CaptureMode WindowCapture::captureMode() const
{
    QMutexLocker locker(&d->mutex);
    return d->captureMode;
}

void WindowCapture::setCaptureQuality(CaptureQuality quality)
{
    QMutexLocker locker(&d->mutex);
    if (d->quality != quality) {
        d->quality = quality;
        
        // 根据质量调整压缩设置
        switch (quality) {
        case LowQuality:
            d->compressionQuality = 50;
            break;
        case MediumQuality:
            d->compressionQuality = 75;
            break;
        case HighQuality:
            d->compressionQuality = 90;
            break;
        case UltraQuality:
            d->compressionQuality = 100;
            break;
        }
    }
}

IScreenCapture::CaptureQuality WindowCapture::captureQuality() const
{
    QMutexLocker locker(&d->mutex);
    return d->quality;
}

void WindowCapture::setFrameRate(int fps)
{
    QMutexLocker locker(&d->mutex);
    if (fps > 0 && fps <= 120 && d->frameRate != fps) {
        d->frameRate = fps;
        updateCaptureTimer();
    }
}

int WindowCapture::frameRate() const
{
    QMutexLocker locker(&d->mutex);
    return d->frameRate;
}

void WindowCapture::setCaptureRegion(const QRect& region)
{
    QMutexLocker locker(&d->mutex);
    d->captureRegion = region;
}

QRect WindowCapture::captureRegion() const
{
    QMutexLocker locker(&d->mutex);
    return d->captureRegion;
}

void WindowCapture::setTargetScreen(QScreen* screen)
{
    Q_UNUSED(screen)
    // 窗口捕获不使用屏幕设置
}

QScreen* WindowCapture::targetScreen() const
{
    return nullptr;
}

QPixmap WindowCapture::captureFrame()
{
    QMutexLocker locker(&d->mutex);
    
    if (!isCapturing()) {
        return QPixmap();
    }
    
    try {
        QPixmap frame = captureWindowInternal();
        if (!frame.isNull()) {
            frame = applyCaptureQuality(frame);
            emit frameCaptured(frame);
        }
        return frame;
        
    } catch (const std::exception& e) {
        emitError(QString("Window frame capture failed: %1").arg(e.what()));
        return QPixmap();
    }
}

QByteArray WindowCapture::captureFrameData()
{
    QPixmap frame = captureFrame();
    if (frame.isNull()) {
        return QByteArray();
    }
    
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    
    QString format = (d->quality == UltraQuality) ? "PNG" : "JPEG";
    frame.save(&buffer, format.toUtf8().constData(), d->compressionQuality);
    
    return data;
}

QSize WindowCapture::captureSize() const
{
    QMutexLocker locker(&d->mutex);
    
    if (d->targetWindow) {
        return d->targetWindow->size();
    } else if (!d->captureRegion.isEmpty()) {
        return d->captureRegion.size();
    }
    
    return QSize();
}

void WindowCapture::setTargetWindow(QWindow* window)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->targetWindow != window) {
        d->targetWindow = window;
        
        if (window) {
            d->windowId = window->winId();
            
            // 连接窗口信号
            connect(window, &QWindow::widthChanged,
                    this, &WindowCapture::onWindowGeometryChanged, Qt::UniqueConnection);
            connect(window, &QWindow::heightChanged,
                    this, &WindowCapture::onWindowGeometryChanged, Qt::UniqueConnection);
        } else {
            d->windowId = 0;
        }
    }
}

QWindow* WindowCapture::targetWindow() const
{
    QMutexLocker locker(&d->mutex);
    return d->targetWindow;
}

void WindowCapture::setTargetWindowId(WId windowId)
{
    QMutexLocker locker(&d->mutex);
    d->windowId = windowId;
    d->targetWindow = nullptr; // 清除QWindow引用
}

WId WindowCapture::targetWindowId() const
{
    QMutexLocker locker(&d->mutex);
    return d->windowId;
}

void WindowCapture::setIncludeWindowFrame(bool include)
{
    QMutexLocker locker(&d->mutex);
    d->includeWindowFrame = include;
}

bool WindowCapture::includeWindowFrame() const
{
    QMutexLocker locker(&d->mutex);
    return d->includeWindowFrame;
}

QStringList WindowCapture::getAvailableWindows() const
{
    QStringList windows;
    
#ifdef Q_OS_WIN
    // Windows平台实现
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        QStringList* windowList = reinterpret_cast<QStringList*>(lParam);
        
        if (IsWindowVisible(hwnd)) {
            WCHAR windowTitle[256];
            GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle) / sizeof(WCHAR));
            
            QString title = QString::fromWCharArray(windowTitle);
            if (!title.isEmpty()) {
                windowList->append(QString("%1 (ID: %2)").arg(title).arg(reinterpret_cast<qintptr>(hwnd)));
            }
        }
        
        return TRUE;
    }, reinterpret_cast<LPARAM>(&windows));
#else
    // 其他平台的简化实现
    windows.append("Desktop Window (ID: 0)");
#endif
    
    return windows;
}

bool WindowCapture::selectWindowByTitle(const QString& title)
{
    Q_UNUSED(title)
    
#ifdef Q_OS_WIN
    HWND hwnd = FindWindowA(nullptr, title.toLocal8Bit().constData());
    if (hwnd) {
        setTargetWindowId(reinterpret_cast<WId>(hwnd));
        return true;
    }
#endif
    
    return false;
}

void WindowCapture::refreshWindowInfo()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->targetWindow) {
        // 更新窗口信息
        d->captureRegion = QRect(QPoint(0, 0), d->targetWindow->size());
    }
}

void WindowCapture::onCaptureTimer()
{
    if (d->captureDelay > 0) {
        QTimer::singleShot(d->captureDelay, this, [this]() {
            captureFrame();
        });
    } else {
        captureFrame();
    }
}

void WindowCapture::onWindowGeometryChanged()
{
    refreshWindowInfo();
}

void WindowCapture::initializeCapture()
{
    // 初始化窗口捕获设置
    d->captureMode = Window;
}

void WindowCapture::updateCaptureTimer()
{
    if (d->captureTimer && d->frameRate > 0) {
        int interval = 1000 / d->frameRate;
        d->captureTimer->setInterval(interval);
    }
}

void WindowCapture::updateStatus(CaptureStatus newStatus)
{
    if (d->status != newStatus) {
        d->status = newStatus;
        emit statusChanged(newStatus);
    }
}

QPixmap WindowCapture::captureWindowInternal()
{
    QPixmap screenshot;
    
    if (d->targetWindow) {
        // 使用QWindow捕获
        screenshot = d->targetWindow->screen()->grabWindow(d->targetWindow->winId());
    } else if (d->windowId != 0) {
        // 使用窗口ID捕获
#ifdef Q_OS_WIN
        HWND hwnd = reinterpret_cast<HWND>(d->windowId);
        if (IsWindow(hwnd)) {
            RECT rect;
            if (d->includeWindowFrame) {
                GetWindowRect(hwnd, &rect);
            } else {
                GetClientRect(hwnd, &rect);
            }
            
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;
            
            if (width > 0 && height > 0) {
                HDC hdcWindow = GetDC(hwnd);
                HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
                HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, width, height);
                SelectObject(hdcMemDC, hbmScreen);
                
                if (d->includeWindowFrame) {
                    PrintWindow(hwnd, hdcMemDC, PW_RENDERFULLCONTENT);
                } else {
                    BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);
                }
                
                // 转换为QPixmap
                screenshot = QPixmap::fromWinHBITMAP(hbmScreen);
                
                DeleteObject(hbmScreen);
                DeleteDC(hdcMemDC);
                ReleaseDC(hwnd, hdcWindow);
            }
        }
#else
        // 其他平台的简化实现
        auto screens = QApplication::screens();
        if (!screens.isEmpty()) {
            screenshot = screens.first()->grabWindow(d->windowId);
        }
#endif
    }
    
    return screenshot;
}

QPixmap WindowCapture::applyCaptureQuality(const QPixmap& source)
{
    if (source.isNull()) {
        return source;
    }
    
    QPixmap result = source;
    
    // 根据质量设置调整图像
    switch (d->quality) {
    case LowQuality:
        // 降低分辨率
        result = source.scaled(source.size() * 0.5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        break;
    case MediumQuality:
        // 保持原始分辨率
        break;
    case HighQuality:
        // 保持原始分辨率，高质量
        break;
    case UltraQuality:
        // 最高质量
        break;
    }
    
    return result;
}

// 窗口特定接口实现
QString WindowCapture::targetWindowTitle() const
{
    QMutexLocker locker(&d->mutex);
    
    if (d->targetWindow) {
        return d->targetWindow->title();
    }
    
#ifdef Q_OS_WIN
    if (d->windowId != 0) {
        HWND hwnd = reinterpret_cast<HWND>(d->windowId);
        if (IsWindow(hwnd)) {
            WCHAR windowTitle[256];
            GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle) / sizeof(WCHAR));
            return QString::fromWCharArray(windowTitle);
        }
    }
#endif
    
    return QString();
}

void WindowCapture::setTargetWindowTitle(const QString& title)
{
    if (selectWindowByTitle(title)) {
        emit targetWindowChanged(title, d->windowId);
    }
}

qulonglong WindowCapture::targetWindowHandle() const
{
    QMutexLocker locker(&d->mutex);
    return d->windowId;
}

void WindowCapture::setTargetWindowHandle(qulonglong handle)
{
    QMutexLocker locker(&d->mutex);
    if (d->windowId != handle) {
        d->windowId = handle;
        d->targetWindow = nullptr;
        
        QString title = targetWindowTitle();
        emit targetWindowChanged(title, handle);
    }
}

WindowCapture::WindowState WindowCapture::windowState() const
{
    QMutexLocker locker(&d->mutex);
    
    if (d->targetWindow) {
        switch (d->targetWindow->windowState()) {
        case Qt::WindowMinimized:
            return WindowMinimized;
        case Qt::WindowMaximized:
            return WindowMaximized;
        case Qt::WindowFullScreen:
            return WindowFullScreen;
        default:
            return WindowNormal;
        }
    }
    
#ifdef Q_OS_WIN
    if (d->windowId != 0) {
        HWND hwnd = reinterpret_cast<HWND>(d->windowId);
        if (IsWindow(hwnd)) {
            if (IsIconic(hwnd)) return WindowMinimized;
            if (IsZoomed(hwnd)) return WindowMaximized;
            if (!IsWindowVisible(hwnd)) return WindowHidden;
            return WindowNormal;
        }
    }
#endif
    
    return WindowNotFound;
}

QRect WindowCapture::windowGeometry() const
{
    QMutexLocker locker(&d->mutex);
    
    if (d->targetWindow) {
        return d->targetWindow->geometry();
    }
    
#ifdef Q_OS_WIN
    if (d->windowId != 0) {
        HWND hwnd = reinterpret_cast<HWND>(d->windowId);
        if (IsWindow(hwnd)) {
            RECT rect;
            GetWindowRect(hwnd, &rect);
            return QRect(rect.left, rect.top, 
                        rect.right - rect.left, 
                        rect.bottom - rect.top);
        }
    }
#endif
    
    return QRect();
}

bool WindowCapture::isWindowValid() const
{
    QMutexLocker locker(&d->mutex);
    
    if (d->targetWindow) {
        return true;
    }
    
#ifdef Q_OS_WIN
    if (d->windowId != 0) {
        return IsWindow(reinterpret_cast<HWND>(d->windowId));
    }
#endif
    
    return false;
}

QStringList WindowCapture::availableWindows() const
{
    return getAvailableWindows();
}

bool WindowCapture::selectWindowByHandle(qulonglong handle)
{
    setTargetWindowHandle(handle);
    return isWindowValid();
}

bool WindowCapture::selectWindowByProcess(const QString& processName)
{
#ifdef Q_OS_WIN
    HWND hwnd = nullptr;
    
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        QString* targetProcess = reinterpret_cast<QString*>(lParam);
        
        DWORD processId;
        GetWindowThreadProcessId(hwnd, &processId);
        
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
        if (hProcess) {
            WCHAR processPath[MAX_PATH];
            if (GetModuleFileNameExW(hProcess, nullptr, processPath, MAX_PATH)) {
                QString fullPath = QString::fromWCharArray(processPath);
                QString fileName = QFileInfo(fullPath).baseName();
                
                if (fileName.compare(*targetProcess, Qt::CaseInsensitive) == 0) {
                    CloseHandle(hProcess);
                    return FALSE; // 找到了，停止枚举
                }
            }
            CloseHandle(hProcess);
        }
        
        return TRUE;
    }, reinterpret_cast<LPARAM>(&processName));
    
    if (hwnd) {
        setTargetWindowHandle(reinterpret_cast<qulonglong>(hwnd));
        return true;
    }
#endif
    
    return false;
}

void WindowCapture::setFollowWindow(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    d->followWindow = enabled;
}

bool WindowCapture::isFollowWindowEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return d->followWindow;
}

void WindowCapture::setCaptureClientArea(bool clientOnly)
{
    QMutexLocker locker(&d->mutex);
    d->includeWindowFrame = !clientOnly;
}

bool WindowCapture::isCaptureClientAreaEnabled() const
{
    QMutexLocker locker(&d->mutex);
    return !d->includeWindowFrame;
}

void WindowCapture::setAutoResize(bool enabled)
{
    QMutexLocker locker(&d->mutex);
    // 这个功能在当前实现中暂时不支持
    Q_UNUSED(enabled)
}

bool WindowCapture::isAutoResizeEnabled() const
{
    return false; // 当前实现中暂时不支持
}

void WindowCapture::refreshWindowList()
{
    // 刷新可用窗口列表
    getAvailableWindows();
}

void WindowCapture::autoFindWindow()
{
    QStringList windows = availableWindows();
    if (!windows.isEmpty()) {
        // 选择第一个可见窗口
        QString firstWindow = windows.first();
        QRegExp rx("ID: (\\d+)");
        if (rx.indexIn(firstWindow) != -1) {
            qulonglong handle = rx.cap(1).toULongLong();
            setTargetWindowHandle(handle);
        }
    }
}

void WindowCapture::bringWindowToFront()
{
#ifdef Q_OS_WIN
    if (d->windowId != 0) {
        HWND hwnd = reinterpret_cast<HWND>(d->windowId);
        if (IsWindow(hwnd)) {
            SetForegroundWindow(hwnd);
            ShowWindow(hwnd, SW_RESTORE);
        }
    }
#endif
}

void WindowCapture::onWindowMonitorTimer()
{
    WindowState currentState = windowState();
    QRect currentGeometry = windowGeometry();
    
    // 检查窗口状态变化
    static WindowState lastState = WindowNotFound;
    static QRect lastGeometry;
    
    if (currentState != lastState) {
        emit windowStateChanged(currentState);
        lastState = currentState;
    }
    
    if (currentGeometry != lastGeometry) {
        emit windowGeometryChanged(currentGeometry);
        lastGeometry = currentGeometry;
    }
    
    // 检查窗口是否丢失
    if (currentState == WindowNotFound && lastState != WindowNotFound) {
        emit windowLost();
    }
}

void WindowCapture::emitError(const QString& error)
{
    qWarning() << "WindowCapture error:" << error;
    emit captureError(error);
}