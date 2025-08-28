#ifndef SCREENSHAREMANAGER_H
#define SCREENSHAREMANAGER_H

#include <QObject>
#include <QVideoWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QPixmap>
#include <QTimer>
#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QWindow>
#include <QRect>
#include <QSize>
#include <QList>
#include <QString>
#include <QMap>
#include <QBuffer>
#include <QImageWriter>
#include <QDesktopWidget>

class WebRTCEngine;

/**
 * @brief ScreenShareManager处理屏幕共享功能
 * 
 * 该类负责：
 * - 屏幕和窗口的枚举和选择
 * - 屏幕内容捕获和编码
 * - 集成WebRTC屏幕流传输
 * - 远程屏幕内容的接收和显示
 */
class ScreenShareManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 屏幕信息结构
     */
    struct ScreenInfo {
        int screenId;
        QString name;
        QSize size;
        QRect geometry;
        bool isPrimary;
        QScreen* screen;
        
        ScreenInfo() : screenId(-1), isPrimary(false), screen(nullptr) {}
        ScreenInfo(int id, const QString& screenName, const QSize& screenSize, 
                  const QRect& screenGeometry, bool primary = false, QScreen* screenPtr = nullptr)
            : screenId(id), name(screenName), size(screenSize), 
              geometry(screenGeometry), isPrimary(primary), screen(screenPtr) {}
    };

    /**
     * @brief 窗口信息结构
     */
    struct WindowInfo {
        qint64 windowId;
        QString title;
        QString processName;
        QRect geometry;
        bool isVisible;
        
        WindowInfo() : windowId(0), isVisible(false) {}
        WindowInfo(qint64 id, const QString& windowTitle, const QString& process,
                  const QRect& windowGeometry, bool visible = true)
            : windowId(id), title(windowTitle), processName(process),
              geometry(windowGeometry), isVisible(visible) {}
    };

    /**
     * @brief 屏幕共享质量设置
     */
    struct ShareQuality {
        QSize resolution;
        int frameRate;
        int bitrate;
        bool adaptiveQuality;
        
        ShareQuality() 
            : resolution(1920, 1080), frameRate(15), bitrate(2000000), adaptiveQuality(true) {}
    };

    explicit ScreenShareManager(QObject *parent = nullptr);
    ~ScreenShareManager();

    // 屏幕和窗口枚举
    QList<ScreenInfo> availableScreens() const;
    QList<WindowInfo> availableWindows() const;
    void refreshScreenList();
    void refreshWindowList();
    
    // 屏幕共享控制
    bool startScreenShare(int screenId = -1);
    bool startWindowShare(qint64 windowId);
    void stopScreenShare();
    
    // 屏幕共享状态
    bool isScreenSharing() const;
    bool isWindowSharing() const;
    ScreenInfo currentScreen() const;
    WindowInfo currentWindow() const;
    
    // 屏幕选择对话框
    bool showScreenSelectionDialog();
    
    // 质量设置
    void setShareQuality(const ShareQuality& quality);
    ShareQuality shareQuality() const;
    
    // 远程屏幕共享
    void addRemoteScreenShare(const QString& participantId, QVideoWidget* widget);
    void removeRemoteScreenShare(const QString& participantId);
    QVideoWidget* remoteScreenShareWidget(const QString& participantId) const;
    QList<QString> remoteScreenShareParticipants() const;
    
    // 本地屏幕共享预览
    QVideoWidget* localScreenShareWidget() const;
    
    // WebRTC集成
    void setWebRTCEngine(WebRTCEngine* engine);
    WebRTCEngine* webRTCEngine() const;

signals:
    // 屏幕共享事件
    void screenShareStarted();
    void screenShareStopped();
    void windowShareStarted();
    void windowShareStopped();
    
    // 远程屏幕共享事件
    void remoteScreenShareReceived(const QString& participantId, QVideoWidget* widget);
    void remoteScreenShareRemoved(const QString& participantId);
    
    // 屏幕和窗口列表变化
    void screenListChanged();
    void windowListChanged();
    
    // 错误事件
    void screenCaptureError(const QString& error);
    void windowCaptureError(const QString& error);
    void encodingError(const QString& error);

private slots:
    void onCaptureTimer();
    void onScreenChanged();
    void onWindowChanged();

private:
    // 初始化和清理
    void initializeCapture();
    void cleanupCapture();
    
    // 屏幕捕获
    void setupScreenCapture(int screenId);
    void setupWindowCapture(qint64 windowId);
    void captureCurrentFrame();
    QPixmap captureScreen(const ScreenInfo& screen);
    QPixmap captureWindow(const WindowInfo& window);
    
    // 编码和传输
    QByteArray encodeFrame(const QPixmap& frame);
    void sendFrameToWebRTC(const QByteArray& frameData);
    
    // 远程屏幕共享处理
    void processRemoteFrame(const QString& participantId, const QByteArray& frameData);
    
    // 屏幕和窗口枚举实现
    void enumerateScreens();
    void enumerateWindows();
    
    // 质量自适应
    void adjustQualityBasedOnPerformance();
    void updateCaptureSettings();
    
    // 捕获定时器
    QTimer* m_captureTimer;
    
    // 当前共享状态
    bool m_isScreenSharing;
    bool m_isWindowSharing;
    ScreenInfo m_currentScreen;
    WindowInfo m_currentWindow;
    
    // 屏幕和窗口列表
    QList<ScreenInfo> m_screens;
    QList<WindowInfo> m_windows;
    
    // 本地屏幕共享显示
    QVideoWidget* m_localScreenShareWidget;
    
    // 远程屏幕共享
    QMap<QString, QVideoWidget*> m_remoteScreenShareWidgets;
    
    // 质量设置
    ShareQuality m_shareQuality;
    
    // WebRTC集成
    WebRTCEngine* m_webrtcEngine;
    
    // 性能监控
    QTimer* m_performanceTimer;
    int m_frameCount;
    qint64 m_lastFrameTime;
    
    // 常量
    static const int DEFAULT_FRAME_RATE = 15;
    static const int DEFAULT_BITRATE = 2000000;
    static const int PERFORMANCE_CHECK_INTERVAL = 5000; // 5秒
    static const int MIN_FRAME_RATE = 5;
    static const int MAX_FRAME_RATE = 30;
};

/**
 * @brief 屏幕选择对话框
 */
class ScreenSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScreenSelectionDialog(const QList<ScreenShareManager::ScreenInfo>& screens,
                                  const QList<ScreenShareManager::WindowInfo>& windows,
                                  QWidget* parent = nullptr);
    ~ScreenSelectionDialog();
    
    int selectedScreenId() const;
    qint64 selectedWindowId() const;
    bool isScreenSelected() const;
    bool isWindowSelected() const;

private slots:
    void onScreenItemClicked();
    void onWindowItemClicked();
    void onShareButtonClicked();
    void onCancelButtonClicked();

private:
    void setupUI();
    void populateScreenList();
    void populateWindowList();
    void updatePreview();
    
    QList<ScreenShareManager::ScreenInfo> m_screens;
    QList<ScreenShareManager::WindowInfo> m_windows;
    
    QListWidget* m_screenList;
    QListWidget* m_windowList;
    QLabel* m_previewLabel;
    QPushButton* m_shareButton;
    QPushButton* m_cancelButton;
    
    int m_selectedScreenId;
    qint64 m_selectedWindowId;
    bool m_screenSelected;
    bool m_windowSelected;
};

#endif // SCREENSHAREMANAGER_H