#ifndef WINDOWCAPTURE_H
#define WINDOWCAPTURE_H

#include "../interfaces/IScreenCapture.h"
#include <QWindow>
#include <QTimer>
#include <QMutex>

/**
 * @brief 窗口捕获实现类
 * 
 * 实现IScreenCapture接口，提供特定窗口捕获功能
 */
class WindowCapture : public IScreenCapture
{
    Q_OBJECT
    Q_PROPERTY(QString targetWindowTitle READ targetWindowTitle WRITE setTargetWindowTitle NOTIFY targetWindowChanged)
    Q_PROPERTY(qulonglong targetWindowHandle READ targetWindowHandle WRITE setTargetWindowHandle NOTIFY targetWindowChanged)

public:
    /**
     * @brief 窗口状态枚举
     */
    enum WindowState {
        WindowNormal,       ///< 正常状态
        WindowMinimized,    ///< 最小化状态
        WindowMaximized,    ///< 最大化状态
        WindowFullScreen,   ///< 全屏状态
        WindowHidden,       ///< 隐藏状态
        WindowNotFound      ///< 窗口未找到
    };
    Q_ENUM(WindowState)

    explicit WindowCapture(QObject *parent = nullptr);
    virtual ~WindowCapture();

    // IScreenCapture接口实现
    bool initialize() override;
    bool startCapture() override;
    void stopCapture() override;
    void pauseCapture() override;
    void resumeCapture() override;

    // 状态查询接口
    CaptureStatus status() const override;
    bool isCapturing() const override;
    bool isInitialized() const override;

    // 捕获配置接口
    void setCaptureMode(CaptureMode mode) override;
    CaptureMode captureMode() const override;
    void setCaptureQuality(CaptureQuality quality) override;
    CaptureQuality captureQuality() const override;
    void setFrameRate(int fps) override;
    int frameRate() const override;

    // 捕获区域接口
    void setCaptureRegion(const QRect& region) override;
    QRect captureRegion() const override;
    void setTargetScreen(QScreen* screen) override;
    QScreen* targetScreen() const override;

    // 捕获数据接口
    QPixmap captureFrame() override;
    QByteArray captureFrameData() override;
    QSize captureSize() const override;

    // 窗口特定接口
    QString targetWindowTitle() const;
    void setTargetWindowTitle(const QString& title);
    qulonglong targetWindowHandle() const;
    void setTargetWindowHandle(qulonglong handle);
    WindowState windowState() const;
    QRect windowGeometry() const;
    bool isWindowValid() const;
    
    // 窗口对象接口
    QWindow* targetWindow() const;
    void setTargetWindow(QWindow* window);
    WId targetWindowId() const;
    void setTargetWindowId(WId windowId);
    bool includeWindowFrame() const;
    void setIncludeWindowFrame(bool include);

    // 窗口管理接口
    QStringList availableWindows() const;
    bool selectWindowByTitle(const QString& title);
    bool selectWindowByHandle(qulonglong handle);
    bool selectWindowByProcess(const QString& processName);

    // 扩展功能接口
    void setFollowWindow(bool enabled);
    bool isFollowWindowEnabled() const;
    void setCaptureClientArea(bool clientOnly);
    bool isCaptureClientAreaEnabled() const;
    void setAutoResize(bool enabled);
    bool isAutoResizeEnabled() const;
    
    // 窗口信息接口
    void refreshWindowInfo();
    QStringList getAvailableWindows() const;

public slots:
    /**
     * @brief 刷新窗口列表
     */
    void refreshWindowList();

    /**
     * @brief 自动查找目标窗口
     */
    void autoFindWindow();

    /**
     * @brief 将目标窗口置于前台
     */
    void bringWindowToFront();

signals:
    /**
     * @brief 目标窗口改变信号
     * @param title 窗口标题
     * @param handle 窗口句柄
     */
    void targetWindowChanged(const QString& title, qulonglong handle);

    /**
     * @brief 窗口状态改变信号
     * @param state 新的窗口状态
     */
    void windowStateChanged(WindowState state);

    /**
     * @brief 窗口几何改变信号
     * @param geometry 新的窗口几何
     */
    void windowGeometryChanged(const QRect& geometry);

    /**
     * @brief 窗口丢失信号
     */
    void windowLost();

private slots:
    void onCaptureTimer();
    void onWindowMonitorTimer();
    void onWindowGeometryChanged();

private:
    void initializeCapture();
    void cleanupCapture();
    void updateCaptureTimer();
    void updateStatus(CaptureStatus newStatus);
    QPixmap captureWindowInternal();
    bool findWindowByTitle(const QString& title);
    bool findWindowByHandle(qulonglong handle);
    void monitorWindowState();
    void updateWindowGeometry();
    void emitError(const QString& error);
    QPixmap applyCaptureQuality(const QPixmap& frame);

#ifdef Q_OS_WIN
    QPixmap captureWindowWin32();
    bool isWindowValidWin32() const;
    QRect getWindowGeometryWin32() const;
#endif

#ifdef Q_OS_LINUX
    QPixmap captureWindowX11();
    bool isWindowValidX11() const;
    QRect getWindowGeometryX11() const;
#endif

#ifdef Q_OS_MACOS
    QPixmap captureWindowMacOS();
    bool isWindowValidMacOS() const;
    QRect getWindowGeometryMacOS() const;
#endif

    class Private;
    Private* d;
};

#endif // WINDOWCAPTURE_H