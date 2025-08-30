#ifndef AUDIOMODULE_H
#define AUDIOMODULE_H

#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief 音频模块核心类
 * 
 * AudioModule是音频模块的核心类，负责底层音频控制和设备管理。
 * 该类提供了音频系统的基础功能，包括设备初始化、状态管理和错误处理。
 */
class AudioModule : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 音频模块状态枚举
     */
    enum ModuleStatus {
        Uninitialized,  ///< 未初始化
        Initializing,   ///< 初始化中
        Ready,          ///< 就绪
        Active,         ///< 活动中
        Error,          ///< 错误状态
        Shutdown        ///< 已关闭
    };
    Q_ENUM(ModuleStatus)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit AudioModule(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~AudioModule();

    /**
     * @brief 初始化音频模块
     * @return 初始化成功返回true，否则返回false
     */
    virtual bool initialize();

    /**
     * @brief 关闭音频模块
     */
    virtual void shutdown();

    /**
     * @brief 获取模块状态
     * @return 当前模块状态
     */
    ModuleStatus status() const;

    /**
     * @brief 获取模块版本
     * @return 模块版本字符串
     */
    static QString version();

    /**
     * @brief 获取模块名称
     * @return 模块名称
     */
    static QString moduleName();

    /**
     * @brief 检查模块是否可用
     * @return 模块可用返回true，否则返回false
     */
    bool isAvailable() const;

signals:
    /**
     * @brief 模块状态改变信号
     * @param status 新的状态
     */
    void statusChanged(ModuleStatus status);

    /**
     * @brief 模块错误信号
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

    /**
     * @brief 模块初始化完成信号
     */
    void initialized();

    /**
     * @brief 模块关闭信号
     */
    void shutdownCompleted();

protected:
    /**
     * @brief 设置模块状态
     * @param status 新状态
     */
    void setStatus(ModuleStatus status);

    /**
     * @brief 报告错误
     * @param error 错误信息
     */
    void reportError(const QString &error);

private:
    ModuleStatus m_status;          ///< 当前状态
    QString m_lastError;            ///< 最后的错误信息
    bool m_initialized;             ///< 初始化标志

    /**
     * @brief 内部初始化函数
     * @return 初始化成功返回true
     */
    bool doInitialize();

    /**
     * @brief 内部清理函数
     */
    void doCleanup();
};

#endif // AUDIOMODULE_H