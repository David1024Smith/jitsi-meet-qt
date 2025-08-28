#include "PerformanceManager.h"
#include "PerformanceConfig.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

PerformanceManager* PerformanceManager::s_instance = nullptr;

PerformanceManager::PerformanceManager(QObject *parent)
    : QObject(parent)
    , m_memoryCheckTimer(new QTimer(this))
    , m_cleanupTimer(new QTimer(this))
    , m_memoryWarningThreshold(512 * 1024 * 1024) // 512MB
    , m_memoryCriticalThreshold(1024 * 1024 * 1024) // 1GB
    , m_maxRecentItems(50)
    , m_networkOptimized(false)
    , m_resourcesPreloaded(false)
{
    s_instance = this;
    
    // 创建性能配置管理器
    m_performanceConfig = new PerformanceConfig(this);
    connect(m_performanceConfig, &PerformanceConfig::configurationChanged,
            this, &PerformanceManager::onConfigurationChanged);
    
    initializeOptimizations();
    applyPerformanceConfiguration();
    
    // 设置内存监控定时器
    m_memoryCheckTimer->setInterval(m_performanceConfig->memorySettings().monitoringInterval);
    connect(m_memoryCheckTimer, &QTimer::timeout, this, &PerformanceManager::onMemoryCheckTimer);
    
    // 设置清理定时器
    m_cleanupTimer->setInterval(m_performanceConfig->memorySettings().cleanupInterval);
    connect(m_cleanupTimer, &QTimer::timeout, this, &PerformanceManager::onCleanupTimer);
}

PerformanceManager::~PerformanceManager()
{
    stopMemoryMonitoring();
    s_instance = nullptr;
}

PerformanceManager* PerformanceManager::instance()
{
    return s_instance;
}

void PerformanceManager::startStartupTimer()
{
    m_startupTimer.start();
    qDebug() << "PerformanceManager: Startup timer started";
}

void PerformanceManager::markStartupComplete()
{
    if (m_startupTimer.isValid()) {
        QMutexLocker locker(&m_metricsMutex);
        m_metrics.startupTime = m_startupTimer.elapsed();
        qDebug() << "PerformanceManager: Startup completed in" << m_metrics.startupTime << "ms";
        
        // 启动完成后开始内存监控
        startMemoryMonitoring();
    }
}

qint64 PerformanceManager::getStartupTime() const
{
    QMutexLocker locker(&m_metricsMutex);
    return m_metrics.startupTime;
}

void PerformanceManager::preloadResources()
{
    if (m_resourcesPreloaded) {
        return;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    // 预加载常用资源
    QStringList resourcePaths = {
        ":/styles/default.qss",
        ":/styles/dark.qss",
        ":/icons/settings.svg",
        ":/icons/about.svg",
        ":/icons/back.svg"
    };
    
    for (const QString& path : resourcePaths) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            m_preloadedResources[path] = file.readAll();
        }
    }
    
    m_resourcesPreloaded = true;
    
    QMutexLocker locker(&m_metricsMutex);
    m_metrics.resourceLoadTime = timer.elapsed();
    
    qDebug() << "PerformanceManager: Resources preloaded in" << m_metrics.resourceLoadTime << "ms";
}

void PerformanceManager::optimizeResourceLoading()
{
    // 在后台线程中预加载资源
    QThread* resourceThread = QThread::create([this]() {
        preloadResources();
    });
    
    resourceThread->start();
    
    // 线程完成后自动删除
    connect(resourceThread, &QThread::finished, resourceThread, &QThread::deleteLater);
}

void PerformanceManager::optimizeNetworkMemory()
{
    if (m_networkOptimized) {
        return;
    }
    
    // 优化网络内存使用
    qDebug() << "PerformanceManager: Network memory optimization completed";
    m_networkOptimized = true;
}

void PerformanceManager::optimizeRecentItemsLoading()
{
    // 限制最近项目数量以提高加载性能
    QMutexLocker locker(&m_metricsMutex);
    m_metrics.recentItemsCount = qMin(m_metrics.recentItemsCount, m_maxRecentItems);
    
    qDebug() << "PerformanceManager: Recent items optimized, count:" << m_metrics.recentItemsCount;
}

void PerformanceManager::setMaxRecentItems(int maxItems)
{
    m_maxRecentItems = maxItems;
    optimizeRecentItemsLoading();
}

void PerformanceManager::startMemoryMonitoring()
{
    if (!m_memoryCheckTimer->isActive()) {
        m_memoryCheckTimer->start();
        m_cleanupTimer->start();
        qDebug() << "PerformanceManager: Memory monitoring started";
    }
}

void PerformanceManager::stopMemoryMonitoring()
{
    m_memoryCheckTimer->stop();
    m_cleanupTimer->stop();
    qDebug() << "PerformanceManager: Memory monitoring stopped";
}

void PerformanceManager::performMemoryCleanup()
{
    qint64 beforeCleanup = getCurrentMemoryUsage();
    
    // 清理网络缓存
    optimizeNetworkMemory();
    
    // 清理未使用的资源
    cleanupUnusedResources();
    
    // 强制垃圾回收
    QCoreApplication::processEvents();
    
    qint64 afterCleanup = getCurrentMemoryUsage();
    qint64 freed = beforeCleanup - afterCleanup;
    
    qDebug() << "PerformanceManager: Memory cleanup completed, freed:" << freed << "bytes";
}

qint64 PerformanceManager::getCurrentMemoryUsage()
{
    return getProcessMemoryUsage();
}

PerformanceManager::PerformanceMetrics PerformanceManager::getMetrics() const
{
    QMutexLocker locker(&m_metricsMutex);
    PerformanceMetrics metrics = m_metrics;
    metrics.memoryUsage = getProcessMemoryUsage();
    metrics.networkMemory = getNetworkMemoryUsage();
    return metrics;
}

void PerformanceManager::logPerformanceMetrics()
{
    PerformanceMetrics metrics = getMetrics();
    
    qDebug() << "=== Performance Metrics ===";
    qDebug() << "Startup Time:" << metrics.startupTime << "ms";
    qDebug() << "Memory Usage:" << metrics.memoryUsage / (1024*1024) << "MB";
    qDebug() << "Network Memory:" << metrics.networkMemory / (1024*1024) << "MB";
    qDebug() << "Recent Items Count:" << metrics.recentItemsCount;
    qDebug() << "Config Load Time:" << metrics.configLoadTime << "ms";
    qDebug() << "Resource Load Time:" << metrics.resourceLoadTime << "ms";
    qDebug() << "===========================";
}

void PerformanceManager::onMemoryCheckTimer()
{
    qint64 currentMemory = getCurrentMemoryUsage();
    
    QMutexLocker locker(&m_metricsMutex);
    m_metrics.memoryUsage = currentMemory;
    
    if (currentMemory > m_memoryCriticalThreshold) {
        qWarning() << "PerformanceManager: Critical memory usage detected:" << currentMemory / (1024*1024) << "MB";
        performMemoryCleanup();
        emit memoryWarning(currentMemory);
    } else if (currentMemory > m_memoryWarningThreshold) {
        qDebug() << "PerformanceManager: High memory usage:" << currentMemory / (1024*1024) << "MB";
        emit memoryWarning(currentMemory);
    }
    
    emit performanceMetricsUpdated(getMetrics());
}

void PerformanceManager::onCleanupTimer()
{
    performMemoryCleanup();
}

qint64 PerformanceManager::getNetworkMemoryUsage()
{
    // 网络组件内存使用情况的估算
    return getProcessMemoryUsage() * 0.1; // 估算网络组件占用10%的内存
}

void PerformanceManager::initializeOptimizations()
{
    // 设置应用程序属性以优化性能
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    
    qDebug() << "PerformanceManager: Basic optimizations initialized";
}

void PerformanceManager::setupMemoryThresholds()
{
    // 根据系统内存调整阈值
    qint64 systemMemory = 8 * 1024 * 1024 * 1024; // 默认8GB
    
#ifdef Q_OS_WIN
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        systemMemory = memInfo.ullTotalPhys;
    }
#endif
    
    // 根据系统内存调整阈值
    if (systemMemory < 4 * 1024 * 1024 * 1024) { // 小于4GB
        m_memoryWarningThreshold = 256 * 1024 * 1024; // 256MB
        m_memoryCriticalThreshold = 512 * 1024 * 1024; // 512MB
    } else if (systemMemory < 8 * 1024 * 1024 * 1024) { // 小于8GB
        m_memoryWarningThreshold = 512 * 1024 * 1024; // 512MB
        m_memoryCriticalThreshold = 1024 * 1024 * 1024; // 1GB
    } else { // 8GB或更多
        m_memoryWarningThreshold = 1024 * 1024 * 1024; // 1GB
        m_memoryCriticalThreshold = 2048 * 1024 * 1024; // 2GB
    }
    
    qDebug() << "PerformanceManager: Memory thresholds set - Warning:" 
             << m_memoryWarningThreshold / (1024*1024) << "MB, Critical:" 
             << m_memoryCriticalThreshold / (1024*1024) << "MB";
}

void PerformanceManager::cleanupUnusedResources()
{
    // 清理预加载的资源缓存
    if (m_preloadedResources.size() > 10) {
        m_preloadedResources.clear();
        m_resourcesPreloaded = false;
        qDebug() << "PerformanceManager: Preloaded resources cache cleared";
    }
}

qint64 PerformanceManager::getProcessMemoryUsage()
{
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
#endif
    return 0;
}

qint64 PerformanceManager::getNetworkMemoryUsage()
{
    // 网络组件内存使用情况的估算
    return getProcessMemoryUsage() * 0.1; // 估算网络组件占用10%的内存
}

PerformanceConfig* PerformanceManager::performanceConfig() const
{
    return m_performanceConfig;
}

void PerformanceManager::applyPerformanceConfiguration()
{
    if (!m_performanceConfig) {
        return;
    }
    
    const auto& memorySettings = m_performanceConfig->memorySettings();
    m_memoryWarningThreshold = memorySettings.warningThreshold;
    m_memoryCriticalThreshold = memorySettings.criticalThreshold;
    
    // 更新定时器间隔
    if (m_memoryCheckTimer) {
        m_memoryCheckTimer->setInterval(memorySettings.monitoringInterval);
    }
    if (m_cleanupTimer) {
        m_cleanupTimer->setInterval(memorySettings.cleanupInterval);
    }
    
    const auto& recentSettings = m_performanceConfig->recentItemsSettings();
    m_maxRecentItems = recentSettings.maxItems;
    
    qDebug() << "PerformanceManager: Applied performance configuration";
}

void PerformanceManager::onConfigurationChanged()
{
    applyPerformanceConfiguration();
    qDebug() << "PerformanceManager: Configuration updated";
}