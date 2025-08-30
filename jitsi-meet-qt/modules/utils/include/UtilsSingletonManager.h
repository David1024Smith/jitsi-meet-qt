#ifndef UTILSSINGLETONMANAGER_H
#define UTILSSINGLETONMANAGER_H

#include <QObject>
#include <QMutex>
#include <QMap>
#include <QString>
#include <QVariant>
#include <memory>

// 前向声明
class Logger;
class FileManager;
class UtilsConfig;

/**
 * @brief 工具模块单例管理器
 * 
 * UtilsSingletonManager负责管理工具模块中所有单例对象的生命周期，
 * 提供统一的单例访问接口和资源管理功能。
 */
class UtilsSingletonManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 单例类型枚举
     */
    enum SingletonType {
        LoggerSingleton,        ///< 日志管理器
        FileManagerSingleton,   ///< 文件管理器
        ConfigSingleton,        ///< 配置管理器
        CryptoManagerSingleton, ///< 加密管理器
        StringUtilsSingleton,   ///< 字符串工具
        ValidatorSingleton      ///< 验证器
    };
    Q_ENUM(SingletonType)

    /**
     * @brief 获取单例管理器实例
     * @return UtilsSingletonManager实例指针
     */
    static UtilsSingletonManager* instance();

    /**
     * @brief 析构函数
     */
    ~UtilsSingletonManager();

    /**
     * @brief 初始化所有单例对象
     * @return 初始化是否成功
     */
    bool initializeAll();

    /**
     * @brief 清理所有单例对象
     */
    void cleanupAll();

    /**
     * @brief 获取Logger单例
     * @return Logger实例指针
     */
    Logger* getLogger();

    /**
     * @brief 获取FileManager单例
     * @return FileManager实例指针
     */
    FileManager* getFileManager();

    /**
     * @brief 获取UtilsConfig单例
     * @return UtilsConfig实例指针
     */
    UtilsConfig* getConfig();

    /**
     * @brief 检查单例是否已初始化
     * @param type 单例类型
     * @return 是否已初始化
     */
    bool isSingletonInitialized(SingletonType type) const;

    /**
     * @brief 获取所有已初始化的单例类型
     * @return 单例类型列表
     */
    QList<SingletonType> getInitializedSingletons() const;

    /**
     * @brief 重新初始化指定单例
     * @param type 单例类型
     * @return 重新初始化是否成功
     */
    bool reinitializeSingleton(SingletonType type);

    /**
     * @brief 获取单例的状态信息
     * @param type 单例类型
     * @return 状态信息映射
     */
    QVariantMap getSingletonStatus(SingletonType type) const;

    /**
     * @brief 获取所有单例的状态信息
     * @return 状态信息映射
     */
    QVariantMap getAllSingletonStatus() const;

    /**
     * @brief 设置单例初始化参数
     * @param type 单例类型
     * @param parameters 初始化参数
     */
    void setSingletonParameters(SingletonType type, const QVariantMap& parameters);

    /**
     * @brief 获取单例初始化参数
     * @param type 单例类型
     * @return 初始化参数
     */
    QVariantMap getSingletonParameters(SingletonType type) const;

    /**
     * @brief 启用/禁用单例
     * @param type 单例类型
     * @param enabled 是否启用
     */
    void setSingletonEnabled(SingletonType type, bool enabled);

    /**
     * @brief 检查单例是否启用
     * @param type 单例类型
     * @return 是否启用
     */
    bool isSingletonEnabled(SingletonType type) const;

    /**
     * @brief 获取单例类型的字符串表示
     * @param type 单例类型
     * @return 类型字符串
     */
    static QString singletonTypeToString(SingletonType type);

    /**
     * @brief 从字符串获取单例类型
     * @param typeName 类型字符串
     * @return 单例类型
     */
    static SingletonType stringToSingletonType(const QString& typeName);

signals:
    /**
     * @brief 单例初始化完成信号
     * @param type 单例类型
     */
    void singletonInitialized(SingletonType type);

    /**
     * @brief 单例清理完成信号
     * @param type 单例类型
     */
    void singletonCleaned(SingletonType type);

    /**
     * @brief 所有单例初始化完成信号
     */
    void allSingletonsInitialized();

    /**
     * @brief 所有单例清理完成信号
     */
    void allSingletonsCleaned();

    /**
     * @brief 单例错误信号
     * @param type 单例类型
     * @param error 错误信息
     */
    void singletonError(SingletonType type, const QString& error);

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit UtilsSingletonManager(QObject* parent = nullptr);

    /**
     * @brief 初始化指定单例
     * @param type 单例类型
     * @return 初始化是否成功
     */
    bool initializeSingleton(SingletonType type);

    /**
     * @brief 清理指定单例
     * @param type 单例类型
     */
    void cleanupSingleton(SingletonType type);

    /**
     * @brief 创建单例实例
     * @param type 单例类型
     * @return 创建是否成功
     */
    bool createSingletonInstance(SingletonType type);

    /**
     * @brief 销毁单例实例
     * @param type 单例类型
     */
    void destroySingletonInstance(SingletonType type);

private:
    static UtilsSingletonManager* s_instance;  ///< 单例实例
    static QMutex s_mutex;                     ///< 线程安全互斥锁

    QMap<SingletonType, bool> m_initializedSingletons;  ///< 已初始化的单例
    QMap<SingletonType, bool> m_enabledSingletons;      ///< 启用的单例
    QMap<SingletonType, QVariantMap> m_singletonParameters; ///< 单例参数
    QMap<SingletonType, QString> m_singletonErrors;     ///< 单例错误信息

    // 单例实例指针
    Logger* m_logger;
    FileManager* m_fileManager;
    UtilsConfig* m_config;

    // 禁用拷贝构造和赋值操作
    Q_DISABLE_COPY(UtilsSingletonManager)
};

#endif // UTILSSINGLETONMANAGER_H