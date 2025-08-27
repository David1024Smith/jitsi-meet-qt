#ifndef PERFORMANCECONFIG_H
#define PERFORMANCECONFIG_H

#include <QObject>
#include <QSettings>

/**
 * @brief 性能配置管理器 - 管理性能优化相关的配置选项
 */
class PerformanceConfig : public QObject
{
    Q_OBJECT

public:
    struct MemorySettings {
        qint64 warningThreshold = 512 * 1024 * 1024;  // 512MB
        qint64 criticalThreshold = 1024 * 1024 * 1024; // 1GB
        int cleanupInterval = 300000;  // 5分钟
        int monitoringInterval = 30000; // 30秒
        bool autoCleanupEnabled = true;
        bool leakDetectionEnabled = true;
    };

    struct StartupSettings {
        bool fastStartupEnabled = true;
        bool resourcePreloadEnabled = true;
        bool deferredInitEnabled = true;
        int optimizationLevel = 1; // 0=Basic, 1=Moderate, 2=Aggressive
        int maxPreloadResources = 10;
        int delayedInitTimeout = 1000; // 1秒
    };

    struct WebEngineSettings {
        qint64 cacheMaxSize = 100 * 1024 * 1024; // 100MB
        bool diskCacheEnabled = true;
        bool memoryOptimizationEnabled = true;
        int cacheCleanupInterval = 600000; // 10分钟
        bool javascriptOptimizationEnabled = true;
    };

    struct RecentItemsSettings {
        int maxItems = 50;
        bool lazyLoadingEnabled = true;
        bool asyncSaveEnabled = true;
        int optimizationInterval = 300000; // 5分钟
        int searchCacheSize = 100;
        int maxAge = 30; // 30天
    };

    explicit PerformanceConfig(QObject *parent = nullptr);
    ~PerformanceConfig();

    // 配置加载和保存
    void loadConfiguration();
    void saveConfiguration();
    void resetToDefaults();

    // 获取配置
    const MemorySettings& memorySettings() const { return m_memorySettings; }
    const StartupSettings& startupSettings() const { return m_startupSettings; }
    const WebEngineSettings& webEngineSettings() const { return m_webEngineSettings; }
    const RecentItemsSettings& recentItemsSettings() const { return m_recentItemsSettings; }

    // 设置配置
    void setMemorySettings(const MemorySettings& settings);
    void setStartupSettings(const StartupSettings& settings);
    void setWebEngineSettings(const WebEngineSettings& settings);
    void setRecentItemsSettings(const RecentItemsSettings& settings);

    // 便利方法
    bool isPerformanceOptimizationEnabled() const;
    void setPerformanceOptimizationEnabled(bool enabled);

    // 自动调整配置
    void autoTuneForSystem();
    void adjustForLowMemorySystem();
    void adjustForHighPerformanceSystem();

signals:
    void configurationChanged();
    void memorySettingsChanged(const MemorySettings& settings);
    void startupSettingsChanged(const StartupSettings& settings);
    void webEngineSettingsChanged(const WebEngineSettings& settings);
    void recentItemsSettingsChanged(const RecentItemsSettings& settings);

private:
    void detectSystemCapabilities();
    qint64 getSystemMemorySize();
    int getCpuCoreCount();
    bool isLowEndSystem();

    QSettings* m_settings;
    MemorySettings m_memorySettings;
    StartupSettings m_startupSettings;
    WebEngineSettings m_webEngineSettings;
    RecentItemsSettings m_recentItemsSettings;
    
    bool m_performanceOptimizationEnabled;
    qint64 m_systemMemorySize;
    int m_cpuCoreCount;
};

#endif // PERFORMANCECONFIG_H