#ifndef CHATMODULE_H
#define CHATMODULE_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <memory>

// Forward declarations
class ChatManager;
class MessageHandler;
class ChatConfig;

/**
 * @brief 聊天模块核心类
 * 
 * ChatModule是聊天功能的核心控制类，负责模块的初始化、
 * 配置管理和生命周期控制。
 */
class ChatModule : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(bool initialized READ isInitialized NOTIFY initializedChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    /**
     * @brief 模块状态枚举
     */
    enum Status {
        NotInitialized,     ///< 未初始化
        Initializing,       ///< 初始化中
        Ready,              ///< 就绪
        Error,              ///< 错误状态
        Shutting_Down       ///< 关闭中
    };
    Q_ENUM(Status)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ChatModule(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ChatModule();

    /**
     * @brief 获取模块版本
     * @return 版本字符串
     */
    QString version() const;

    /**
     * @brief 初始化模块
     * @param config 配置参数
     * @return 初始化是否成功
     */
    bool initialize(const QVariantMap& config = QVariantMap());

    /**
     * @brief 关闭模块
     */
    void shutdown();

    /**
     * @brief 检查模块是否已初始化
     * @return 是否已初始化
     */
    bool isInitialized() const;

    /**
     * @brief 获取模块状态
     * @return 状态字符串
     */
    QString status() const;

    /**
     * @brief 获取模块状态枚举
     * @return 状态枚举值
     */
    Status moduleStatus() const;

    /**
     * @brief 获取聊天管理器
     * @return 聊天管理器指针
     */
    ChatManager* chatManager() const;

    /**
     * @brief 获取消息处理器
     * @return 消息处理器指针
     */
    MessageHandler* messageHandler() const;

    /**
     * @brief 设置模块配置
     * @param config 配置对象
     */
    void setConfiguration(const ChatConfig& config);

    /**
     * @brief 获取模块配置
     * @return 配置对象
     */
    ChatConfig configuration() const;

    /**
     * @brief 重新加载配置
     * @return 重新加载是否成功
     */
    bool reloadConfiguration();

    /**
     * @brief 获取模块信息
     * @return 模块信息映射
     */
    QVariantMap moduleInfo() const;

    /**
     * @brief 获取模块统计信息
     * @return 统计信息映射
     */
    QVariantMap statistics() const;

public slots:
    /**
     * @brief 启动模块
     */
    void start();

    /**
     * @brief 停止模块
     */
    void stop();

    /**
     * @brief 重启模块
     */
    void restart();

    /**
     * @brief 重置模块到默认状态
     */
    void reset();

signals:
    /**
     * @brief 初始化状态改变信号
     * @param initialized 是否已初始化
     */
    void initializedChanged(bool initialized);

    /**
     * @brief 状态改变信号
     * @param status 新状态
     */
    void statusChanged(const QString& status);

    /**
     * @brief 模块状态改变信号
     * @param status 新状态枚举
     */
    void moduleStatusChanged(Status status);

    /**
     * @brief 配置改变信号
     */
    void configurationChanged();

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

    /**
     * @brief 模块启动信号
     */
    void started();

    /**
     * @brief 模块停止信号
     */
    void stopped();

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
     * @brief 设置模块状态
     * @param status 新状态
     */
    void setStatus(Status status);

    /**
     * @brief 验证配置
     * @param config 配置参数
     * @return 配置是否有效
     */
    bool validateConfiguration(const QVariantMap& config);

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // CHATMODULE_H