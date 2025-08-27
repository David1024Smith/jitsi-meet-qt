#include "PerformanceConfig.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

PerformanceConfig::PerformanceConfig(QObject *parent)
    : QObject(parent)
    , m_settings(nullptr)
    , m_performanceOptimizationEnabled(true)
    , m_systemMemorySize(0)
    , m_cpuCoreCount(0)
{
    // 初始化设置
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    QString settingsPath = configPath + "/performance.ini";
    
    m_settings = new QSettings(settingsPath, QSettings::IniFormat, this);
    
    // 检测系统能力
    detectSystemCapabilities();
    
    // 加载配置
    loadConfiguration();
    
    qDebug() << "PerformanceConfig: Initialized with system memory:" 
             << m_systemMemorySize / (1024*1024) << "MB, CPU cores:" << m_cpuCoreCount;
}

PerformanceConfig::~PerformanceConfig()
{
    saveConfiguration();
}

void PerformanceConfig::loadConfiguration()
{
    m_settings->beginGroup("Performance");
    m_performanceOptimizationEnabled = m_settings->value("enabled", true).toBool();
    m_settings->endGroup();
    
    // 加载内存设置
    m_settings->beginGroup("Memory");
    m_memorySettings.warningThreshold = m_settings->value("warningThreshold", 
                                                         m_memorySettings.warningThreshold).toLongLong();
    m_memorySettings.criticalThreshold = m_settings->value("criticalThreshold", 
                                                          m_memorySettings.criticalThreshold).toLongLong();
    m_memorySettings.cleanupInterval = m_settings->value("cleanupInterval", 
                                                        m_memorySettings.cleanupInterval).toInt();
    m_memorySettings.monitoringInterval = m_settings->value("monitoringInterval", 
                                                           m_memorySettings.monitoringInterval).toInt();
    m_memorySettings.autoCleanupEnabled = m_settings->value("autoCleanupEnabled", 
                                                           m_memorySettings.autoCleanupEnabled).toBool();
    m_memorySettings.leakDetectionEnabled = m_settings->value("leakDetectionEnabled", 
                                                             m_memorySettings.leakDetectionEnabled).toBool();
    m_settings->endGroup();
    
    // 加载启动设置
    m_settings->beginGroup("Startup");
    m_startupSettings.fastStartupEnabled = m_settings->value("fastStartupEnabled", 
                                                            m_startupSettings.fastStartupEnabled).toBool();
    m_startupSettings.resourcePreloadEnabled = m_settings->value("resourcePreloadEnabled", 
                                                                m_startupSettings.resourcePreloadEnabled).toBool();
    m_startupSettings.deferredInitEnabled = m_settings->value("deferredInitEnabled", 
                                                             m_startupSettings.deferredInitEnabled).toBool();
    m_startupSettings.optimizationLevel = m_settings->value("optimizationLevel", 
                                                           m_startupSettings.optimizationLevel).toInt();
    m_startupSettings.maxPreloadResources = m_settings->value("maxPreloadResources", 
                                                             m_startupSettings.maxPreloadResources).toInt();
    m_startupSettings.delayedInitTimeout = m_settings->value("delayedInitTimeout", 
                                                            m_startupSettings.delayedInitTimeout).toInt();
    m_settings->endGroup();
    
    // 加载WebEngine设置
    m_settings->beginGroup("WebEngine");
    m_webEngineSettings.cacheMaxSize = m_settings->value("cacheMaxSize", 
                                                        m_webEngineSettings.cacheMaxSize).toLongLong();
    m_webEngineSettings.diskCacheEnabled = m_settings->value("diskCacheEnabled", 
                                                            m_webEngineSettings.diskCacheEnabled).toBool();
    m_webEngineSettings.memoryOptimizationEnabled = m_settings->value("memoryOptimizationEnabled", 
                                                                     m_webEngineSettings.memoryOptimizationEnabled).toBool();
    m_webEngineSettings.cacheCleanupInterval = m_settings->value("cacheCleanupInterval", 
                                                                m_webEngineSettings.cacheCleanupInterval).toInt();
    m_webEngineSettings.javascriptOptimizationEnabled = m_settings->value("javascriptOptimizationEnabled", 
                                                                         m_webEngineSettings.javascriptOptimizationEnabled).toBool();
    m_settings->endGroup();
    
    // 加载最近项目设置
    m_settings->beginGroup("RecentItems");
    m_recentItemsSettings.maxItems = m_settings->value("maxItems", 
                                                       m_recentItemsSettings.maxItems).toInt();
    m_recentItemsSettings.lazyLoadingEnabled = m_settings->value("lazyLoadingEnabled", 
                                                                m_recentItemsSettings.lazyLoadingEnabled).toBool();
    m_recentItemsSettings.asyncSaveEnabled = m_settings->value("asyncSaveEnabled", 
                                                              m_recentItemsSettings.asyncSaveEnabled).toBool();
    m_recentItemsSettings.optimizationInterval = m_settings->value("optimizationInterval", 
                                                                  m_recentItemsSettings.optimizationInterval).toInt();
    m_recentItemsSettings.searchCacheSize = m_settings->value("searchCacheSize", 
                                                             m_recentItemsSettings.searchCacheSize).toInt();
    m_recentItemsSettings.maxAge = m_settings->value("maxAge", 
                                                     m_recentItemsSettings.maxAge).toInt();
    m_settings->endGroup();
    
    // 如果是首次运行，自动调整配置
    if (!m_settings->contains("Performance/configured")) {
        autoTuneForSystem();
        m_settings->setValue("Performance/configured", true);
    }
    
    qDebug() << "PerformanceConfig: Configuration loaded";
}

void PerformanceConfig::saveConfiguration()
{
    m_settings->beginGroup("Performance");
    m_settings->setValue("enabled", m_performanceOptimizationEnabled);
    m_settings->endGroup();
    
    // 保存内存设置
    m_settings->beginGroup("Memory");
    m_settings->setValue("warningThreshold", m_memorySettings.warningThreshold);
    m_settings->setValue("criticalThreshold", m_memorySettings.criticalThreshold);
    m_settings->setValue("cleanupInterval", m_memorySettings.cleanupInterval);
    m_settings->setValue("monitoringInterval", m_memorySettings.monitoringInterval);
    m_settings->setValue("autoCleanupEnabled", m_memorySettings.autoCleanupEnabled);
    m_settings->setValue("leakDetectionEnabled", m_memorySettings.leakDetectionEnabled);
    m_settings->endGroup();
    
    // 保存启动设置
    m_settings->beginGroup("Startup");
    m_settings->setValue("fastStartupEnabled", m_startupSettings.fastStartupEnabled);
    m_settings->setValue("resourcePreloadEnabled", m_startupSettings.resourcePreloadEnabled);
    m_settings->setValue("deferredInitEnabled", m_startupSettings.deferredInitEnabled);
    m_settings->setValue("optimizationLevel", m_startupSettings.optimizationLevel);
    m_settings->setValue("maxPreloadResources", m_startupSettings.maxPreloadResources);
    m_settings->setValue("delayedInitTimeout", m_startupSettings.delayedInitTimeout);
    m_settings->endGroup();
    
    // 保存WebEngine设置
    m_settings->beginGroup("WebEngine");
    m_settings->setValue("cacheMaxSize", m_webEngineSettings.cacheMaxSize);
    m_settings->setValue("diskCacheEnabled", m_webEngineSettings.diskCacheEnabled);
    m_settings->setValue("memoryOptimizationEnabled", m_webEngineSettings.memoryOptimizationEnabled);
    m_settings->setValue("cacheCleanupInterval", m_webEngineSettings.cacheCleanupInterval);
    m_settings->setValue("javascriptOptimizationEnabled", m_webEngineSettings.javascriptOptimizationEnabled);
    m_settings->endGroup();
    
    // 保存最近项目设置
    m_settings->beginGroup("RecentItems");
    m_settings->setValue("maxItems", m_recentItemsSettings.maxItems);
    m_settings->setValue("lazyLoadingEnabled", m_recentItemsSettings.lazyLoadingEnabled);
    m_settings->setValue("asyncSaveEnabled", m_recentItemsSettings.asyncSaveEnabled);
    m_settings->setValue("optimizationInterval", m_recentItemsSettings.optimizationInterval);
    m_settings->setValue("searchCacheSize", m_recentItemsSettings.searchCacheSize);
    m_settings->setValue("maxAge", m_recentItemsSettings.maxAge);
    m_settings->endGroup();
    
    m_settings->sync();
    
    qDebug() << "PerformanceConfig: Configuration saved";
}

void PerformanceConfig::resetToDefaults()
{
    m_memorySettings = MemorySettings();
    m_startupSettings = StartupSettings();
    m_webEngineSettings = WebEngineSettings();
    m_recentItemsSettings = RecentItemsSettings();
    m_performanceOptimizationEnabled = true;
    
    autoTuneForSystem();
    saveConfiguration();
    
    emit configurationChanged();
    qDebug() << "PerformanceConfig: Reset to defaults";
}

void PerformanceConfig::setMemorySettings(const MemorySettings& settings)
{
    if (memcmp(&m_memorySettings, &settings, sizeof(MemorySettings)) != 0) {
        m_memorySettings = settings;
        emit memorySettingsChanged(settings);
        emit configurationChanged();
    }
}

void PerformanceConfig::setStartupSettings(const StartupSettings& settings)
{
    if (memcmp(&m_startupSettings, &settings, sizeof(StartupSettings)) != 0) {
        m_startupSettings = settings;
        emit startupSettingsChanged(settings);
        emit configurationChanged();
    }
}

void PerformanceConfig::setWebEngineSettings(const WebEngineSettings& settings)
{
    if (memcmp(&m_webEngineSettings, &settings, sizeof(WebEngineSettings)) != 0) {
        m_webEngineSettings = settings;
        emit webEngineSettingsChanged(settings);
        emit configurationChanged();
    }
}

void PerformanceConfig::setRecentItemsSettings(const RecentItemsSettings& settings)
{
    if (memcmp(&m_recentItemsSettings, &settings, sizeof(RecentItemsSettings)) != 0) {
        m_recentItemsSettings = settings;
        emit recentItemsSettingsChanged(settings);
        emit configurationChanged();
    }
}

bool PerformanceConfig::isPerformanceOptimizationEnabled() const
{
    return m_performanceOptimizationEnabled;
}

void PerformanceConfig::setPerformanceOptimizationEnabled(bool enabled)
{
    if (m_performanceOptimizationEnabled != enabled) {
        m_performanceOptimizationEnabled = enabled;
        emit configurationChanged();
    }
}

void PerformanceConfig::autoTuneForSystem()
{
    if (isLowEndSystem()) {
        adjustForLowMemorySystem();
    } else {
        adjustForHighPerformanceSystem();
    }
    
    qDebug() << "PerformanceConfig: Auto-tuned for system capabilities";
}

void PerformanceConfig::adjustForLowMemorySystem()
{
    // 低内存系统优化
    m_memorySettings.warningThreshold = 256 * 1024 * 1024;  // 256MB
    m_memorySettings.criticalThreshold = 512 * 1024 * 1024; // 512MB
    m_memorySettings.cleanupInterval = 180000; // 3分钟
    m_memorySettings.monitoringInterval = 15000; // 15秒
    
    m_startupSettings.optimizationLevel = 2; // Aggressive
    m_startupSettings.maxPreloadResources = 5;
    m_startupSettings.delayedInitTimeout = 2000; // 2秒
    
    m_webEngineSettings.cacheMaxSize = 50 * 1024 * 1024; // 50MB
    m_webEngineSettings.cacheCleanupInterval = 300000; // 5分钟
    
    m_recentItemsSettings.maxItems = 25;
    m_recentItemsSettings.searchCacheSize = 50;
    
    qDebug() << "PerformanceConfig: Adjusted for low memory system";
}

void PerformanceConfig::adjustForHighPerformanceSystem()
{
    // 高性能系统优化
    m_memorySettings.warningThreshold = 1024 * 1024 * 1024; // 1GB
    m_memorySettings.criticalThreshold = 2048 * 1024 * 1024; // 2GB
    m_memorySettings.cleanupInterval = 600000; // 10分钟
    m_memorySettings.monitoringInterval = 60000; // 1分钟
    
    m_startupSettings.optimizationLevel = 1; // Moderate
    m_startupSettings.maxPreloadResources = 15;
    m_startupSettings.delayedInitTimeout = 500; // 0.5秒
    
    m_webEngineSettings.cacheMaxSize = 200 * 1024 * 1024; // 200MB
    m_webEngineSettings.cacheCleanupInterval = 900000; // 15分钟
    
    m_recentItemsSettings.maxItems = 100;
    m_recentItemsSettings.searchCacheSize = 200;
    
    qDebug() << "PerformanceConfig: Adjusted for high performance system";
}

void PerformanceConfig::detectSystemCapabilities()
{
    m_systemMemorySize = getSystemMemorySize();
    m_cpuCoreCount = getCpuCoreCount();
}

qint64 PerformanceConfig::getSystemMemorySize()
{
#ifdef Q_OS_WIN
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return memInfo.ullTotalPhys;
    }
#endif
    
    // 默认假设8GB
    return 8LL * 1024 * 1024 * 1024;
}

int PerformanceConfig::getCpuCoreCount()
{
    return QThread::idealThreadCount();
}

bool PerformanceConfig::isLowEndSystem()
{
    // 小于4GB内存或少于4个CPU核心认为是低端系统
    return (m_systemMemorySize < 4LL * 1024 * 1024 * 1024) || (m_cpuCoreCount < 4);
}