#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H

#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QMutex>
#include "../interfaces/IScreenShareManager.h"

/**
 * @brief 视频编码器类
 * 
 * 负责将捕获的帧数据编码为视频流
 */
class VideoEncoder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(EncodingFormat format READ encodingFormat WRITE setEncodingFormat NOTIFY formatChanged)
    Q_PROPERTY(int bitrate READ bitrate WRITE setBitrate NOTIFY bitrateChanged)
    Q_PROPERTY(QSize resolution READ resolution WRITE setResolution NOTIFY resolutionChanged)

public:
    // 从接口导入枚举类型
    using EncodingFormat = IScreenShareManager::EncodingFormat;

    /**
     * @brief 编码器状态枚举
     */
    enum EncoderStatus {
        Uninitialized,  ///< 未初始化
        Initializing,   ///< 初始化中
        Ready,          ///< 就绪状态
        Encoding,       ///< 编码中
        Paused,         ///< 暂停状态
        Error           ///< 错误状态
    };
    Q_ENUM(EncoderStatus)

    /**
     * @brief 编码质量枚举
     */
    enum EncodingQuality {
        VeryLow,        ///< 极低质量
        Low,            ///< 低质量
        Medium,         ///< 中等质量
        High,           ///< 高质量
        VeryHigh,       ///< 极高质量
        Lossless        ///< 无损质量
    };
    Q_ENUM(EncodingQuality)

    /**
     * @brief 编码预设枚举
     */
    enum EncodingPreset {
        UltraFast,      ///< 超快速编码
        SuperFast,      ///< 超快编码
        VeryFast,       ///< 很快编码
        Faster,         ///< 较快编码
        Fast,           ///< 快速编码
        Medium,         ///< 中等编码
        Slow,           ///< 慢速编码
        Slower,         ///< 较慢编码
        VerySlow        ///< 很慢编码
    };
    Q_ENUM(EncodingPreset)

    explicit VideoEncoder(QObject *parent = nullptr);
    virtual ~VideoEncoder();

    // 编码器控制接口
    bool initialize();
    void shutdown();
    bool start();
    void stop();
    void pause();
    void resume();

    // 状态查询接口
    EncoderStatus status() const;
    bool isActive() const;
    bool isInitialized() const;
    bool isPaused() const;

    // 编码配置接口
    EncodingFormat encodingFormat() const;
    void setEncodingFormat(EncodingFormat format);
    EncodingQuality encodingQuality() const;
    void setEncodingQuality(EncodingQuality quality);
    EncodingPreset encodingPreset() const;
    void setEncodingPreset(EncodingPreset preset);

    // 视频参数接口
    QSize resolution() const;
    void setResolution(const QSize& size);
    int frameRate() const;
    void setFrameRate(int fps);
    int bitrate() const;
    void setBitrate(int kbps);
    int keyFrameInterval() const;
    void setKeyFrameInterval(int interval);

    // 高级配置接口
    bool hardwareAcceleration() const;
    void setHardwareAcceleration(bool enabled);
    int threadCount() const;
    void setThreadCount(int count);
    int bufferSize() const;
    void setBufferSize(int size);

    // 编码接口
    bool encodeFrame(const QPixmap& frame);
    bool encodeFrameData(const QByteArray& data, const QSize& size);
    void flush();

    // 统计信息接口
    qint64 totalFramesEncoded() const;
    double averageEncodingTime() const;
    qint64 totalEncodedBytes() const;
    double compressionRatio() const;
    QVariantMap getEncodingStatistics() const;

public slots:
    /**
     * @brief 重置编码器
     */
    void reset();

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

    /**
     * @brief 优化编码设置
     */
    void optimizeSettings();

signals:
    /**
     * @brief 编码器激活状态改变信号
     * @param active 是否激活
     */
    void activeChanged(bool active);

    /**
     * @brief 编码器状态改变信号
     * @param status 新的编码器状态
     */
    void statusChanged(EncoderStatus status);

    /**
     * @brief 编码格式改变信号
     * @param format 新的编码格式
     */
    void formatChanged(EncodingFormat format);

    /**
     * @brief 比特率改变信号
     * @param bitrate 新的比特率
     */
    void bitrateChanged(int bitrate);

    /**
     * @brief 分辨率改变信号
     * @param resolution 新的分辨率
     */
    void resolutionChanged(const QSize& resolution);

    /**
     * @brief 编码数据就绪信号
     * @param data 编码后的数据
     * @param timestamp 时间戳
     */
    void encodedDataReady(const QByteArray& data, qint64 timestamp);

    /**
     * @brief 关键帧编码信号
     * @param data 关键帧数据
     */
    void keyFrameEncoded(const QByteArray& data);

    /**
     * @brief 编码错误信号
     * @param error 错误信息
     */
    void encodingError(const QString& error);

    /**
     * @brief 编码统计更新信号
     * @param statistics 统计信息
     */
    void statisticsUpdated(const QVariantMap& statistics);

private slots:
    void onEncodingTimer();
    void onStatisticsTimer();

private:
    void initializeEncoder();
    void cleanupEncoder();
    void updateStatus(EncoderStatus newStatus);
    bool configureEncoder();
    QByteArray processFrame(const QPixmap& frame);
    void updateStatistics();
    void emitError(const QString& error);

    // 格式特定编码器
    bool initializeH264Encoder();
    bool initializeVP8Encoder();
    bool initializeVP9Encoder();
    bool initializeAV1Encoder();

    QByteArray encodeH264Frame(const QPixmap& frame);
    QByteArray encodeVP8Frame(const QPixmap& frame);
    QByteArray encodeVP9Frame(const QPixmap& frame);
    QByteArray encodeAV1Frame(const QPixmap& frame);

    class Private;
    Private* d;
};

#endif // VIDEOENCODER_H