#include "BaseOptimizer.h"
#include <QDebug>
#include <QDateTime>

BaseOptimizer::BaseOptimizer(const QString& optimizerName, QObject *parent)
    : QObject(parent)
    , m_optimizerName(optimizerName)
    , m_status(OptimizerStatus::Idle)
    , m_enabled(true)
    , m_autoOptimizationEnabled(false)
    , m_optimizationInterval(DEFAULT_OPTIMIZATION_INTERVAL)
    , m_cancellationRequested(false)
    , m_optimizationCount(0)
    , m_successfulOptimizations(0)
    , m_failedOptimizations(0)
    , m_totalOptimizationTime(0)
{
    // 初始化自动优化定时器
    m_autoOptimizationTimer = new QTimer(this);
    m_autoOptimizationTimer->setSingleShot(false);
    m_autoOptimizationTimer->setInterval(m_optimizationInterval);
    connect(m_autoOptimizationTimer, &QTimer::timeout,
            this, &BaseOptimizer::performAutoOptimization);
    
    // 初始化历史清理定时器
    m_historyCleanupTimer = new QTimer(this);
    m_historyCleanupTimer->setSingleShot(false);
    m_historyCleanupTimer->setInterval(HISTORY_CLEANUP_INTERVAL);
    connect(m_historyCleanupTimer, &QTimer::timeout,
            this, &BaseOptimizer::cleanupHistory);
    m_historyCleanupTimer->start();
    
    // 初始化优化历史容器
    m_optimizationHistory.reserve(MAX_HISTORY_SIZE);
}

BaseOptimizer::~BaseOptimizer()
{
    if (m_autoOptimizationTimer->isActive()) {
        m_autoOptimizationTimer->stop();
    }
    
    if (m_historyCleanupTimer->isActive()) {
        m_historyCleanupTimer->stop();
    }
}

bool BaseOptimizer::initialize()
{
    qDebug() << "BaseOptimizer: Initializing optimizer" << m_optimizerName;
    
    setStatus(OptimizerStatus::Analyzing);
    
    if (!initializeOptimizer()) {
        addError("Failed to initialize optimizer-specific functionality");
        setStatus(OptimizerStatus::Failed);
        return false;
    }
    
    // 加载默认参数
    if (m_optimizationParameters.isEmpty()) {
        m_optimizationParameters = getDefaultParameters();
    }
    
    // 验证配置
    if (!validateConfiguration()) {
        addError("Invalid optimizer configuration");
        setStatus(OptimizerStatus::Failed);
        return false;
    }
    
    setStatus(OptimizerStatus::Idle);
    
    qDebug() << "BaseOptimizer: Initialized successfully";
    return true;
}

OptimizationResult BaseOptimizer::optimize(OptimizationStrategy strategy)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_enabled) {
        OptimizationResult result;
        result.status = OptimizationResultStatus::Failed;
        result.message = "Optimizer is disabled";
        result.timestamp = QDateTime::currentDateTime();
        result.errors << "Optimizer is disabled";
        return result;
    }
    
    if (m_status == OptimizerStatus::Optimizing) {
        OptimizationResult result;
        result.status = OptimizationResultStatus::Failed;
        result.message = "Optimization already in progress";
        result.timestamp = QDateTime::currentDateTime();
        result.errors << "Optimization already in progress";
        return result;
    }
    
    setStatus(OptimizerStatus::Analyzing);
    m_cancellationRequested = false;
    
    emit optimizationStarted(strategy);
    
    QDateTime startTime = QDateTime::currentDateTime();
    
    // 获取优化前指标
    QVariantMap beforeMetrics = getBeforeMetrics();
    
    setStatus(OptimizerStatus::Optimizing);
    updateProgress(10, "Starting optimization");
    
    // 执行具体优化操作
    OptimizationResult result = performOptimization(strategy);
    
    // 获取优化后指标
    QVariantMap afterMetrics = getAfterMetrics();
    
    // 计算改善效果
    result.improvements = calculateImprovements(beforeMetrics, afterMetrics);
    result.beforeMetrics = beforeMetrics;
    result.afterMetrics = afterMetrics;
    result.executionTime = startTime.msecsTo(QDateTime::currentDateTime());
    
    // 更新统计信息
    updateStatistics(result);
    
    // 记录结果
    recordOptimizationResult(result);
    
    if (result.isSuccess()) {
        setStatus(OptimizerStatus::Completed);
        updateProgress(100, "Optimization completed successfully");
    } else {
        setStatus(OptimizerStatus::Failed);
        if (!result.errors.isEmpty()) {
            addError(result.errors.first());
        }
    }
    
    emit optimizationCompleted(result);
    
    // 重置状态
    QTimer::singleShot(5000, this, [this]() {
        setStatus(OptimizerStatus::Idle);
    });
    
    return result;
}

bool BaseOptimizer::shouldOptimize() const
{
    if (!m_enabled) {
        return false;
    }
    
    return analyzeOptimizationNeed();
}

QStringList BaseOptimizer::getOptimizationSuggestions() const
{
    if (!m_enabled) {
        return QStringList() << "Optimizer is disabled";
    }
    
    return generateSuggestions();
}

QVariantMap BaseOptimizer::estimateImprovements(OptimizationStrategy strategy) const
{
    if (!m_enabled) {
        return QVariantMap();
    }
    
    return estimateOptimizationImprovements(strategy);
}

QString BaseOptimizer::getOptimizerName() const
{
    return m_optimizerName;
}

QString BaseOptimizer::getVersion() const
{
    return getOptimizerVersion();
}

OptimizationType BaseOptimizer::getOptimizationType() const
{
    return getOptimizerType();
}

OptimizerStatus BaseOptimizer::getStatus() const
{
    QMutexLocker locker(&m_mutex);
    return m_status;
}

QString BaseOptimizer::getDescription() const
{
    return getOptimizerDescription();
}

void BaseOptimizer::setOptimizationParameters(const QVariantMap& parameters)
{
    QMutexLocker locker(&m_mutex);
    
    if (validateOptimizationParameters(parameters)) {
        m_optimizationParameters = parameters;
        qDebug() << "BaseOptimizer: Parameters updated for" << m_optimizerName;
    } else {
        qWarning() << "BaseOptimizer: Invalid parameters for" << m_optimizerName;
    }
}

QVariantMap BaseOptimizer::getOptimizationParameters() const
{
    QMutexLocker locker(&m_mutex);
    return m_optimizationParameters;
}

void BaseOptimizer::enable()
{
    QMutexLocker locker(&m_mutex);
    if (!m_enabled) {
        m_enabled = true;
        qDebug() << "BaseOptimizer: Enabled" << m_optimizerName;
    }
}

void BaseOptimizer::disable()
{
    QMutexLocker locker(&m_mutex);
    if (m_enabled) {
        m_enabled = false;
        
        // 停止自动优化
        if (m_autoOptimizationTimer->isActive()) {
            m_autoOptimizationTimer->stop();
        }
        
        qDebug() << "BaseOptimizer: Disabled" << m_optimizerName;
    }
}

bool BaseOptimizer::isEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_enabled;
}

OptimizationResult BaseOptimizer::getLastOptimizationResult() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastResult;
}

QList<OptimizationResult> BaseOptimizer::getOptimizationHistory(const QDateTime& from, const QDateTime& to) const
{
    QMutexLocker locker(&m_mutex);
    
    QList<OptimizationResult> filteredHistory;
    
    for (const OptimizationResult& result : m_optimizationHistory) {
        if (result.timestamp >= from && result.timestamp <= to) {
            filteredHistory.append(result);
        }
    }
    
    return filteredHistory;
}

void BaseOptimizer::reset()
{
    QMutexLocker locker(&m_mutex);
    
    // 重置统计信息
    m_optimizationCount = 0;
    m_successfulOptimizations = 0;
    m_failedOptimizations = 0;
    m_totalOptimizationTime = 0;
    m_totalImprovements.clear();
    
    // 清空历史记录
    m_optimizationHistory.clear();
    m_errors.clear();
    
    // 重置状态
    setStatus(OptimizerStatus::Idle);
    
    qDebug() << "BaseOptimizer: Reset" << m_optimizerName;
}

bool BaseOptimizer::validateConfiguration() const
{
    // 基本配置验证
    if (m_optimizerName.isEmpty()) {
        return false;
    }
    
    if (!validateOptimizationParameters(m_optimizationParameters)) {
        return false;
    }
    
    return true;
}

QVariantMap BaseOptimizer::getStatistics() const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap stats;
    stats["optimizerName"] = m_optimizerName;
    stats["enabled"] = m_enabled;
    stats["status"] = static_cast<int>(m_status);
    stats["optimizationCount"] = m_optimizationCount;
    stats["successfulOptimizations"] = m_successfulOptimizations;
    stats["failedOptimizations"] = m_failedOptimizations;
    stats["successRate"] = m_optimizationCount > 0 ? (100.0 * m_successfulOptimizations / m_optimizationCount) : 0.0;
    stats["averageOptimizationTime"] = getAverageOptimizationTime();
    stats["totalImprovements"] = m_totalImprovements;
    stats["firstOptimizationTime"] = m_firstOptimizationTime;
    stats["lastOptimizationTime"] = m_lastOptimizationTime;
    stats["autoOptimizationEnabled"] = m_autoOptimizationEnabled;
    stats["optimizationInterval"] = m_optimizationInterval;
    
    return stats;
}

void BaseOptimizer::cancelOptimization()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status == OptimizerStatus::Optimizing || m_status == OptimizerStatus::Analyzing) {
        m_cancellationRequested = true;
        qDebug() << "BaseOptimizer: Cancellation requested for" << m_optimizerName;
        emit optimizationCancelled();
    }
}

bool BaseOptimizer::canCancel() const
{
    QMutexLocker locker(&m_mutex);
    return (m_status == OptimizerStatus::Optimizing || m_status == OptimizerStatus::Analyzing);
}

void BaseOptimizer::setStatus(OptimizerStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void BaseOptimizer::setOptimizationInterval(int interval)
{
    QMutexLocker locker(&m_mutex);
    
    if (interval > 0 && m_optimizationInterval != interval) {
        m_optimizationInterval = interval;
        m_autoOptimizationTimer->setInterval(interval);
        
        qDebug() << "BaseOptimizer: Optimization interval changed to" << interval << "ms for" << m_optimizerName;
    }
}

int BaseOptimizer::optimizationInterval() const
{
    QMutexLocker locker(&m_mutex);
    return m_optimizationInterval;
}

void BaseOptimizer::setAutoOptimizationEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_autoOptimizationEnabled != enabled) {
        m_autoOptimizationEnabled = enabled;
        
        if (enabled && m_enabled) {
            m_autoOptimizationTimer->start();
        } else {
            m_autoOptimizationTimer->stop();
        }
        
        qDebug() << "BaseOptimizer: Auto optimization" << (enabled ? "enabled" : "disabled") << "for" << m_optimizerName;
    }
}

bool BaseOptimizer::isAutoOptimizationEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoOptimizationEnabled;
}

int BaseOptimizer::getOptimizationCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_optimizationCount;
}

int BaseOptimizer::getSuccessfulOptimizationCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_successfulOptimizations;
}

int BaseOptimizer::getFailedOptimizationCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_failedOptimizations;
}

double BaseOptimizer::getAverageOptimizationTime() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_optimizationCount > 0) {
        return static_cast<double>(m_totalOptimizationTime) / m_optimizationCount;
    }
    
    return 0.0;
}

QVariantMap BaseOptimizer::getTotalImprovements() const
{
    QMutexLocker locker(&m_mutex);
    return m_totalImprovements;
}

void BaseOptimizer::addError(const QString& error)
{
    QMutexLocker locker(&m_mutex);
    
    m_errors.append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString()).arg(error));
    
    // 限制错误列表大小
    if (m_errors.size() > 100) {
        m_errors.removeFirst();
    }
    
    emit errorOccurred(error);
}

void BaseOptimizer::updateProgress(int progress, const QString& description)
{
    emit optimizationProgress(progress, description);
}

void BaseOptimizer::recordOptimizationResult(const OptimizationResult& result)
{
    m_lastResult = result;
    m_optimizationHistory.append(result);
    
    // 限制历史记录大小
    if (m_optimizationHistory.size() > MAX_HISTORY_SIZE) {
        m_optimizationHistory.removeFirst();
    }
}

QVariantMap BaseOptimizer::getBeforeMetrics() const
{
    // 子类可以重写此方法来提供特定的指标
    QVariantMap metrics;
    metrics["timestamp"] = QDateTime::currentDateTime();
    return metrics;
}

QVariantMap BaseOptimizer::getAfterMetrics() const
{
    // 子类可以重写此方法来提供特定的指标
    QVariantMap metrics;
    metrics["timestamp"] = QDateTime::currentDateTime();
    return metrics;
}

QVariantMap BaseOptimizer::calculateImprovements(const QVariantMap& beforeMetrics, const QVariantMap& afterMetrics) const
{
    QVariantMap improvements;
    
    // 基本改善计算
    // 子类可以重写此方法来提供更具体的计算
    
    Q_UNUSED(beforeMetrics)
    Q_UNUSED(afterMetrics)
    
    return improvements;
}

bool BaseOptimizer::validateOptimizationParameters(const QVariantMap& parameters) const
{
    // 基本参数验证
    // 子类可以重写此方法来提供更具体的验证
    
    Q_UNUSED(parameters)
    return true;
}

QVariantMap BaseOptimizer::getDefaultParameters() const
{
    // 返回默认参数
    QVariantMap defaults;
    defaults["strategy"] = static_cast<int>(OptimizationStrategy::Balanced);
    defaults["timeout"] = 30000; // 30秒超时
    defaults["retryCount"] = 3;
    
    return defaults;
}

void BaseOptimizer::performAutoOptimization()
{
    if (!m_enabled || !m_autoOptimizationEnabled) {
        return;
    }
    
    if (m_status != OptimizerStatus::Idle) {
        return; // 已经在执行优化
    }
    
    if (shouldOptimize()) {
        qDebug() << "BaseOptimizer: Performing auto optimization for" << m_optimizerName;
        optimize(OptimizationStrategy::Balanced);
    }
}

void BaseOptimizer::cleanupHistory()
{
    QMutexLocker locker(&m_mutex);
    
    // 清理超过24小时的历史记录
    QDateTime cutoffTime = QDateTime::currentDateTime().addDays(-1);
    
    int removedCount = 0;
    for (int i = m_optimizationHistory.size() - 1; i >= 0; --i) {
        if (m_optimizationHistory[i].timestamp < cutoffTime) {
            m_optimizationHistory.removeAt(i);
            removedCount++;
        }
    }
    
    if (removedCount > 0) {
        qDebug() << "BaseOptimizer: Cleaned up" << removedCount << "old optimization records for" << m_optimizerName;
    }
}

void BaseOptimizer::updateStatistics(const OptimizationResult& result)
{
    m_optimizationCount++;
    
    if (result.isSuccess()) {
        m_successfulOptimizations++;
    } else {
        m_failedOptimizations++;
    }
    
    m_totalOptimizationTime += result.executionTime;
    
    // 更新时间戳
    if (!m_firstOptimizationTime.isValid()) {
        m_firstOptimizationTime = result.timestamp;
    }
    m_lastOptimizationTime = result.timestamp;
    
    // 累计改善效果
    if (result.isSuccess()) {
        m_totalImprovements["cpuImprovement"] = m_totalImprovements.value("cpuImprovement", 0.0).toDouble() + result.improvements.value("cpuImprovement", 0.0).toDouble();
        m_totalImprovements["memoryImprovement"] = m_totalImprovements.value("memoryImprovement", 0.0).toDouble() + result.improvements.value("memoryImprovement", 0.0).toDouble();
        m_totalImprovements["performanceGain"] = m_totalImprovements.value("performanceGain", 0.0).toDouble() + result.improvements.value("performanceGain", 0.0).toDouble();
        m_totalImprovements["responseTimeGain"] = m_totalImprovements.value("responseTimeGain", 0.0).toDouble() + result.improvements.value("responseTimeGain", 0.0).toDouble();
    }
}