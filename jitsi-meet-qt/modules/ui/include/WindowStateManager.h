#ifndef WINDOWSTATEMANAGER_H
#define WINDOWSTATEMANAGER_H

#include <QObject>
#include <QWidget>
#include <QSize>
#include <QRect>
#include <QMap>
#include <QTimer>

/**
 * @brief 窗口状态管理器
 * 
 * WindowStateManager负责管理窗口状态，包括响应式布局、
 * 窗口大小变化监听和自适应调整。
 */
class WindowStateManager : public QObject
{
    Q_OBJECT

public:
    enum WindowState {
        Normal,
        Minimized,
        Maximized,
        FullScreen
    };
    Q_ENUM(WindowState)

    enum ScreenSize {
        ExtraSmall,  // < 600px
        Small,       // 600-800px
        Medium,      // 800-1200px
        Large,       // 1200-1600px
        ExtraLarge   // > 1600px
    };
    Q_ENUM(ScreenSize)

    explicit WindowStateManager(QObject *parent = nullptr);
    ~WindowStateManager();

    // 窗口管理
    bool registerWindow(QWidget* window, const QString& name = QString());
    bool unregisterWindow(QWidget* window);
    QWidget* getWindow(const QString& name) const;
    QStringList registeredWindows() const;

    // 状态管理
    WindowState getWindowState(QWidget* window) const;
    bool setWindowState(QWidget* window, WindowState state);
    
    // 尺寸管理
    QSize getWindowSize(QWidget* window) const;
    bool setWindowSize(QWidget* window, const QSize& size);
    ScreenSize getScreenSize(QWidget* window) const;
    ScreenSize getScreenSize(const QSize& size) const;

    // 响应式设计
    bool enableResponsiveDesign(QWidget* window, bool enabled = true);
    bool isResponsiveDesignEnabled(QWidget* window) const;
    void updateResponsiveLayout(QWidget* window);
    void updateAllResponsiveLayouts();

    // 配置
    void setResizeThrottleInterval(int milliseconds);
    int resizeThrottleInterval() const;

signals:
    void windowRegistered(QWidget* window, const QString& name);
    void windowUnregistered(QWidget* window);
    void windowStateChanged(QWidget* window, WindowState state);
    void windowSizeChanged(QWidget* window, const QSize& size);
    void screenSizeChanged(QWidget* window, ScreenSize size);
    void responsiveLayoutUpdated(QWidget* window);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onResizeThrottleTimeout();

private:
    struct WindowInfo {
        QString name;
        QWidget* widget;
        WindowState state;
        QSize size;
        ScreenSize screenSize;
        bool responsiveEnabled;
        QTimer* resizeThrottle;
    };

    void setupWindow(QWidget* window);
    void cleanupWindow(QWidget* window);
    void updateWindowInfo(QWidget* window);
    ScreenSize calculateScreenSize(const QSize& size) const;
    QString generateWindowName(QWidget* window) const;

    QMap<QWidget*, WindowInfo> m_windows;
    QMap<QString, QWidget*> m_windowsByName;
    int m_throttleInterval;
};

#endif // WINDOWSTATEMANAGER_H