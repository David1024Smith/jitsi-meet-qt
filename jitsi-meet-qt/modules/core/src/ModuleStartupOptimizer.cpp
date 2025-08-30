#include "ModuleStartupOptimizer.h"
#include <QDebug>
#include <QMutexLocker>
#include <QReadLocker>
#include <QWriteLocker>
#include <QCoreApplication>
#include <QDateTime>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <algorithm>

ModuleStartupOptimizer* ModuleStartupOptimizer::s_instance = nullptr;
QMutex ModuleStartupOptimizer::s_mutex;

ModuleStartupOptimizer* ModuleStartupOptimizer::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new ModuleStartupOptimizer();
    }
    return s_instance;
}

ModuleStartupOptimizer::ModuleStartupOptimizer(QObject* parent)
    : QObject(parent)
    , m_parallelLoadingEnabled(true)
    , m_lazyLoadingEnabled(true)
    , m_preloadingEnabled(true)
    , m_memoryOptimizationEnabled(true)
    , m_dependencyOptimizationEnabled(true)
    , m_maxParallelLoads(4)
    , m_preloadDelay(1000)
    , m_lazyLoadTimeout(30000)
    , m_optimizationInProgress(false)
    , m_preloadTimer(new QTimer(this))
    , m_lazyLoadTimer(new QTimer(this))
    , m_metricsTimer(new QTimer(this))
    , m_threadPool(new QThreadPool(this))
{
    initializeSystem();
}

ModuleStartupOptimizer::~ModuleStartupOptimizer()
{
    shutdownSystem();
}

void ModuleStartupOptimizer::initializeSystem()
{
    // 初始化默认性能配置
    m_currentProfile = PerformanceProfile();
    m_currentProfile.profileName = "Default";
    m_currentProfile.optimizationLevel = Basic;
    m_currentProfile.maxParallelLoads = m_maxParallelLoads;
    m_currentProfile.preloadDelay = m_preloadDelay;
    m_currentProfile.lazyLoadTimeout = m_lazyLoadTimeout;
    
    // 初始化指标
    m_metrics = StartupMetrics();
    
    // 配置定时器
    m_preloadTimer->setSingleShot(false);
    m_preloadTimer->setInterval(1000);  // 1秒检查一次预加载队列
    connect(m_preloadTimer, &QTimer::timeout, this, &ModuleStartupOptimizer::processPreloadQueue);
    
    m_lazyLoadTimer->setSingleShot(false);
    m_lazyLoadTimer->setInterval(5000);  // 5秒检查一次延迟加载队列
    connect(m_lazyLoadTimer, &QTimer::timeout, this, &ModuleStartupOptimizer::processLazyLoadQueue);
    
    m_metricsTimer->setSingleShot(false);
    m_metricsTimer->setInterval(10000);  // 10秒更新一次指标
    connect(m_metricsTimer, &QTimer::timeout, this, &ModuleStartupOptimizer::updateMetrics);
    
    // 配置线程池
    m_threadPool->setMaxThreadCount(qMax(2, QThread::idealThreadCount()));
    
    qDebug() << "ModuleStartupOptimizer initialized";
}

void ModuleStartupOptimizer::shutdownSystem()
{
    // 停止所有定时器
    m_preloadTimer->stop();
    m_lazyLoadTimer->stop();
    m_metricsTimer->stop();
    
    // 等待所有任务完成
    m_threadPool->waitForDone(5000);
    
    // 清理队列
    {
        QMutexLocker locker(&m_queueLock);
        m_preloadQueue.clear();
        m_lazyLoadQueue.clear();
        m_preloadSchedule.clear();
        m_lazyLoadSchedule.clear();
    }
    
    qDebug() << "ModuleStartupOptimizer shutdown completed";
}

void ModuleStartupOptimizer::initialize()
{
    m_preloadTimer->start();
    m_lazyLoadTimer->start();
    m_metricsTimer->start();
    
    qDebug() << "ModuleStartupOptimizer started";
}

void ModuleStartupOptimizer::shutdown()
{
    shutdownSystem();
}

void ModuleStartupOptimizer::setPerformanceProfile(const PerformanceProfile& profile)
{
    QWriteLocker locker(&m_configLock);
    m_currentProfile = profile;
    
    // 应用配置
    m_maxParallelLoads = profile.maxParallelLoads;
    m_preloadDelay = profile.preloadDelay;
    m_lazyLoadTimeout = profile.lazyLoadTimeout;
    m_memoryOptimizationEnabled = profile.memoryOptimizationEnabled;
    m_dependencyOptimizationEnabled = profile.dependencyOptimizationEnabled;
    
    qDebug() << "Performance profile set:" << profile.profileName;
}

ModuleStartupOptimizer::PerformanceProfile ModuleStartupOptimizer::getPerformanceProfile() const
{
    QReadLocker locker(&m_configLock);
    return m_currentProfile;
}

void ModuleStartupOptimizer::setOptimizationLevel(OptimizationLevel level)
{
    QWriteLocker locker(&m_configLock);
    m_currentProfile.optimizationLevel = level;
    
    // 根据优化级别调整参数
    switch (level) {
    case None:
        m_parallelLoadingEnabled = false;
        m_lazyLoadingEnabled = false;
        m_preloadingEnabled = false;
        break;
    case Basic:
        m_parallelLoadingEnabled = true;
        m_maxParallelLoads = 2;
        break;
    case Aggressive:
        m_parallelLoadingEnabled = true;
        m_maxParallelLoads = 8;
        m_preloadingEnabled = true;
        break;
    case Adaptive:
        // 自适应模式会根据系统性能动态调整
        analyzeLoadPerformance();
        break;
    }
    
    qDebug() << "Optimization level set to:" << level;
}

ModuleStartupOptimizer::OptimizationLevel ModuleStartupOptimizer::getOptimizationLevel() const
{
    QReadLocker locker(&m_configLock);
    return m_currentProfile.optimizationLevel;
}

void ModuleStartupOptimizer::setModuleLoadInfo(const QString& moduleName, const ModuleLoadInfo& info)
{
    QWriteLocker locker(&m_configLock);
    m_moduleConfigs[moduleName] = info;
    
    qDebug() << "Module load info set for:" << moduleName;
}

ModuleStartupOptimizer::ModuleLoadInfo ModuleStartupOptimizer::getModuleLoadInfo(const QString& moduleName) const
{
    QReadLocker locker(&m_configLock);
    return m_moduleConfigs.value(moduleName, ModuleLoadInfo());
}

void ModuleStartupOptimizer::setModuleLoadStrategy(const QString& moduleName, LoadStrategy strategy)
{
    QWriteLocker locker(&m_configLock);
    if (m_moduleConfigs.contains(moduleName)) {
        m_moduleConfigs[moduleName].strategy = strategy;
    } else {
        ModuleLoadInfo info;
        info.moduleName = moduleName;
        info.strategy = strategy;
        m_moduleConfigs[moduleName] = info;
    }
    
    qDebug() << "Load strategy set for" << moduleName << ":" << strategy;
}

void ModuleStartupOptimizer::setModulePriority(const QString& moduleName, int priority)
{
    QWriteLocker locker(&m_configLock);
    if (m_moduleConfigs.contains(moduleName)) {
        m_moduleConfigs[moduleName].priority = priority;
    } else {
        ModuleLoadInfo info;
        info.moduleName = moduleName;
        info.priority = priority;
        m_moduleConfigs[moduleName] = info;
    }
    
    qDebug() << "Priority set for" << moduleName << ":" << priority;
}

void ModuleStartupOptimizer::setModuleDependencies(const QString& moduleName, const QStringList& dependencies)
{
    QWriteLocker locker(&m_configLock);
    if (m_moduleConfigs.contains(moduleName)) {
        m_moduleConfigs[moduleName].dependencies = dependencies;
    } else {
        ModuleLoadInfo info;
        info.moduleName = moduleName;
        info.dependencies = dependencies;
        m_moduleConfigs[moduleName] = info;
    }
    
    qDebug() << "Dependencies set for" << moduleName << ":" << dependencies;
}

QString ModuleStartupOptimizer::startLoadSession(const QStringList& modules)
{
    QString sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    LoadSession session;
    session.sessionId = sessionId;
    session.modulesToLoad = modules;
    session.startTime = QDateTime::currentMSecsSinceEpoch();
    
    {
        QWriteLocker locker(&m_sessionLock);
        m_loadSessions[sessionId] = session;
    }
    
    emit loadSessionStarted(sessionId, modules);
    
    // 开始加载过程
    if (m_parallelLoadingEnabled && modules.size() > 1) {
        loadModuleBatch(modules, sessionId);
    } else {
        for (const QString& moduleName : modules) {
            loadModuleAsync(moduleName, sessionId);
        }
    }
    
    qDebug() << "Load session started:" << sessionId << "Modules:" << modules.size();
    return sessionId;
}

bool ModuleStartupOptimizer::stopLoadSession(const QString& sessionId)
{
    QWriteLocker locker(&m_sessionLock);
    
    if (!m_loadSessions.contains(sessionId)) {
        return false;
    }
    
    LoadSession& session = m_loadSessions[sessionId];
    session.endTime = QDateTime::currentMSecsSinceEpoch();
    session.totalLoadTime = session.endTime - session.startTime;
    
    emit loadSessionCompleted(sessionId, session);
    
    qDebug() << "Load session stopped:" << sessionId;
    return true;
}

void ModuleStartupOptimizer::enableParallelLoading(bool enabled, int maxParallel)
{
    m_parallelLoadingEnabled = enabled;
    m_maxParallelLoads = maxParallel;
    m_threadPool->setMaxThreadCount(qMax(1, maxParallel));
    
    qDebug() << "Parallel loading" << (enabled ? "enabled" : "disabled") << "Max parallel:" << maxParallel;
}

void ModuleStartupOptimizer::enableLazyLoading(bool enabled, qint64 timeoutMs)
{
    m_lazyLoadingEnabled = enabled;
    m_lazyLoadTimeout = timeoutMs;
    
    qDebug() << "Lazy loading" << (enabled ? "enabled" : "disabled") << "Timeout:" << timeoutMs;
}

void ModuleStartupOptimizer::enablePreloading(bool enabled, qint64 delayMs)
{
    m_preloadingEnabled = enabled;
    m_preloadDelay = delayMs;
    
    qDebug() << "Preloading" << (enabled ? "enabled" : "disabled") << "Delay:" << delayMs;
}

void ModuleStartupOptimizer::schedulePreload(const QString& moduleName, qint64 delayMs)
{
    if (!m_preloadingEnabled) {
        return;
    }
    
    qint64 scheduleTime = QDateTime::currentMSecsSinceEpoch() + (delayMs > 0 ? delayMs : m_preloadDelay);
    
    {
        QMutexLocker locker(&m_queueLock);
        m_preloadSchedule[moduleName] = scheduleTime;
        if (!m_preloadQueue.contains(moduleName)) {
            m_preloadQueue.enqueue(moduleName);
        }
    }
    
    emit preloadScheduled(moduleName, delayMs);
    
    qDebug() << "Preload scheduled for" << moduleName << "in" << delayMs << "ms";
}

void ModuleStartupOptimizer::scheduleLazyLoad(const QString& moduleName, qint64 timeoutMs)
{
    if (!m_lazyLoadingEnabled) {
        return;
    }
    
    qint64 scheduleTime = QDateTime::currentMSecsSinceEpoch() + (timeoutMs > 0 ? timeoutMs : m_lazyLoadTimeout);
    
    {
        QMutexLocker locker(&m_queueLock);
        m_lazyLoadSchedule[moduleName] = scheduleTime;
        if (!m_lazyLoadQueue.contains(moduleName)) {
            m_lazyLoadQueue.enqueue(moduleName);
        }
    }
    
    qDebug() << "Lazy load scheduled for" << moduleName << "timeout:" << timeoutMs;
}

void ModuleStartupOptimizer::processPreloadQueue()
{
    QMutexLocker locker(&m_queueLock);
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    while (!m_preloadQueue.isEmpty()) {
        QString moduleName = m_preloadQueue.head();
        qint64 scheduleTime = m_preloadSchedule.value(moduleName, 0);
        
        if (currentTime >= scheduleTime) {
            m_preloadQueue.dequeue();
            m_preloadSchedule.remove(moduleName);
            
            locker.unlock();
            
            // 执行预加载
            loadModuleAsync(moduleName, "preload");
            emit preloadCompleted(moduleName);
            
            locker.relock();
        } else {
            break;  // 还没到时间
        }
    }
}

void ModuleStartupOptimizer::processLazyLoadQueue()
{
    QMutexLocker locker(&m_queueLock);
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    while (!m_lazyLoadQueue.isEmpty()) {
        QString moduleName = m_lazyLoadQueue.head();
        qint64 scheduleTime = m_lazyLoadSchedule.value(moduleName, 0);
        
        if (currentTime >= scheduleTime) {
            m_lazyLoadQueue.dequeue();
            m_lazyLoadSchedule.remove(moduleName);
            
            locker.unlock();
            
            // 执行延迟加载
            loadModuleAsync(moduleName, "lazy");
            emit lazyLoadTriggered(moduleName);
            
            locker.relock();
        } else {
            break;  // 还没到时间
        }
    }
}

void ModuleStartupOptimizer::loadModuleAsync(const QString& moduleName, const QString& sessionId)
{
    AsyncModuleLoadTask* task = new AsyncModuleLoadTask(moduleName, sessionId, this);
    m_threadPool->start(task);
    
    emit moduleLoadStarted(moduleName, sessionId);
}

void ModuleStartupOptimizer::loadModuleBatch(const QStringList& modules, const QString& sessionId)
{
    BatchModuleLoadTask* task = new BatchModuleLoadTask(modules, sessionId, this);
    m_threadPool->start(task);
}

ModuleStartupOptimizer::StartupMetrics ModuleStartupOptimizer::getStartupMetrics() const
{
    QMutexLocker locker(&m_metricsLock);
    return m_metrics;
}

void ModuleStartupOptimizer::updateMetrics()
{
    QMutexLocker locker(&m_metricsLock);
    
    // 更新统计信息
    m_metrics.totalModules = m_moduleConfigs.size();
    
    // 计算平均加载时间
    if (!m_moduleLoadTimes.isEmpty()) {
        qint64 total = 0;
        for (auto it = m_moduleLoadTimes.begin(); it != m_moduleLoadTimes.end(); ++it) {
            total += it.value();
        }
        m_metrics.averageLoadTime = (double)total / m_moduleLoadTimes.size();
    }
    
    qDebug() << "Metrics updated - Total modules:" << m_metrics.totalModules 
             << "Average load time:" << m_metrics.averageLoadTime;
}

void ModuleStartupOptimizer::analyzeLoadPerformance()
{
    // 分析加载性能并优化策略
    QMutexLocker locker(&m_metricsLock);
    
    if (m_metrics.averageLoadTime > 5000) {  // 5秒
        // 加载时间过长，增加并行度
        m_maxParallelLoads = qMin(8, m_maxParallelLoads + 1);
        qDebug() << "Increased parallel loads to:" << m_maxParallelLoads;
    } else if (m_metrics.averageLoadTime < 1000) {  // 1秒
        // 加载很快，可以减少并行度以节省资源
        m_maxParallelLoads = qMax(2, m_maxParallelLoads - 1);
        qDebug() << "Decreased parallel loads to:" << m_maxParallelLoads;
    }
}

// AsyncModuleLoadTask 实现
AsyncModuleLoadTask::AsyncModuleLoadTask(const QString& moduleName, const QString& sessionId, 
                                       ModuleStartupOptimizer* optimizer)
    : m_moduleName(moduleName)
    , m_sessionId(sessionId)
    , m_optimizer(optimizer)
{
    setAutoDelete(true);
}

void AsyncModuleLoadTask::run()
{
    if (!m_optimizer) {
        return;
    }
    
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    
    try {
        // 模拟模块加载过程
        // 实际实现中这里会调用真正的模块加载逻辑
        QThread::msleep(100 + (qrand() % 500));  // 模拟加载时间
        
        qint64 loadTime = QDateTime::currentMSecsSinceEpoch() - startTime;
        
        // 记录加载时间
        {
            QMutexLocker locker(&m_optimizer->m_metricsLock);
            m_optimizer->m_moduleLoadTimes[m_moduleName] = loadTime;
            m_optimizer->m_metrics.loadedModules++;
        }
        
        emit m_optimizer->moduleLoadCompleted(m_moduleName, m_sessionId, loadTime);
        
    } catch (const std::exception& e) {
        emit m_optimizer->moduleLoadFailed(m_moduleName, m_sessionId, e.what());
        
        QMutexLocker locker(&m_optimizer->m_metricsLock);
        m_optimizer->m_metrics.failedModules++;
    }
}

// BatchModuleLoadTask 实现
BatchModuleLoadTask::BatchModuleLoadTask(const QStringList& modules, const QString& sessionId,
                                       ModuleStartupOptimizer* optimizer)
    : m_modules(modules)
    , m_sessionId(sessionId)
    , m_optimizer(optimizer)
{
    setAutoDelete(true);
}

void BatchModuleLoadTask::run()
{
    if (!m_optimizer) {
        return;
    }
    
    // 优化加载顺序
    QStringList optimizedOrder = m_optimizer->optimizeLoadOrder(m_modules);
    
    // 创建加载批次
    QList<QStringList> batches = m_optimizer->createLoadBatches(optimizedOrder);
    
    // 按批次加载
    for (const QStringList& batch : batches) {
        QList<AsyncModuleLoadTask*> tasks;
        
        // 启动批次中的所有任务
        for (const QString& moduleName : batch) {
            AsyncModuleLoadTask* task = new AsyncModuleLoadTask(moduleName, m_sessionId, m_optimizer);
            tasks.append(task);
            m_optimizer->m_threadPool->start(task);
        }
        
        // 等待批次完成（简化实现）
        QThread::msleep(50);
    }
}

QStringList ModuleStartupOptimizer::optimizeLoadOrder(const QStringList& modules) const
{
    QReadLocker locker(&m_configLock);
    
    // 创建模块优先级映射
    QList<QPair<int, QString>> priorityList;
    
    for (const QString& moduleName : modules) {
        ModuleLoadInfo info = m_moduleConfigs.value(moduleName, ModuleLoadInfo());
        priorityList.append(qMakePair(info.priority, moduleName));
    }
    
    // 按优先级排序（优先级高的先加载）
    std::sort(priorityList.begin(), priorityList.end(), 
              [](const QPair<int, QString>& a, const QPair<int, QString>& b) {
                  return a.first > b.first;
              });
    
    QStringList result;
    for (const auto& pair : priorityList) {
        result.append(pair.second);
    }
    
    return result;
}

QList<QStringList> ModuleStartupOptimizer::createLoadBatches(const QStringList& modules) const
{
    QList<QStringList> batches;
    QStringList currentBatch;
    QStringList loaded;
    QStringList remaining = modules;
    
    while (!remaining.isEmpty()) {
        QStringList readyToLoad = getReadyToLoadModules(remaining, loaded);
        
        if (readyToLoad.isEmpty()) {
            // 如果没有模块准备好加载，可能存在循环依赖
            // 强制加载第一个模块
            readyToLoad.append(remaining.first());
        }
        
        // 限制批次大小
        int batchSize = qMin(readyToLoad.size(), m_maxParallelLoads);
        QStringList batch = readyToLoad.mid(0, batchSize);
        
        batches.append(batch);
        
        // 更新状态
        for (const QString& moduleName : batch) {
            loaded.append(moduleName);
            remaining.removeAll(moduleName);
        }
    }
    
    return batches;
}

QStringList ModuleStartupOptimizer::getReadyToLoadModules(const QStringList& remaining, const QStringList& loaded) const
{
    QReadLocker locker(&m_configLock);
    QStringList ready;
    
    for (const QString& moduleName : remaining) {
        ModuleLoadInfo info = m_moduleConfigs.value(moduleName, ModuleLoadInfo());
        
        // 检查依赖是否已满足
        bool dependenciesSatisfied = true;
        for (const QString& dep : info.dependencies) {
            if (!loaded.contains(dep)) {
                dependenciesSatisfied = false;
                break;
            }
        }
        
        if (dependenciesSatisfied) {
            ready.append(moduleName);
        }
    }
    
    return ready;
}