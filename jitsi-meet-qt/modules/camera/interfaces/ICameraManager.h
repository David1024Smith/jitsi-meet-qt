#ifndef ICAMERAMANAGER_H
#define ICAMERAMANAGER_H

#include <QObject>
#include <QStringList>
#include <QVideoWidget>
#include "ICameraDevice.h"

/**
 * @brief 摄像头管理器接口
 * 
 * 定义了摄像头管理的高级接口，包括设备管理、预览控制等
 */
class ICameraManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 管理器状态
     */
    enum ManagerStatus {
        Uninitialized,  ///< 未初始化
        Initializing,   ///< 初始化中
        Ready,          ///< 就绪
        Busy,           ///< 忙碌
        Error           ///< 错误
    };
    Q_ENUM(ManagerStatus)

    explicit ICameraManager(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ICameraManager() = default;

    // 管理器控制
    virtual bool initialize() = 0;
    virtual void cleanup() = 0;
    virtual ManagerStatus status() const = 0;

    // 设备管理
    virtual QStringList availableDevices() const = 0;
    virtual ICameraDevice* currentDevice() const = 0;
    virtual bool selectDevice(const QString& deviceId) = 0;
    virtual void refreshDevices() = 0;

    // 摄像头控制
    virtual bool startCamera() = 0;
    virtual void stopCamera() = 0;
    virtual bool isCameraActive() const = 0;

    // 预览控制
    virtual QVideoWidget* createPreviewWidget(QWidget* parent = nullptr) = 0;
    virtual void setPreviewWidget(QVideoWidget* widget) = 0;
    virtual QVideoWidget* previewWidget() const = 0;

    // 快捷配置
    virtual bool startWithPreset(ICameraDevice::QualityPreset preset) = 0;
    virtual void applyConfiguration(const QVariantMap& config) = 0;
    virtual QVariantMap currentConfiguration() const = 0;

    // 统计信息
    virtual int frameCount() const = 0;
    virtual double averageFrameRate() const = 0;
    virtual QSize currentResolution() const = 0;

signals:
    /**
     * @brief 管理器状态改变
     */
    void statusChanged(ManagerStatus status);

    /**
     * @brief 设备列表更新
     */
    void devicesUpdated(const QStringList& devices);

    /**
     * @brief 当前设备改变
     */
    void currentDeviceChanged(const QString& deviceId);

    /**
     * @brief 摄像头启动/停止
     */
    void cameraStarted();
    void cameraStopped();

    /**
     * @brief 错误信号
     */
    void errorOccurred(const QString& error);
};

#endif // ICAMERAMANAGER_H