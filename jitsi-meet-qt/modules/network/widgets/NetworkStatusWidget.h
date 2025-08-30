#ifndef NETWORKSTATUSWIDGET_H
#define NETWORKSTATUSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

// 前向声明
class INetworkManager;

/**
 * @brief 网络状态显示组件
 * 
 * NetworkStatusWidget提供网络连接状态的可视化显示，包括连接状态、
 * 网络质量、延迟、带宽等信息的实时更新。
 */
class NetworkStatusWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate NOTIFY autoUpdateChanged)
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)

public:
    /**
     * @brief 显示模式枚举
     */
    enum DisplayMode {
        Compact,            ///< 紧凑模式
        Detailed,           ///< 详细模式
        Minimal             ///< 最小模式
    };
    Q_ENUM(DisplayMode)

    /**
     * @brief 构造函数
     * @param parent 父组件
     */
    explicit NetworkStatusWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~NetworkStatusWidget();

    /**
     * @brief 设置网络管理器
     * @param manager 网络管理器指针
     */
    void setNetworkManager(INetworkManager* manager);

    /**
     * @brief 获取网络管理器
     * @return 网络管理器指针
     */
    INetworkManager* networkManager() const;

    /**
     * @brief 设置显示模式
     * @param mode 显示模式
     */
    void setDisplayMode(DisplayMode mode);

    /**
     * @brief 获取显示模式
     * @return 当前显示模式
     */
    DisplayMode displayMode() const;

    /**
     * @brief 设置是否自动更新
     * @param enabled 是否启用自动更新
     */
    void setAutoUpdate(bool enabled);

    /**
     * @brief 获取是否自动更新
     * @return 是否启用自动更新
     */
    bool autoUpdate() const;

    /**
     * @brief 设置更新间隔
     * @param interval 更新间隔（毫秒）
     */
    void setUpdateInterval(int interval);

    /**
     * @brief 获取更新间隔
     * @return 更新间隔（毫秒）
     */
    int updateInterval() const;

    /**
     * @brief 设置是否显示详细信息
     * @param show 是否显示
     */
    void setShowDetails(bool show);

    /**
     * @brief 获取是否显示详细信息
     * @return 是否显示详细信息
     */
    bool showDetails() const;

public slots:
    /**
     * @brief 刷新网络状态显示
     */
    void refreshStatus();

    /**
     * @brief 开始自动更新
     */
    void startAutoUpdate();

    /**
     * @brief 停止自动更新
     */
    void stopAutoUpdate();

    /**
     * @brief 重置显示
     */
    void reset();

signals:
    /**
     * @brief 自动更新设置改变信号
     * @param enabled 是否启用
     */
    void autoUpdateChanged(bool enabled);

    /**
     * @brief 更新间隔改变信号
     * @param interval 新的间隔
     */
    void updateIntervalChanged(int interval);

    /**
     * @brief 网络状态点击信号
     */
    void statusClicked();

    /**
     * @brief 详细信息请求信号
     */
    void detailsRequested();

protected:
    /**
     * @brief 鼠标点击事件
     * @param event 鼠标事件
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief 绘制事件
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief 尺寸提示
     * @return 建议尺寸
     */
    QSize sizeHint() const override;

private slots:
    /**
     * @brief 处理连接状态改变
     * @param state 新的连接状态
     */
    void handleConnectionStateChanged(int state);

    /**
     * @brief 处理网络质量改变
     * @param quality 新的网络质量
     */
    void handleNetworkQualityChanged(int quality);

    /**
     * @brief 处理网络统计更新
     * @param stats 统计信息
     */
    void handleNetworkStatsUpdated(const QVariantMap& stats);

    /**
     * @brief 处理自动更新定时器
     */
    void handleUpdateTimer();

private:
    /**
     * @brief 初始化UI
     */
    void initializeUI();

    /**
     * @brief 创建紧凑模式UI
     */
    void createCompactUI();

    /**
     * @brief 创建详细模式UI
     */
    void createDetailedUI();

    /**
     * @brief 创建最小模式UI
     */
    void createMinimalUI();

    /**
     * @brief 更新连接状态显示
     * @param state 连接状态
     */
    void updateConnectionStatus(int state);

    /**
     * @brief 更新网络质量显示
     * @param quality 网络质量
     */
    void updateNetworkQuality(int quality);

    /**
     * @brief 更新统计信息显示
     * @param stats 统计信息
     */
    void updateStatistics(const QVariantMap& stats);

    /**
     * @brief 获取连接状态文本
     * @param state 连接状态
     * @return 状态文本
     */
    QString getConnectionStateText(int state) const;

    /**
     * @brief 获取网络质量文本
     * @param quality 网络质量
     * @return 质量文本
     */
    QString getNetworkQualityText(int quality) const;

    /**
     * @brief 获取连接状态颜色
     * @param state 连接状态
     * @return 状态颜色
     */
    QColor getConnectionStateColor(int state) const;

    /**
     * @brief 获取网络质量颜色
     * @param quality 网络质量
     * @return 质量颜色
     */
    QColor getNetworkQualityColor(int quality) const;

    /**
     * @brief 应用样式
     */
    void applyStyles();

    class Private;
    Private* d;
};

#endif // NETWORKSTATUSWIDGET_H