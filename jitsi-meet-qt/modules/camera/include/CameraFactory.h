#ifndef CAMERAFACTORY_H
#define CAMERAFACTORY_H

#include <QObject>
#include <QString>
#include <QMap>
#include <memory>
#include "../interfaces/ICameraManager.h"

// 前向声明
class CameraManager;
class CameraModule;

/**
 * @brief 摄像头工厂 - 创建和管理摄像头实例
 * 
 * 这个工厂类提供：
 * - 摄像头管理器的创建和配置
 * - 预定义的摄像头配置模板
 * - 摄像头实例的生命周期管理
 * - 全局摄像头设置
 */
class CameraFactory : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 摄像头类型
     */
    enum CameraType {
        LocalCamera,        // 本地摄像头
        RemoteCamera,       // 远程摄像头
        ScreenShareCamera,  // 屏幕共享摄像头
        VirtualCamera       // 虚拟摄像头
    };

    /**
     * @brief 摄像头配置模板
     */
    struct CameraTemplate {
        QString name;                           // 模板名称
        QString description;                    // 模板描述
        bool autoStart;                        // 是否自动启动
        bool enableRecovery;                   // 是否启用恢复
    };

    /**
     * @brief 获取工厂单例
     */
    static CameraFactory* instance();

    /**
     * @brief 销毁工厂单例
     */
    static void destroyInstance();

    // === 摄像头管理器创建 ===
    /**
     * @brief 创建摄像头管理器
     * @param type 摄像头类型
     * @param name 实例名称
     * @return 摄像头管理器指针
     */
    CameraManager* createManager(CameraType type, const QString& name = QString());

    /**
     * @brief 创建带模板的摄像头管理器
     * @param templateName 模板名称
     * @param name 实例名称
     * @return 摄像头管理器指针
     */
    CameraManager* createManagerWithTemplate(const QString& templateName, const QString& name = QString());

    /**
     * @brief 获取摄像头管理器
     * @param name 实例名称
     * @return 摄像头管理器指针
     */
    CameraManager* getManager(const QString& name);

    /**
     * @brief 销毁摄像头管理器
     * @param name 实例名称
     */
    void destroyManager(const QString& name);

    /**
     * @brief 获取所有管理器名称
     */
    QStringList getManagerNames() const;

    // === 模板管理 ===
    /**
     * @brief 注册摄像头模板
     */
    void registerTemplate(const QString& name, const CameraTemplate& tmpl);

    /**
     * @brief 获取摄像头模板
     */
    CameraTemplate getTemplate(const QString& name) const;

    /**
     * @brief 获取所有模板名称
     */
    QStringList getTemplateNames() const;

    /**
     * @brief 移除摄像头模板
     */
    void removeTemplate(const QString& name);

    // === 全局设置 ===
    /**
     * @brief 设置默认摄像头类型
     */
    void setDefaultCameraType(CameraType type);

    /**
     * @brief 获取默认摄像头类型
     */
    CameraType defaultCameraType() const;

    /**
     * @brief 设置全局自动恢复
     */
    void setGlobalAutoRecovery(bool enable);

    /**
     * @brief 获取全局自动恢复设置
     */
    bool globalAutoRecovery() const;

    /**
     * @brief 设置全局监控
     */
    void setGlobalMonitoring(bool enable);

    /**
     * @brief 获取全局监控设置
     */
    bool globalMonitoring() const;

    // === 便捷方法 ===
    /**
     * @brief 创建本地摄像头管理器
     */
    CameraManager* createLocalCamera(const QString& name = "local");

    /**
     * @brief 创建远程摄像头管理器
     */
    CameraManager* createRemoteCamera(const QString& name);

    /**
     * @brief 创建屏幕共享摄像头管理器
     */
    CameraManager* createScreenShareCamera(const QString& name = "screenshare");

    /**
     * @brief 获取本地摄像头管理器
     */
    CameraManager* localCamera();

    /**
     * @brief 获取屏幕共享摄像头管理器
     */
    CameraManager* screenShareCamera();

    // === ICameraManager接口便捷方法 ===
    /**
     * @brief 创建本地摄像头管理器（返回接口）
     */
    ICameraManager* createLocalCameraInterface(const QString& name = "local");

    /**
     * @brief 创建远程摄像头管理器（返回接口）
     */
    ICameraManager* createRemoteCameraInterface(const QString& name);

    /**
     * @brief 销毁摄像头管理器（通过接口）
     */
    void destroyCamera(ICameraManager* camera);

signals:
    /**
     * @brief 管理器创建
     */
    void managerCreated(const QString& name, CameraManager* manager);

    /**
     * @brief 管理器销毁
     */
    void managerDestroyed(const QString& name);

    /**
     * @brief 模板注册
     */
    void templateRegistered(const QString& name);

    /**
     * @brief 模板移除
     */
    void templateRemoved(const QString& name);

private:
    explicit CameraFactory(QObject *parent = nullptr);
    ~CameraFactory();

    /**
     * @brief 初始化默认模板
     */
    void initializeDefaultTemplates();

    /**
     * @brief 生成唯一名称
     */
    QString generateUniqueName(const QString& prefix) const;

    /**
     * @brief 应用全局设置到管理器
     */
    void applyGlobalSettings(CameraManager* manager);

private:
    static CameraFactory* s_instance;                           // 单例实例

    QMap<QString, CameraManager*> m_managers;                   // 管理器实例
    QMap<QString, CameraTemplate> m_templates;                  // 摄像头模板

    // 全局设置
    CameraType m_defaultType;                                   // 默认摄像头类型
    bool m_globalAutoRecovery;                                  // 全局自动恢复
    bool m_globalMonitoring;                                    // 全局监控
    int m_instanceCounter;                                      // 实例计数器
};

#endif // CAMERAFACTORY_H