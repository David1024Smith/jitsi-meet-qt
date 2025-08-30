#ifndef IDISPLAYMANAGER_H
#define IDISPLAYMANAGER_H

#include <QObject>
#include <QScreen>
#include <QWindow>
#include <QRect>
#include <QStringList>

/**
 * @brief 显示管理器接口
 * 
 * 定义显示设备和窗口管理的标准接口
 */
class IDisplayManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 显示信息结构
     */
    struct DisplayInfo {
        QString id;             ///< 显示器ID
        QString name;           ///< 显示器名称
        QRect geometry;         ///< 几何信息
        QRect availableGeometry; ///< 可用几何信息
        qreal devicePixelRatio; ///< 设备像素比
        int refreshRate;        ///< 刷新率
        bool isPrimary;         ///< 是否为主显示器
    };

    /**
     * @brief 窗口信息结构
     */
    struct WindowInfo {
        QString id;             ///< 窗口ID
        QString title;          ///< 窗口标题
        QString processName;    ///< 进程名称
        QRect geometry;         ///< 窗口几何信息
        bool isVisible;         ///< 是否可见
        bool isMinimized;       ///< 是否最小化
        bool isMaximized;       ///< 是否最大化
        qulonglong windowHandle; ///< 窗口句柄
    };

    explicit IDisplayManager(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IDisplayManager() = default;

    // 显示器管理接口
    virtual bool initialize() = 0;
    virtual QList<DisplayInfo> availableDisplays() const = 0;
    virtual DisplayInfo primaryDisplay() const = 0;
    virtual DisplayInfo displayById(const QString& id) const = 0;
    virtual QScreen* screenById(const QString& id) const = 0;
    virtual QString currentDisplayId() const = 0;

    // 窗口管理接口
    virtual QList<WindowInfo> availableWindows() const = 0;
    virtual QList<WindowInfo> visibleWindows() const = 0;
    virtual WindowInfo windowById(const QString& id) const = 0;
    virtual WindowInfo windowByHandle(qulonglong handle) const = 0;
    virtual QStringList windowTitles() const = 0;

    // 窗口操作接口
    virtual bool bringWindowToFront(const QString& windowId) = 0;
    virtual bool minimizeWindow(const QString& windowId) = 0;
    virtual bool maximizeWindow(const QString& windowId) = 0;
    virtual bool restoreWindow(const QString& windowId) = 0;
    virtual bool setWindowGeometry(const QString& windowId, const QRect& geometry) = 0;

    // 捕获区域计算接口
    virtual QRect calculateCaptureRegion(const QString& sourceId) const = 0;
    virtual QRect adjustRegionToDisplay(const QRect& region, const QString& displayId) const = 0;
    virtual bool isRegionValid(const QRect& region) const = 0;

    // 系统信息接口
    virtual int totalDisplayCount() const = 0;
    virtual QSize totalDesktopSize() const = 0;
    virtual QRect virtualDesktopGeometry() const = 0;
    virtual qreal systemDevicePixelRatio() const = 0;

signals:
    /**
     * @brief 显示器配置改变信号
     */
    void displayConfigurationChanged();

    /**
     * @brief 新显示器添加信号
     * @param display 新添加的显示器信息
     */
    void displayAdded(const DisplayInfo& display);

    /**
     * @brief 显示器移除信号
     * @param displayId 被移除的显示器ID
     */
    void displayRemoved(const QString& displayId);

    /**
     * @brief 窗口列表更新信号
     */
    void windowListUpdated();

    /**
     * @brief 窗口状态改变信号
     * @param windowId 窗口ID
     * @param window 窗口信息
     */
    void windowStateChanged(const QString& windowId, const WindowInfo& window);

    /**
     * @brief 主显示器改变信号
     * @param display 新的主显示器信息
     */
    void primaryDisplayChanged(const DisplayInfo& display);
};

// 注册元类型
Q_DECLARE_METATYPE(IDisplayManager::DisplayInfo)
Q_DECLARE_METATYPE(IDisplayManager::WindowInfo)

#endif // IDISPLAYMANAGER_H