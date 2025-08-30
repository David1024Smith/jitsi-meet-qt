#ifndef AUDIOFACTORY_H
#define AUDIOFACTORY_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>

// 前向声明
class IAudioDevice;
class IAudioManager;
class AudioManager;
class AudioModule;

/**
 * @brief 音频工厂类
 * 
 * AudioFactory负责创建和管理音频模块中的各种对象实例，
 * 包括音频设备、管理器等。采用工厂模式确保对象创建的一致性。
 */
class AudioFactory : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 设备类型枚举
     */
    enum DeviceType {
        InputDevice,    ///< 输入设备
        OutputDevice    ///< 输出设备
    };
    Q_ENUM(DeviceType)

    /**
     * @brief 获取工厂单例实例
     * @return 工厂实例指针
     */
    static AudioFactory* instance();

    /**
     * @brief 析构函数
     */
    virtual ~AudioFactory();

    /**
     * @brief 创建音频模块实例
     * @param parent 父对象
     * @return 音频模块实例
     */
    AudioModule* createAudioModule(QObject *parent = nullptr);

    /**
     * @brief 创建音频管理器实例
     * @param parent 父对象
     * @return 音频管理器实例
     */
    AudioManager* createAudioManager(QObject *parent = nullptr);

    /**
     * @brief 创建音频设备实例
     * @param deviceId 设备ID
     * @param deviceType 设备类型
     * @param parent 父对象
     * @return 音频设备实例
     */
    IAudioDevice* createAudioDevice(const QString &deviceId, 
                                   DeviceType deviceType, 
                                   QObject *parent = nullptr);

    /**
     * @brief 获取可用的音频设备列表
     * @param deviceType 设备类型
     * @return 设备ID列表
     */
    QStringList availableDevices(DeviceType deviceType) const;

    /**
     * @brief 获取默认音频设备ID
     * @param deviceType 设备类型
     * @return 默认设备ID
     */
    QString defaultDevice(DeviceType deviceType) const;

    /**
     * @brief 检查设备是否可用
     * @param deviceId 设备ID
     * @param deviceType 设备类型
     * @return 设备可用返回true
     */
    bool isDeviceAvailable(const QString &deviceId, DeviceType deviceType) const;

    /**
     * @brief 获取设备信息
     * @param deviceId 设备ID
     * @return 设备信息映射
     */
    QVariantMap deviceInfo(const QString &deviceId) const;

    /**
     * @brief 注册自定义设备创建器
     * @param deviceType 设备类型
     * @param creator 创建器函数
     */
    void registerDeviceCreator(DeviceType deviceType, 
                              std::function<IAudioDevice*(const QString&, QObject*)> creator);

    /**
     * @brief 清理所有创建的对象
     */
    void cleanup();

signals:
    /**
     * @brief 设备列表改变信号
     */
    void devicesChanged();

    /**
     * @brief 默认设备改变信号
     * @param deviceType 设备类型
     * @param deviceId 新的默认设备ID
     */
    void defaultDeviceChanged(DeviceType deviceType, const QString &deviceId);

private:
    /**
     * @brief 私有构造函数 (单例模式)
     * @param parent 父对象
     */
    explicit AudioFactory(QObject *parent = nullptr);

    /**
     * @brief 初始化工厂
     */
    void initialize();

    /**
     * @brief 扫描可用设备
     */
    void scanDevices();

    /**
     * @brief 创建平台特定的音频设备
     * @param deviceId 设备ID
     * @param deviceType 设备类型
     * @param parent 父对象
     * @return 音频设备实例
     */
    IAudioDevice* createPlatformDevice(const QString &deviceId, 
                                      DeviceType deviceType, 
                                      QObject *parent);

private:
    class AudioFactoryPrivate;
    std::unique_ptr<AudioFactoryPrivate> d_ptr;
    Q_DECLARE_PRIVATE(AudioFactory)

    static AudioFactory* s_instance;    ///< 单例实例
};

#endif // AUDIOFACTORY_H