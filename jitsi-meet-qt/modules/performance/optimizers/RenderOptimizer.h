#ifndef RENDEROPTIMIZER_H
#define RENDEROPTIMIZER_H

#include "BaseOptimizer.h"
#include <QTimer>

/**
 * @brief 渲染优化器类
 * 
 * RenderOptimizer专门负责优化渲染性能：
 * - GPU加速优化
 * - 帧率优化
 * - 渲染管线优化
 * - 视频编解码优化
 */
class RenderOptimizer : public BaseOptimizer
{
    Q_OBJECT

public:
    /**
     * @brief 渲染优化策略枚举
     */
    enum RenderStrategy {
        PowerSaving,        ///< 节能模式 - 降低渲染质量以节省电力
        Balanced,          ///< 平衡模式 - 平衡质量和性能
        HighQuality        ///< 高质量模式 - 最佳渲染质量
    };
    Q_ENUM(RenderStrategy)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit RenderOptimizer(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~RenderOptimizer();

    /**
     * @brief 设置渲染策略
     * @param strategy 渲染策略
     */
    void setRenderStrategy(RenderStrategy strategy);

    /**
     * @brief 获取渲染策略
     * @return 渲染策略
     */
    RenderStrategy renderStrategy() const;

    /**
     * @brief 优化GPU设置
     * @return 优化是否成功
     */
    bool optimizeGPUSettings();

    /**
     * @brief 设置目标帧率
     * @param fps 目标帧率
     */
    void setTargetFrameRate(int fps);

    /**
     * @brief 获取目标帧率
     * @return 目标帧率
     */
    int targetFrameRate() const;

    /**
     * @brief 获取当前帧率
     * @return 当前帧率
     */
    double getCurrentFrameRate() const;

    /**
     * @brief 优化视频编解码
     * @return 优化是否成功
     */
    bool optimizeVideoCodec();

    /**
     * @brief 设置渲染质量
     * @param quality 质量等级(0-100)
     */
    void setRenderQuality(int quality);

    /**
     * @brief 获取渲染质量
     * @return 质量等级(0-100)
     */
    int renderQuality() const;

    /**
     * @brief 获取渲染统计信息
     * @return 渲染统计信息
     */
    QVariantMap getRenderStatistics() const;

protected:
    /**
     * @brief 初始化渲染优化器
     * @return 初始化是否成功
     */
    bool initializeOptimizer() override;

    /**
     * @brief 执行渲染优化
     * @param strategy 优化策略
     * @return 优化结果
     */
    OptimizationResult performOptimization(OptimizationStrategy strategy) override;

    /**
     * @brief 分析是否需要渲染优化
     * @return 是否需要优化
     */
    bool analyzeOptimizationNeed() const override;

    /**
     * @brief 生成渲染优化建议
     * @return 优化建议列表
     */
    QStringList generateSuggestions() const override;

    /**
     * @brief 预估渲染优化效果
     * @param strategy 优化策略
     * @return 预估改善效果
     */
    QVariantMap estimateOptimizationImprovements(OptimizationStrategy strategy) const override;

    /**
     * @brief 获取优化器版本
     * @return 版本字符串
     */
    QString getOptimizerVersion() const override;

    /**
     * @brief 获取优化器描述
     * @return 描述字符串
     */
    QString getOptimizerDescription() const override;

    /**
     * @brief 获取优化类型
     * @return 优化类型
     */
    OptimizationType getOptimizerType() const override;

private slots:
    /**
     * @brief 执行帧率监控
     */
    void performFrameRateMonitoring();

private:
    /**
     * @brief 执行节能渲染优化
     * @return 优化结果
     */
    OptimizationResult performPowerSavingOptimization();

    /**
     * @brief 执行平衡渲染优化
     * @return 优化结果
     */
    OptimizationResult performBalancedRenderOptimization();

    /**
     * @brief 执行高质量渲染优化
     * @return 优化结果
     */
    OptimizationResult performHighQualityOptimization();

    /**
     * @brief 检测GPU能力
     * @return GPU信息
     */
    QVariantMap detectGPUCapabilities();

    /**
     * @brief 调整渲染管线
     * @return 调整是否成功
     */
    bool adjustRenderPipeline();

    /**
     * @brief 优化纹理设置
     * @return 优化是否成功
     */
    bool optimizeTextureSettings();

    /**
     * @brief 测量渲染性能
     * @return 性能指标
     */
    QVariantMap measureRenderPerformance();

    RenderStrategy m_renderStrategy;                ///< 渲染策略
    int m_targetFrameRate;                         ///< 目标帧率
    int m_renderQuality;                           ///< 渲染质量
    
    QTimer* m_frameRateTimer;                      ///< 帧率监控定时器
    
    // 渲染统计
    double m_currentFrameRate;                     ///< 当前帧率
    qint64 m_frameCount;                           ///< 帧计数
    QDateTime m_lastFrameTime;                     ///< 上次帧时间
    
    // GPU信息
    QString m_gpuVendor;                           ///< GPU厂商
    QString m_gpuModel;                            ///< GPU型号
    bool m_hardwareAcceleration;                   ///< 硬件加速支持
    
    mutable QMutex m_renderMutex;                  ///< 渲染互斥锁
};

#endif // RENDEROPTIMIZER_H