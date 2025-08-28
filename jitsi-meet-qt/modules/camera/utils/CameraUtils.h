#ifndef CAMERAUTILS_H
#define CAMERAUTILS_H

#include <QObject>
#include <QString>
#include <QSize>
#include <QVideoFrame>
#include <QPixmap>
#include <QStringList>
#include "../interfaces/ICameraDevice.h"

/**
 * @brief 摄像头工具类
 * 
 * 提供摄像头相关的实用工具函数
 */
class CameraUtils : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 分辨率预设映射
     */
    static QSize resolutionForPreset(ICameraDevice::QualityPreset preset);
    
    /**
     * @brief 帧率预设映射
     */
    static int frameRateForPreset(ICameraDevice::QualityPreset preset);
    
    /**
     * @brief 质量预设名称
     */
    static QString presetName(ICameraDevice::QualityPreset preset);
    
    /**
     * @brief 状态名称
     */
    static QString statusName(ICameraDevice::Status status);
    
    /**
     * @brief 检查分辨率是否有效
     */
    static bool isValidResolution(const QSize& resolution);
    
    /**
     * @brief 检查帧率是否有效
     */
    static bool isValidFrameRate(int frameRate);
    
    /**
     * @brief 获取推荐的分辨率列表
     */
    static QList<QSize> recommendedResolutions();
    
    /**
     * @brief 获取推荐的帧率列表
     */
    static QList<int> recommendedFrameRates();
    
    /**
     * @brief 计算视频比特率
     */
    static int calculateBitrate(const QSize& resolution, int frameRate, ICameraDevice::QualityPreset preset);
    
    /**
     * @brief 格式化分辨率字符串
     */
    static QString formatResolution(const QSize& resolution);
    
    /**
     * @brief 解析分辨率字符串
     */
    static QSize parseResolution(const QString& resolutionStr);
    
    /**
     * @brief 视频帧转换为QPixmap
     */
    static QPixmap frameToPixmap(const QVideoFrame& frame);
    
    /**
     * @brief 计算视频帧大小
     */
    static qint64 calculateFrameSize(const QSize& resolution, const QString& format = "RGB32");
    
    /**
     * @brief 检查设备ID是否有效
     */
    static bool isValidDeviceId(const QString& deviceId);
    
    /**
     * @brief 生成设备友好名称
     */
    static QString generateFriendlyName(const QString& deviceId, const QString& deviceName);
    
    /**
     * @brief 比较两个分辨率
     */
    static int compareResolutions(const QSize& res1, const QSize& res2);
    
    /**
     * @brief 获取最接近的支持分辨率
     */
    static QSize findClosestResolution(const QSize& target, const QList<QSize>& supported);
    
    /**
     * @brief 获取最接近的支持帧率
     */
    static int findClosestFrameRate(int target, const QList<int>& supported);
    
    /**
     * @brief 计算宽高比
     */
    static double aspectRatio(const QSize& resolution);
    
    /**
     * @brief 检查是否为标准宽高比
     */
    static bool isStandardAspectRatio(const QSize& resolution);
    
    /**
     * @brief 获取宽高比名称
     */
    static QString aspectRatioName(const QSize& resolution);
    
    /**
     * @brief 性能评估
     */
    static QString performanceLevel(const QSize& resolution, int frameRate);
    
    /**
     * @brief 内存使用估算
     */
    static qint64 estimateMemoryUsage(const QSize& resolution, int frameRate, int bufferCount = 3);

private:
    CameraUtils() = delete; // 工具类，不允许实例化
};

#endif // CAMERAUTILS_H