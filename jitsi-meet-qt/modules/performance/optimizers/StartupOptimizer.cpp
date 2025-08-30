#include "StartupOptimizer.h"
#include <QDebug>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QElapsedTimer>
#include <QThread>

StartupOptimizer::StartupOptimizer(QObject *parent)
    : BaseOptimizer("StartupOptimizer", parent)
    , m_startupStrategy(BalancedStart)
    , m_startupTimeout(30000) // 30秒默认超时
    , m_startupSettings(nullptr)
    , m_lastStartupTime(0)
    , m_averageStartupTime(0)
    , m_bestStartupTime(LLONG_MAX)
    , m_cacheOptimized(false)
    , m_configOptimized(false)
    , m_moduleOrderOptimized(false)
{
    // 设置缓存目录
    m_cacheDirectory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/startup";
    QDir().mkpath(m_cacheDirectory);
    
    // 初始化启动设置
    QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/startup_optimizer.ini";
    m_startupSettings = new QSettings(settingsPath, QSettings::IniFormat, this);
    
    // 加载历史启动时间
    loadStartupTimeHistory();
    
    // 设置默认延迟加载模块
    m_deferredModules << "help" << "tutorial" << "analytics" << "feedback";
}

StartupOptimizer::~StartupOptimizer()
{
    // 保存启动时间历史
    saveStartupTimeHistory();
}

void StartupOptimizer::setStartupStrategy(StartupStrategy strategy)
{
    QMutexLocker locker(&m_startupMutex);
    if (m_startupStrategy != strategy) {
        m_startupStrategy = strategy;
        qDebug() << "StartupOptimizer: Strategy changed to" << strategy;
    }
}

StartupOptimizer::StartupStrategy StartupOptimizer::startupStrategy() const
{
    QMutexLocker locker(&m_startupMutex);
    return m_startupStrategy;
}

bool StartupOptimizer::preloadModules(const QStringList& moduleNames)
{
    qDebug() << "StartupOptimizer: Preloading modules:" << moduleNames;
    
    bool success = true;
    for (const QString& moduleName : moduleNames) {
        // 模拟模块预加载
        QThread::msleep(10); // 模拟加载时间
        
        if (!m_preloadedModules.contains(moduleName)) {
            m_preloadedModules.append(moduleName);
        }
        
        updateProgress(50 + (m_preloadedModules.size() * 30 / moduleNames.size()), 
                      QString("Preloading module: %1").arg(moduleName));
    }
    
    qDebug() << "StartupOptimizer: Preloaded" << m_preloadedModules.size() << "modules";
    return success;
}

void StartupOptimizer::setDeferredModules(const QStringList& moduleNames)
{
    QMutexLocker locker(&m_startupMutex);
    m_deferredModules = moduleNames;
    
    // 保存到设置
    m_startupSettings->setValue("deferredModules", moduleNames);
    m_startupSettings->sync();
}

QStringList StartupOptimizer::deferredModules() const
{
    QMutexLocker locker(&m_startupMutex);
    return m_deferredModules;
}

bool StartupOptimizer::optimizeStartupCache()
{
    qDebug() << "StartupOptimizer: Optimizing startup cache...";
    
    updateProgress(20, "Creating startup cache");
    
    if (!createStartupCache()) {
        addError("Failed to create startup cache");
        return false;
    }
    
    updateProgress(40, "Validating startup cache");
    
    if (!validateStartupCache()) {
        addError("Startup cache validation failed");
        return false;
    }
    
    m_cacheOptimized = true;
    updateProgress(60, "Startup cache optimized");
    
    return true;
}

bool StartupOptimizer::clearStartupCache()
{
    qDebug() << "StartupOptimizer: Clearing startup cache...";
    
    QDir cacheDir(m_cacheDirectory);
    if (cacheDir.exists()) {
        bool success = cacheDir.removeRecursively();
        if (success) {
            QDir().mkpath(m_cacheDirectory);
            m_cacheOptimized = false;
            qDebug() << "StartupOptimizer: Cache cleared successfully";
        }
        return success;
    }
    
    return true;
}

QVariantMap StartupOptimizer::getStartupTimeStats() const
{
    QMutexLocker locker(&m_startupMutex);
    
    QVariantMap stats;
    stats["lastStartupTime"] = m_lastStartupTime;
    stats["averageStartupTime"] = m_averageStartupTime;
    stats["bestStartupTime"] = (m_bestStartupTime == LLONG_MAX) ? 0 : m_bestStartupTime;
    stats["startupCount"] = m_startupTimeHistory.size();
    
    if (!m_startupTimeHistory.isEmpty()) {
        qint64 worstTime = *std::max_element(m_startupTimeHistory.begin(), m_startupTimeHistory.end());
        stats["worstStartupTime"] = worstTime;
        
        // 计算改善百分比
        if (m_startupTimeHistory.size() > 1) {
            qint64 firstTime = m_startupTimeHistory.first();
            qint64 lastTime = m_startupTimeHistory.last();
            double improvement = firstTime > 0 ? (100.0 * (firstTime - lastTime) / firstTime) : 0.0;
            stats["improvementPercent"] = improvement;
        }
    }
    
    return stats;
}

void StartupOptimizer::setStartupTimeout(int timeout)
{
    if (timeout > 0) {
        m_startupTimeout = timeout;
        m_startupSettings->setValue("startupTimeout", timeout);
        m_startupSettings->sync();
    }
}

int StartupOptimizer::startupTimeout() const
{
    return m_startupTimeout;
}

bool StartupOptimizer::initializeOptimizer()
{
    qDebug() << "StartupOptimizer: Initializing startup optimizer...";
    
    // 加载设置
    m_startupTimeout = m_startupSettings->value("startupTimeout", 30000).toInt();
    m_deferredModules = m_startupSettings->value("deferredModules", m_deferredModules).toStringList();
    
    // 检查缓存状态
    m_cacheOptimized = validateStartupCache();
    
    qDebug() << "StartupOptimizer: Initialized successfully";
    qDebug() << "  Cache optimized:" << m_cacheOptimized;
    qDebug() << "  Deferred modules:" << m_deferredModules.size();
    
    return true;
}

OptimizationResult StartupOptimizer::performOptimization(OptimizationStrategy strategy)
{
    OptimizationResult result;
    result.optimizerName = getOptimizerName();
    result.timestamp = QDateTime::currentDateTime();
    
    qDebug() << "StartupOptimizer: Performing optimization with strategy" << strategy;
    
    QElapsedTimer timer;
    timer.start();
    
    try {
        switch (m_startupStrategy) {
        case FastStart:
            result = performFastStartOptimization();
            break;
        case BalancedStart:
            result = performBalancedStartOptimization();
            break;
        case FullStart:
            result = performFullStartOptimization();
            break;
        }
        
        if (result.success) {
            result.description = QString("Startup optimization completed using %1 strategy")
                               .arg(QMetaEnum::fromType<StartupStrategy>().valueToKey(m_startupStrategy));
            
            // 测量优化后的启动时间
            qint64 newStartupTime = measureStartupTime();
            if (newStartupTime > 0) {
                recordStartupTime(newStartupTime);
                result.improvements.responseTimeGain = calculateStartupImprovement();
            }
        }
        
    } catch (const std::exception& e) {
        result.success = false;
        result.details.errorMessage = QString("Optimization failed: %1").arg(e.what());
        addError(result.details.errorMessage);
    }
    
    result.details.duration = timer.elapsed();
    return result;
}

bool StartupOptimizer::analyzeOptimizationNeed() const
{
    // 检查是否需要启动优化
    
    // 如果缓存未优化，需要优化
    if (!m_cacheOptimized) {
        return true;
    }
    
    // 如果启动时间过长，需要优化
    if (m_lastStartupTime > m_startupTimeout) {
        return true;
    }
    
    // 如果启动时间比平均时间长很多，需要优化
    if (m_averageStartupTime > 0 && m_lastStartupTime > m_averageStartupTime * 1.5) {
        return true;
    }
    
    return false;
}

QStringList StartupOptimizer::generateSuggestions() const
{
    QStringList suggestions;
    
    if (!m_cacheOptimized) {
        suggestions << "Optimize startup cache to improve loading speed";
    }
    
    if (m_lastStartupTime > m_startupTimeout) {
        suggestions << "Startup time exceeds timeout, consider using FastStart strategy";
    }
    
    if (m_deferredModules.isEmpty()) {
        suggestions << "Configure deferred modules to reduce initial startup time";
    }
    
    if (!m_moduleOrderOptimized) {
        suggestions << "Optimize module loading order for better performance";
    }
    
    if (m_preloadedModules.isEmpty()) {
        suggestions << "Preload critical modules to improve responsiveness";
    }
    
    if (suggestions.isEmpty()) {
        suggestions << "Startup performance is already optimized";
    }
    
    return suggestions;
}

QVariantMap StartupOptimizer::estimateOptimizationImprovements(OptimizationStrategy strategy) const
{
    QVariantMap improvements;
    
    // 基于策略和当前状态估算改善效果
    double timeImprovement = 0.0;
    double memoryImprovement = 0.0;
    
    switch (m_startupStrategy) {
    case FastStart:
        timeImprovement = 30.0; // 30%时间改善
        memoryImprovement = 15.0; // 15%内存改善
        break;
    case BalancedStart:
        timeImprovement = 20.0; // 20%时间改善
        memoryImprovement = 10.0; // 10%内存改善
        break;
    case FullStart:
        timeImprovement = 10.0; // 10%时间改善
        memoryImprovement = 5.0; // 5%内存改善
        break;
    }
    
    // 如果缓存未优化，额外改善
    if (!m_cacheOptimized) {
        timeImprovement += 15.0;
        memoryImprovement += 8.0;
    }
    
    // 如果模块顺序未优化，额外改善
    if (!m_moduleOrderOptimized) {
        timeImprovement += 10.0;
    }
    
    improvements["responseTimeGain"] = timeImprovement;
    improvements["memoryImprovement"] = memoryImprovement;
    improvements["startupTimeReduction"] = timeImprovement;
    
    Q_UNUSED(strategy)
    return improvements;
}

QString StartupOptimizer::getOptimizerVersion() const
{
    return "1.0.0";
}

QString StartupOptimizer::getOptimizerDescription() const
{
    return "Startup performance optimizer for reducing application launch time";
}

IOptimizer::OptimizationType StartupOptimizer::getOptimizerType() const
{
    return StartupOptimization;
}

QVariantMap StartupOptimizer::getBeforeMetrics() const
{
    QVariantMap metrics = BaseOptimizer::getBeforeMetrics();
    
    metrics["startupTime"] = m_lastStartupTime;
    metrics["averageStartupTime"] = m_averageStartupTime;
    metrics["cacheOptimized"] = m_cacheOptimized;
    metrics["moduleOrderOptimized"] = m_moduleOrderOptimized;
    metrics["preloadedModules"] = m_preloadedModules.size();
    metrics["deferredModules"] = m_deferredModules.size();
    
    return metrics;
}

QVariantMap StartupOptimizer::getAfterMetrics() const
{
    QVariantMap metrics = BaseOptimizer::getAfterMetrics();
    
    // 重新测量指标
    metrics["startupTime"] = measureStartupTime();
    metrics["cacheOptimized"] = m_cacheOptimized;
    metrics["moduleOrderOptimized"] = m_moduleOrderOptimized;
    metrics["preloadedModules"] = m_preloadedModules.size();
    metrics["deferredModules"] = m_deferredModules.size();
    
    return metrics;
}

OptimizationResult StartupOptimizer::performFastStartOptimization()
{
    OptimizationResult result;
    result.success = true;
    
    updateProgress(10, "Starting fast startup optimization");
    
    // 1. 优化缓存
    if (!optimizeStartupCache()) {
        result.success = false;
        result.details.errorMessage = "Failed to optimize startup cache";
        return result;
    }
    
    updateProgress(30, "Optimizing module loading order");
    
    // 2. 优化模块加载顺序
    if (!optimizeModuleLoadOrder()) {
        result.success = false;
        result.details.errorMessage = "Failed to optimize module load order";
        return result;
    }
    
    updateProgress(50, "Configuring deferred loading");
    
    // 3. 配置延迟加载
    QStringList criticalModules = getCriticalModules();
    QStringList optionalModules = getOptionalModules();
    
    // 在快速启动模式下，延迟加载更多模块
    setDeferredModules(optionalModules);
    
    updateProgress(70, "Preloading critical modules");
    
    // 4. 预加载关键模块
    if (!preloadModules(criticalModules)) {
        result.success = false;
        result.details.errorMessage = "Failed to preload critical modules";
        return result;
    }
    
    updateProgress(90, "Finalizing optimization");
    
    // 5. 优化配置
    if (!optimizeStartupConfiguration()) {
        result.success = false;
        result.details.errorMessage = "Failed to optimize startup configuration";
        return result;
    }
    
    result.details.actionsPerformed << "Optimized startup cache"
                                   << "Optimized module loading order"
                                   << "Configured deferred loading"
                                   << "Preloaded critical modules"
                                   << "Optimized startup configuration";
    
    updateProgress(100, "Fast startup optimization completed");
    
    return result;
}

OptimizationResult StartupOptimizer::performBalancedStartOptimization()
{
    OptimizationResult result;
    result.success = true;
    
    updateProgress(10, "Starting balanced startup optimization");
    
    // 平衡模式：在启动时间和功能完整性之间平衡
    
    // 1. 优化缓存
    if (!optimizeStartupCache()) {
        result.success = false;
        result.details.errorMessage = "Failed to optimize startup cache";
        return result;
    }
    
    updateProgress(25, "Optimizing module loading");
    
    // 2. 优化模块加载
    if (!optimizeModuleLoadOrder()) {
        result.success = false;
        result.details.errorMessage = "Failed to optimize module load order";
        return result;
    }
    
    updateProgress(50, "Configuring module loading strategy");
    
    // 3. 配置适中的延迟加载策略
    QStringList criticalModules = getCriticalModules();
    QStringList optionalModules = getOptionalModules();
    
    // 只延迟加载真正可选的模块
    QStringList deferredModules;
    for (const QString& module : optionalModules) {
        if (module.contains("help") || module.contains("tutorial") || module.contains("analytics")) {
            deferredModules.append(module);
        }
    }
    setDeferredModules(deferredModules);
    
    updateProgress(75, "Preloading essential modules");
    
    // 4. 预加载基本模块
    if (!preloadModules(criticalModules)) {
        result.success = false;
        result.details.errorMessage = "Failed to preload essential modules";
        return result;
    }
    
    updateProgress(90, "Applying configuration optimizations");
    
    // 5. 应用配置优化
    if (!optimizeStartupConfiguration()) {
        result.success = false;
        result.details.errorMessage = "Failed to optimize startup configuration";
        return result;
    }
    
    result.details.actionsPerformed << "Optimized startup cache"
                                   << "Optimized module loading order"
                                   << "Applied balanced loading strategy"
                                   << "Preloaded essential modules"
                                   << "Applied configuration optimizations";
    
    updateProgress(100, "Balanced startup optimization completed");
    
    return result;
}

OptimizationResult StartupOptimizer::performFullStartOptimization()
{
    OptimizationResult result;
    result.success = true;
    
    updateProgress(10, "Starting full startup optimization");
    
    // 完整模式：预加载所有功能，确保最佳用户体验
    
    // 1. 优化缓存
    if (!optimizeStartupCache()) {
        result.success = false;
        result.details.errorMessage = "Failed to optimize startup cache";
        return result;
    }
    
    updateProgress(20, "Preloading all modules");
    
    // 2. 预加载所有模块
    QStringList allModules = getCriticalModules() + getOptionalModules();
    if (!preloadModules(allModules)) {
        result.success = false;
        result.details.errorMessage = "Failed to preload all modules";
        return result;
    }
    
    updateProgress(50, "Optimizing module loading order");
    
    // 3. 优化模块加载顺序
    if (!optimizeModuleLoadOrder()) {
        result.success = false;
        result.details.errorMessage = "Failed to optimize module load order";
        return result;
    }
    
    updateProgress(70, "Minimizing deferred loading");
    
    // 4. 最小化延迟加载
    QStringList minimalDeferred;
    minimalDeferred << "analytics"; // 只延迟加载分析模块
    setDeferredModules(minimalDeferred);
    
    updateProgress(90, "Applying full optimization configuration");
    
    // 5. 应用完整优化配置
    if (!optimizeStartupConfiguration()) {
        result.success = false;
        result.details.errorMessage = "Failed to optimize startup configuration";
        return result;
    }
    
    result.details.actionsPerformed << "Optimized startup cache"
                                   << "Preloaded all modules"
                                   << "Optimized module loading order"
                                   << "Minimized deferred loading"
                                   << "Applied full optimization configuration";
    
    updateProgress(100, "Full startup optimization completed");
    
    return result;
}