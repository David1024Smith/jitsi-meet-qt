#ifndef NETWORKMODULE_H
#define NETWORKMODULE_H

#include <QObject>
#include <QString>
#include <QVariantMap>

/**
 * @brief 网络模块核心类
 * 
 * NetworkModule是网络模块的核心类，负责底层网络控制和模块生命周期管理。
 * 它提供了网络模块的基础功能，包括初始化、配置管理和状态监控。
 */
class NetworkModule : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 模块状态枚举
     */
    enum ModuleStatus {
        NotInitialized,     ///< 未初始化
        Initializing,       ///< 初始化中
        Ready,              ///< 就绪状态
        Error,              ///< 错误状态
        Shutdown            ///< 已关闭
    };
    Q_ENUM(ModuleStatus)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit NetworkModule(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~NetworkModule();

    /**
     * @brief 获取模块单例实例
     * @return NetworkModule实例指针
     */
    static NetworkModule* instance();

    /**
     * @brief 初始化网络模块
     * @param config 配置参数
     * @return 初始化是否成功
     */
    bool initialize(const QVariantMap& config = QVariantMap());

    /**
     * @brief 关闭网络模块
     */
    void shutdown();

    /**
     * @brief 获取模块状态
     * @return 当前模块状态
     */
    ModuleStatus status() const;

    /**
     * @brief 获取模块名称
     * @return 模块名称
     */
    QString moduleName() const;

    /**
     * @brief 获取模块版本
     * @return 模块版本字符串
     */
    QString moduleVersion() const;

    /**
     * @brief 检查模块是否已初始化
     * @return 是否已初始化
     */
    bool isInitialized() const;

    /**
     * @brief 获取模块配置
     * @return 配置参数映射
     */
    QVariantMap configuration() const;

    /**
     * @brief 设置模块配置
     * @param config 配置参数
     */
    void setConfiguration(const QVariantMap& config);

signals:
    /**
     * @brief 模块状态改变信号
     * @param status 新的状态
     */
    void statusChanged(ModuleStatus status);

    /**
     * @brief 模块初始化完成信号
     */
    void initialized();

    /**
     * @brief 模块关闭信号
     */
    void shutdownCompleted();

    /**
     * @brief 模块错误信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private slots:
    /**
     * @brief 处理内部状态变化
     */
    void handleStatusChange();

private:
    /**
     * @brief 执行实际的初始化工作
     * @return 初始化是否成功
     */
    bool doInitialize();

    /**
     * @brief 执行实际的关闭工作
     */
    void doShutdown();

    /**
     * @brief 验证配置参数
     * @param config 配置参数
     * @return 配置是否有效
     */
    bool validateConfiguration(const QVariantMap& config);

    class Private;
    Private* d;
};

#endif // NETWORKMODULE_H