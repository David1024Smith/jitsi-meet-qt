#ifndef OPTIMIZATIONTYPE_H
#define OPTIMIZATIONTYPE_H

/**
 * @brief 优化类型枚举
 * 
 * 定义不同类型的性能优化策略
 */
enum class OptimizationType
{
    // 启动优化
    Startup,            // 启动时间优化
    
    // 内存优化
    Memory,             // 内存使用优化
    MemoryLeakFix,      // 内存泄漏修复
    
    // CPU优化
    CPU,                // CPU使用优化
    Threading,          // 线程优化
    
    // 网络优化
    Network,            // 网络性能优化
    Bandwidth,          // 带宽优化
    
    // UI优化
    UI,                 // 用户界面优化
    Rendering,          // 渲染优化
    
    // 存储优化
    Storage,            // 存储优化
    Cache,              // 缓存优化
    
    // 电源优化
    Power,              // 电源管理优化
    Battery,            // 电池优化
    
    // 综合优化
    General,            // 通用优化
    Balanced,           // 平衡优化
    Performance,        // 性能优先优化
    Efficiency          // 效率优先优化
};

/**
 * @brief 优化策略枚举
 */
enum class OptimizationStrategy
{
    Conservative,       // 保守策略
    Balanced,          // 平衡策略
    Aggressive         // 激进策略
};

/**
 * @brief 性能级别枚举
 */
enum class PerformanceLevel
{
    Poor,              // 差
    Fair,              // 一般
    Good,              // 良好
    Excellent          // 优秀
};

/**
 * @brief 优化结果状态枚举
 */
enum class OptimizationResultStatus
{
    Success,           // 成功
    Failed,            // 失败
    Partial,           // 部分成功
    NotNeeded,         // 不需要优化
    NotSupported       // 不支持
};

#endif // OPTIMIZATIONTYPE_H