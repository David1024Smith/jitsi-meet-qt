#ifndef MEETINGMODULE_H
#define MEETINGMODULE_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <memory>

class MeetingManager;
class LinkHandler;
class MeetingConfig;

/**
 * @brief 会议模块核心类
 * 
 * 提供会议模块的底层控制和管理功能，负责模块的初始化、配置和生命周期管理
 */
class MeetingModule : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 模块状态枚举
     */
    enum ModuleStatus {
        Uninitialized,   ///< 未初始化
        Initializing,    ///< 初始化中
        Ready,           ///< 就绪
        Active,          ///< 活跃
        Error,           ///< 错误
        Shutdown         ///< 关闭
    };
    Q_ENUM(ModuleStatus)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MeetingModule(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MeetingModule();

    /**
     * @brief 获取模块单例实例
     * @return 模块实例指针
     */
    static MeetingModule* instance();

    /**
     * @brief 初始化模块
     * @param config 初始化配置
     * @return 初始化是否成功
     */
    bool initialize(const QVariantMap& config = QVariantMap());

    /**
     * @brief 关闭模块
     */
    void shutdown();

    /**
     * @brief 获取模块状态
     * @return 当前状态
     */
    ModuleStatus status() const;

    /**
     * @brief 获取模块版本
     * @return 版本字符串
     */
    QString version() const;

    /**
     * @brief 获取模块名称
     * @return 模块名称
     */
    QString moduleName() const;

    /**
     * @brief 获取会议管理器
     * @return 会议管理器指针
     */
    MeetingManager* meetingManager() const;

    /**
     * @brief 获取链接处理器
     * @return 链接处理器指针
     */
    LinkHandler* linkHandler() const;

    /**
     * @brief 获取模块配置
     * @return 配置对象指针
     */
    MeetingConfig* config() const;

    /**
     * @brief 设置模块配置
     * @param config 配置映射
     */
    void setConfiguration(const QVariantMap& config);

    /**
     * @brief 获取模块配置
     * @return 配置映射
     */
    QVariantMap getConfiguration() const;

    /**
     * @brief 重新加载配置
     * @return 重新加载是否成功
     */
    bool reloadConfiguration();

    /**
     * @brief 验证模块依赖
     * @return 依赖是否满足
     */
    bool validateDependencies() const;

    /**
     * @brief 获取模块统计信息
     * @return 统计信息映射
     */
    QVariantMap getStatistics() const;

    /**
     * @brief 重置模块状态
     */
    void reset();

    /**
     * @brief 检查模块健康状态
     * @return 健康状态信息
     */
    QVariantMap healthCheck() const;

signals:
    /**
     * @brief 模块状态改变信号
     * @param status 新状态
     */
    void statusChanged(ModuleStatus status);

    /**
     * @brief 模块初始化完成信号
     * @param success 是否成功
     */
    void initialized(bool success);

    /**
     * @brief 模块关闭完成信号
     */
    void shutdownCompleted();

    /**
     * @brief 配置改变信号
     * @param config 新配置
     */
    void configurationChanged(const QVariantMap& config);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private slots:
    /**
     * @brief 处理内部错误
     * @param error 错误信息
     */
    void handleInternalError(const QString& error);

private:
    /**
     * @brief 初始化组件
     * @return 初始化是否成功
     */
    bool initializeComponents();

    /**
     * @brief 清理资源
     */
    void cleanup();

    /**
     * @brief 设置状态
     * @param status 新状态
     */
    void setStatus(ModuleStatus status);

private:
    class Private;
    std::unique_ptr<Private> d;
    
    static MeetingModule* s_instance;
};

#endif // MEETINGMODULE_H