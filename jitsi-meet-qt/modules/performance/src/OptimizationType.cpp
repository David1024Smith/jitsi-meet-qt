#include "OptimizationType.h"

/**
 * @brief OptimizationStrategy的QDebug操作符实现
 * @param debug QDebug对象
 * @param strategy 优化策略
 * @return QDebug对象引用
 */
QDebug operator<<(QDebug debug, OptimizationStrategy strategy)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    
    switch (strategy) {
    case OptimizationStrategy::Conservative:
        debug << "OptimizationStrategy::Conservative";
        break;
    case OptimizationStrategy::Balanced:
        debug << "OptimizationStrategy::Balanced";
        break;
    case OptimizationStrategy::Aggressive:
        debug << "OptimizationStrategy::Aggressive";
        break;
    default:
        debug << "OptimizationStrategy::Unknown";
        break;
    }
    
    return debug;
}

/**
 * @brief OptimizationResultStatus的QDebug操作符实现
 * @param debug QDebug对象
 * @param result 优化结果状态
 * @return QDebug对象引用
 */
QDebug operator<<(QDebug debug, OptimizationResultStatus result)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    
    switch (result) {
    case OptimizationResultStatus::Success:
        debug << "OptimizationResultStatus::Success";
        break;
    case OptimizationResultStatus::Failed:
        debug << "OptimizationResultStatus::Failed";
        break;
    case OptimizationResultStatus::Partial:
        debug << "OptimizationResultStatus::Partial";
        break;
    case OptimizationResultStatus::NotNeeded:
        debug << "OptimizationResultStatus::NotNeeded";
        break;
    case OptimizationResultStatus::NotSupported:
        debug << "OptimizationResultStatus::NotSupported";
        break;
    default:
        debug << "OptimizationResultStatus::Unknown";
        break;
    }
    
    return debug;
}