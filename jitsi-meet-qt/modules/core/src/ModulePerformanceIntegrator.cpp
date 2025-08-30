#include "ModulePerformanceIntegrator.h"
#include <QDebug>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

ModulePerformanceIntegrator* ModulePerformanceIntegrator::s_instance = nullptr;
QMutex ModulePerformanceIntegrator::s_mutex;

ModulePerformanceIntegrator* ModulePerformanceIntegrator::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new ModulePerformanceIntegrator();
    }
    return s_instance;
}

ModulePerformanceIntegrator::ModulePerformanceIntegrator(QObject* parent)
    : QObject(parent)
    , m_communicationBus(nullptr)
    , m_resourceManager(nullptr)
    , m_startupOptimizer(nullptr)
    , m_autoOptimizationEnabled(true)
    , m_optimizationInterval(300)  // 5分钟
    , m_memoryThreshold(512 * 1024 * 1024)  // 512MB
    , m_cpuThreshold(80.0)  // 80%
    , m_optimizationTimer(new QTimer(this))
    , m_metricsTimer(new QTimer(this))
    , m_optimizationActive(false)
    , m_optimizationPaused(false)
    , m_currentProfileName("Default")
{
    initializeComponents();
    connectSignals();
    loadPerformanceProfiles();
}

ModulePerformanceIntegrator::~ModulePerformanceIntegrator()
{
    shutdown();
    savePerformanceProfiles();
}

void ModulePerformanceIntegrator::initializeComponents()
{
    // 获取核心组件实例
    m_communicationBus = ModuleCommunicationBus::instance();
    m_resourceManager = ModuleResourceManager::instance();
    m_startupOptimizer = ModuleStartupOptimizer::instance();
    
    // 配置定时器
    m_optimizationTimer->setSingleShot(false);
    m_optimizationTimer->setInterval(m_optimizationInterval * 1000);
    connect(m_optimizationTimer, &QTimer::timeout, this, &ModulePerformanceIntegrator::performPeriodicOptimization);
    
    m_metricsTimer->setSingleShot(false);
    m_metricsTimer->setInterval(30000);  // 30秒更新一次指标
    connect(m_metricsTimer, &QTimer::timeout, this, &ModulePerformanceIntegrator::updatePerformanceMetrics);
    
    qDebug() << "ModulePerformanceIntegrator components initialized";
}

void ModulePerformanceIntegrator::connectSignals()
{
    // 连接通信总线信号
    if (m_communicationBus) {
        connect(m_communicationBus, &ModuleCommunicationBus::performanceAlert,
                this, &ModulePerformanceIntegrator::onCommunicationPerformanceAlert);
    }
    
    // 连接资源管理器信号
    if (m_resourceManager) {
        connect(m_resourceManager, &ModuleResourceManager::memoryWarning,
                this, &ModulePerformanceIntegrator::onResourceMemoryWarning);
    }
    
    qDebug() << "ModulePerformanceIntegrator signals connected";
}

void ModulePerformanceIntegrator::initialize()
{
    if (m_communicationBus) {
        m_communicationBus->start();
    }
    
    if (m_resourceManager) {
        m_resourceManager->initialize();
    }
    
    if (m_startupOptimizer) {
        m_startupOptimizer->initialize();
    }
    
    // 启动监控
    m_metricsTimer->start();
    
    if (m_autoOptimizationEnabled) {
        startOptimization();
    }
    
    qDebug() << "ModulePerformanceIntegrator initialized";
}

void ModulePerformanceIntegrator::shutdown()
{
    stopOptimization();
    m_metricsTimer->stop();
    
    if (m_communicationBus) {
        m_communicationBus->stop();
    }
    
    if (m_resourceManager) {
        m_resourceManager->shutdown();
    }
    
    if (m_startupOptimizer) {
        m_startupOptimizer->shutdown();
    }
    
    qDebug() << "ModulePerformanceIntegrator shutdown completed";
}

void ModulePerformanceIntegrator::startOptimization()
{
    if (m_optimizationActive) {
        return;
    }
    
    m_optimizationActive = true;
    m_optimizationPaused = false;
    
    if (m_autoOptimizationEnabled) {
        m_optimizationTimer->start();
    }
    
    qDebug() << "Performance optimization started";
}

void ModulePerformanceIntegrator::stopOptimization()
{
    m_optimizationActive = false;
    m_optimizationTimer->stop();
    
    qDebug() << "Performance optimization stopped";
}

void ModulePerformanceIntegrator::pauseOptimization()
{
    m_optimizationPaused = true;
    qDebug() << "Performance optimization paused";
}

void ModulePerformanceIntegrator::resumeOptimization()
{
    m_optimizationPaused = false;
    qDebug() << "Performance optimization resumed";
}

ModulePerformanceIntegrator::SystemPerformanceMetrics ModulePerformanceIntegrator::getSystemMetrics() const
{
    QMutexLocker locker(&m_metricsLock);
    return m_currentMetrics;
}

void ModulePerformanceIntegrator::updatePerformanceMetrics()
{
    SystemPerformanceMetrics metrics;
    
    // 收集通信性能指标
    if (m_communicationBus) {
        metrics.communicationMetrics = m_communicationBus->getPerformanceMetrics();
        metrics.totalMessages = metrics.communicationMetrics.totalMessages;
    }
    
    // 收集资源管理指标
    if (m_resourceManager) {
        metrics.resourceMetrics = m_resourceManager->getCacheStatistics();
        metrics.totalMemoryUsage = m_resourceManager->getMemoryUsage();
        metrics.totalResources = metrics.resourceMetrics.itemCount;
    }
    
    // 收集启动优化指标
    if (m_startupOptimizer) {
        metrics.startupMetrics = m_startupOptimizer->getStartupMetrics();
        metrics.activeModules = metrics.startupMetrics.loadedModules;
    }
    
    // 计算系统整体指标
    metrics.peakMemoryUsage = qMax(metrics.totalMemoryUsage, m_currentMetrics.peakMemoryUsage);
    
    // 计算性能评分
    metrics.performanceScore = calculatePerformanceScore(metrics);
    metrics.performanceLevel = getPerformanceLevel(metrics.performanceScore);
    
    {
        QMutexLocker locker(&m_metricsLock);
        m_currentMetrics = metrics;
    }
    
    emit performanceMetricsUpdated(metrics);
    
    // 检查性能阈值
    checkPerformanceThresholds();
    
    // 生成优化建议
    generateOptimizationRecommendations();
}

int ModulePerformanceIntegrator::calculatePerformanceScore(const SystemPerformanceMetrics& metrics) const
{
    int score = 100;
    
    // 内存使用评分 (30%)
    double memoryRatio = (double)metrics.totalMemoryUsage / m_memoryThreshold;
    if (memoryRatio > 1.0) {
        score -= 30;
    } else if (memoryRatio > 0.8) {
        score -= (int)(20 * (memoryRatio - 0.8) / 0.2);
    }
    
    // 通信性能评分 (25%)
    if (metrics.communicationMetrics.averageLatency > 1000) {  // 1秒
        score -= 25;
    } else if (metrics.communicationMetrics.averageLatency > 500) {  // 0.5秒
        score -= (int)(15 * (metrics.communicationMetrics.averageLatency - 500) / 500);
    }
    
    // 缓存命中率评分 (20%)
    if (metrics.resourceMetrics.hitRatio < 0.5) {  // 50%
        score -= 20;
    } else if (metrics.resourceMetrics.hitRatio < 0.8) {  // 80%
        score -= (int)(10 * (0.8 - metrics.resourceMetrics.hitRatio) / 0.3);
    }
    
    // 启动性能评分 (15%)
    if (metrics.startupMetrics.averageLoadTime > 5000) {  // 5秒
        score -= 15;
    } else if (metrics.startupMetrics.averageLoadTime > 2000) {  // 2秒
        score -= (int)(10 * (metrics.startupMetrics.averageLoadTime - 2000) / 3000);
    }
    
    // 消息丢包率评分 (10%)
    if (metrics.communicationMetrics.totalMessages > 0) {
        double dropRate = (double)metrics.communicationMetrics.droppedMessages / metrics.communicationMetrics.totalMessages;
        if (dropRate > 0.05) {  // 5%
            score -= 10;
        } else if (dropRate > 0.01) {  // 1%
            score -= (int)(5 * (dropRate - 0.01) / 0.04);
        }
    }
    
    return qMax(0, qMin(100, score));
}

QString ModulePerformanceIntegrator::getPerformanceLevel(int score) const
{
    if (score >= 90) return "Excellent";
    if (score >= 75) return "Good";
    if (score >= 60) return "Fair";
    return "Poor";
}

void ModulePerformanceIntegrator::generateOptimizationRecommendations()
{
    QList<OptimizationRecommendation> recommendations;
    
    SystemPerformanceMetrics metrics = getSystemMetrics();
    
    // 内存优化建议
    if (metrics.totalMemoryUsage > m_memoryThreshold * 0.8) {
        OptimizationRecommendation rec;
        rec.category = "Memory";
        rec.issue = "High memory usage detected";
        rec.recommendation = "Consider clearing unused resources and optimizing cache";
        rec.action = "optimizeMemoryUsage";
        rec.priority = 4;
        rec.autoApplicable = true;
        recommendations.append(rec);
    }
    
    // 通信优化建议
    if (metrics.communicationMetrics.averageLatency > 500) {
        OptimizationRecommendation rec;
        rec.category = "Communication";
        rec.issue = "High message latency detected";
        rec.recommendation = "Optimize message processing and reduce batch size";
        rec.action = "optimizeCommunication";
        rec.priority = 3;
        rec.autoApplicable = true;
        recommendations.append(rec);
    }
    
    // 缓存优化建议
    if (metrics.resourceMetrics.hitRatio < 0.7) {
        OptimizationRecommendation rec;
        rec.category = "Cache";
        rec.issue = "Low cache hit ratio";
        rec.recommendation = "Adjust cache policies and increase cache size";
        rec.action = "optimizeResourceUsage";
        rec.priority = 2;
        rec.autoApplicable = true;
        recommendations.append(rec);
    }
    
    // 启动优化建议
    if (metrics.startupMetrics.averageLoadTime > 3000) {
        OptimizationRecommendation rec;
        rec.category = "Startup";
        rec.issue = "Slow module loading detected";
        rec.recommendation = "Enable parallel loading and preloading for critical modules";
        rec.action = "optimizeStartupPerformance";
        rec.priority = 3;
        rec.autoApplicable = true;
        recommendations.append(rec);
    }
    
    {
        QMutexLocker locker(&m_metricsLock);
        m_recommendations = recommendations;
    }
    
    // 发出建议信号
    for (const auto& rec : recommendations) {
        emit optimizationRecommendationAvailable(rec);
    }
}

QList<ModulePerformanceIntegrator::OptimizationRecommendation> ModulePerformanceIntegrator::getOptimizationRecommendations() const
{
    QMutexLocker locker(&m_metricsLock);
    return m_recommendations;
}

void ModulePerformanceIntegrator::enableAutoOptimization(bool enabled)
{
    m_autoOptimizationEnabled = enabled;
    
    if (enabled && m_optimizationActive) {
        m_optimizationTimer->start();
    } else {
        m_optimizationTimer->stop();
    }
    
    qDebug() << "Auto optimization" << (enabled ? "enabled" : "disabled");
}

bool ModulePerformanceIntegrator::isAutoOptimizationEnabled() const
{
    return m_autoOptimizationEnabled;
}

void ModulePerformanceIntegrator::setOptimizationInterval(int seconds)
{
    m_optimizationInterval = seconds;
    m_optimizationTimer->setInterval(seconds * 1000);
    
    qDebug() << "Optimization interval set to" << seconds << "seconds";
}

void ModulePerformanceIntegrator::optimizeMemoryUsage()
{
    if (m_resourceManager) {
        m_resourceManager->compactMemory();
        m_resourceManager->freeUnusedResources();
    }
    
    emit optimizationCompleted("Memory", "Memory optimization completed");
    qDebug() << "Memory optimization completed";
}

void ModulePerformanceIntegrator::optimizeCommunication()
{
    if (m_communicationBus) {
        // 调整批处理大小
        SystemPerformanceMetrics metrics = getSystemMetrics();
        if (metrics.communicationMetrics.averageLatency > 1000) {
            m_communicationBus->setBatchSize(50);  // 减少批处理大小
        } else if (metrics.communicationMetrics.averageLatency < 100) {
            m_communicationBus->setBatchSize(200);  // 增加批处理大小
        }
        
        // 调整处理间隔
        if (metrics.communicationMetrics.queueSize > 1000) {
            m_communicationBus->setProcessingInterval(5);  // 更频繁处理
        }
    }
    
    emit optimizationCompleted("Communication", "Communication optimization completed");
    qDebug() << "Communication optimization completed";
}

void ModulePerformanceIntegrator::optimizeStartupPerformance()
{
    if (m_startupOptimizer) {
        // 启用并行加载
        m_startupOptimizer->enableParallelLoading(true, 6);
        
        // 启用预加载
        m_startupOptimizer->enablePreloading(true, 500);
        
        // 分析并优化加载策略
        m_startupOptimizer->optimizeForNextStartup();
    }
    
    emit optimizationCompleted("Startup", "Startup optimization completed");
    qDebug() << "Startup optimization completed";
}

void ModulePerformanceIntegrator::optimizeResourceUsage()
{
    if (m_resourceManager) {
        // 优化缓存
        m_resourceManager->optimizeCache();
        
        // 调整缓存大小
        SystemPerformanceMetrics metrics = getSystemMetrics();
        if (metrics.resourceMetrics.hitRatio < 0.7) {
            qint64 newCacheSize = metrics.resourceMetrics.maxSize * 1.5;
            m_resourceManager->setCacheMaxSize(newCacheSize);
        }
    }
    
    emit optimizationCompleted("Resource", "Resource optimization completed");
    qDebug() << "Resource optimization completed";
}

void ModulePerformanceIntegrator::performFullOptimization()
{
    optimizeMemoryUsage();
    optimizeCommunication();
    optimizeStartupPerformance();
    optimizeResourceUsage();
    
    emit optimizationCompleted("Full", "Full system optimization completed");
    qDebug() << "Full system optimization completed";
}

void ModulePerformanceIntegrator::performPeriodicOptimization()
{
    if (m_optimizationPaused || !m_autoOptimizationEnabled) {
        return;
    }
    
    if (shouldTriggerAutoOptimization()) {
        executeAutoOptimization();
    }
}

bool ModulePerformanceIntegrator::shouldTriggerAutoOptimization() const
{
    SystemPerformanceMetrics metrics = getSystemMetrics();
    
    // 检查是否需要优化
    if (metrics.performanceScore < 70) {
        return true;
    }
    
    if (metrics.totalMemoryUsage > m_memoryThreshold * 0.9) {
        return true;
    }
    
    if (metrics.communicationMetrics.averageLatency > 1000) {
        return true;
    }
    
    return false;
}

void ModulePerformanceIntegrator::executeAutoOptimization()
{
    QList<OptimizationRecommendation> recommendations = getOptimizationRecommendations();
    
    for (const auto& rec : recommendations) {
        if (rec.autoApplicable && rec.priority >= 3) {
            if (rec.action == "optimizeMemoryUsage") {
                optimizeMemoryUsage();
            } else if (rec.action == "optimizeCommunication") {
                optimizeCommunication();
            } else if (rec.action == "optimizeStartupPerformance") {
                optimizeStartupPerformance();
            } else if (rec.action == "optimizeResourceUsage") {
                optimizeResourceUsage();
            }
            
            emit autoOptimizationTriggered(rec.issue);
        }
    }
}

void ModulePerformanceIntegrator::onCommunicationPerformanceAlert(const QString& alert)
{
    emit performanceAlert(QString("Communication: %1").arg(alert), 2);
    
    // 触发通信优化
    if (m_autoOptimizationEnabled) {
        optimizeCommunication();
    }
}

void ModulePerformanceIntegrator::onResourceMemoryWarning(qint64 currentUsage, qint64 maxUsage)
{
    QString alert = QString("Memory usage: %1/%2 MB").arg(currentUsage / 1024 / 1024).arg(maxUsage / 1024 / 1024);
    emit performanceAlert(alert, 3);
    
    // 触发内存优化
    if (m_autoOptimizationEnabled) {
        optimizeMemoryUsage();
    }
}

void ModulePerformanceIntegrator::checkPerformanceThresholds()
{
    SystemPerformanceMetrics metrics = getSystemMetrics();
    
    // 检查内存阈值
    if (metrics.totalMemoryUsage > m_memoryThreshold) {
        emit performanceAlert("Memory threshold exceeded", 4);
    }
    
    // 检查性能评分
    if (metrics.performanceScore < 50) {
        emit performanceAlert("System performance is poor", 4);
    } else if (metrics.performanceScore < 70) {
        emit performanceAlert("System performance needs attention", 2);
    }
}

void ModulePerformanceIntegrator::loadPerformanceProfiles()
{
    // 加载性能配置文件
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString profileFile = QDir(configDir).filePath("performance_profiles.json");
    
    QFile file(profileFile);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject obj = doc.object();
        
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            m_performanceProfiles[it.key()] = it.value().toVariant().toMap();
        }
        
        qDebug() << "Loaded" << m_performanceProfiles.size() << "performance profiles";
    }
}

void ModulePerformanceIntegrator::savePerformanceProfiles()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    QString profileFile = QDir(configDir).filePath("performance_profiles.json");
    
    QJsonObject obj;
    for (auto it = m_performanceProfiles.begin(); it != m_performanceProfiles.end(); ++it) {
        obj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    
    QFile file(profileFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson());
        qDebug() << "Saved performance profiles";
    }
}

ModuleCommunicationBus* ModulePerformanceIntegrator::getCommunicationBus() const
{
    return m_communicationBus;
}

ModuleResourceManager* ModulePerformanceIntegrator::getResourceManager() const
{
    return m_resourceManager;
}

ModuleStartupOptimizer* ModulePerformanceIntegrator::getStartupOptimizer() const
{
    return m_startupOptimizer;
}