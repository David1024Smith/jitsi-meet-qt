#ifndef UTILSMODULE_H
#define UTILSMODULE_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QMutex>

/**
 * @brief 工具模块核心类
 * 
 * UtilsModule是工具模块的核心管理器，负责初始化和管理所有工具组件。
 * 提供统一的工具模块访问接口和生命周期管理。
 */
class UtilsModule : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 模块状态枚举
     */
    enum ModuleStatus {
        NotInitialized,     ///< 未初始化
        Initializing,       ///< 初始化中
        Ready,              ///< 就绪
        Error               ///< 错误状态
    };
    Q_ENUM(ModuleStatus)

    /**
     * @brief 获取工具模块单例实例
     * @return UtilsModule实例指针
     */
    static UtilsModule* instance();

    /**
     * @brief 析构函数
     */
    ~UtilsModule();

    /**
     * @brief 初始化工具模块
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 清理工具模块
     */
    void cleanup();

    /**
     * @brief 获取模块状态
     * @return 当前模块状态
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
     * @brief 获取模块配置
     * @return 配置映射
     */
    QVariantMap configuration() const;

    /**
     * @brief 设置模块配置
     * @param config 配置映射
     */
    void setConfiguration(const QVariantMap& config);

    /**
     * @brief 检查模块是否已初始化
     * @return 是否已初始化
     */
    bool isInitialized() const;

    /**
     * @brief 获取错误信息
     * @return 最后的错误信息
     */
    QString lastError() const;

    /**
     * @brief 获取配置管理器
     * @return UtilsConfig实例指针
     */
    class UtilsConfig* getConfig() const;

    /**
     * @brief 获取单例管理器
     * @return UtilsSingletonManager实例指针
     */
    class UtilsSingletonManager* getSingletonManager() const;

    /**
     * @brief 获取错误处理器
     * @return UtilsErrorHandler实例指针
     */
    class UtilsErrorHandler* getErrorHandler() const;

    /**
     * @brief 重新加载配置
     * @return 重新加载是否成功
     */
    bool reloadConfiguration();

    /**
     * @brief 保存当前配置
     * @return 保存是否成功
     */
    bool saveConfiguration();

    /**
     * @brief 获取模块统计信息
     * @return 统计信息映射
     */
    QVariantMap getModuleStatistics() const;

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
     * @brief 模块清理完成信号
     */
    void cleanedUp();

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit UtilsModule(QObject* parent = nullptr);

    /**
     * @brief 初始化日志系统
     * @return 初始化是否成功
     */
    bool initializeLogging();

    /**
     * @brief 初始化文件系统
     * @return 初始化是否成功
     */
    bool initializeFileSystem();

    /**
     * @brief 初始化加密系统
     * @return 初始化是否成功
     */
    bool initializeCrypto();

    /**
     * @brief 设置模块状态
     * @param status 新状态
     */
    void setStatus(ModuleStatus status);

    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void setError(const QString& error);

private:
    static UtilsModule* s_instance;     ///< 单例实例
    static QMutex s_mutex;              ///< 线程安全互斥锁

    ModuleStatus m_status;              ///< 当前状态
    QString m_lastError;                ///< 最后的错误信息
    QVariantMap m_configuration;        ///< 模块配置
    
    // 禁用拷贝构造和赋值操作
    Q_DISABLE_COPY(UtilsModule)
};

#endif // UTILSMODULE_H