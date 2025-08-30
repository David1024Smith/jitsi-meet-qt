#ifndef CAPTUREENGINE_H
#define CAPTUREENGINE_H

#include <QObject>
#include <QTimer>
#include <QPixmap>
#include <QMutex>
#include "../interfaces/IScreenCapture.h"

class VideoEncoder;
class FrameProcessor;

/**
 * @brief 捕获引擎类
 * 
 * 负责协调屏幕捕获、帧处理和编码的核心引擎
 */
class CaptureEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(double currentFPS READ currentFPS NOTIFY fpsChanged)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY frameCountChanged)

public:
    /**
     * @brief 引擎状态枚举
     */
    enum EngineStatus {
        Stopped,        ///< 停止状态
        Starting,       ///< 启动中
        Running,        ///< 运行中
        Pausing,        ///< 暂停中
        Paused,         ///< 已暂停
        Stopping,       ///< 停止中
        Error           ///< 错误状态
    };
    Q_ENUM(EngineStatus)

    /**
     * @brief 性能模式枚举
     */
    enum PerformanceMode {
        PowerSaving,    ///< 节能模式
        Balanced,       ///< 平衡模式
        Performance,    ///< 性能模式
        UltraPerformance ///< 超高性能模式
    };
    Q_ENUM(PerformanceMode)

    explicit CaptureEngine(QObject *parent = nullptr);
    virtual ~CaptureEngine();

    // 引擎控制接口
    bool initialize();
    void shutdown();
    bool start();
    void stop();
    void pause();
    void resume();

    // 状态查询接口
    EngineStatus status() const;
    bool isActive() const;
    bool isInitialized() const;
    bool isPaused() const;

    // 捕获配置接口
    void setCaptureSource(IScreenCapture* capture);
    IScreenCapture* captureSource() const;
    void setTargetFrameRate(int fps);
    int targetFrameRate() const;
    void setPerformanceMode(PerformanceMode mode);
    PerformanceMode performanceMode() const;

    // 编码配置接口
    void setVideoEncoder(VideoEncoder* encoder);
    VideoEncoder* videoEncoder() const;
    void setFrameProcessor(FrameProcessor* processor);
    FrameProcessor* frameProcessor() const;

    // 统计信息接口
    double currentFPS() const;
    int frameCount() const;
    qint64 totalProcessingTime() const;
    double averageProcessingTime() const;
    QVariantMap getPerformanceMetrics() const;

    // 质量控制接口
    void setQualityAdjustmentEnabled(bool enabled);
    bool isQualityAdjustmentEnabled() const;
    void setAdaptiveFrameRate(bool enabled);
    bool isAdaptiveFrameRateEnabled() const;

public slots:
    /**
     * @brief 手动触发帧捕获
     */
    void captureFrame();

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

    /**
     * @brief 优化性能设置
     */
    void optimizePerformance();

signals:
    /**
     * @brief 引擎激活状态改变信号
     * @param active 是否激活
     */
    void activeChanged(bool active);

    /**
     * @brief 引擎状态改变信号
     * @param status 新的引擎状态
     */
    void statusChanged(EngineStatus status);

    /**
     * @brief FPS改变信号
     * @param fps 当前FPS
     */
    void fpsChanged(double fps);

    /**
     * @brief 帧计数改变信号
     * @param count 当前帧计数
     */
    void frameCountChanged(int count);

    /**
     * @brief 帧处理完成信号
     * @param frame 处理后的帧数据
     * @param processingTime 处理时间(毫秒)
     */
    void frameProcessed(const QPixmap& frame, qint64 processingTime);

    /**
     * @brief 编码数据就绪信号
     * @param data 编码后的数据
     */
    void encodedDataReady(const QByteArray& data);

    /**
     * @brief 引擎错误信号
     * @param error 错误信息
     */
    void engineError(const QString& error);

    /**
     * @brief 性能警告信号
     * @param warning 警告信息
     */
    void performanceWarning(const QString& warning);

private slots:
    void onCaptureTimer();
    void onFrameCaptured(const QPixmap& frame);
    void onCaptureError(const QString& error);
    void onStatisticsTimer();
    void onQualityAdjustmentTimer();

private:
    void initializeTimers();
    void cleanupTimers();
    void processFrame(const QPixmap& frame);
    void updateStatistics();
    void adjustQuality();
    void updateStatus(EngineStatus newStatus);
    void calculateFPS();

    class Private;
    Private* d;
};

#endif // CAPTUREENGINE_H