#include "PerformanceManager.h"
#include <QApplication>
#include <QProcess>
#include <QDebug>
#include <QThread>
#include <algorithm>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

PerformanceManager* PerformanceManager::s_instance = nullptr;

PerformanceManager::PerformanceManager(QObject *parent)
    : QObject(parent)
    , m_memoryMonitorTimer(new QTimer(this))
    , m_metricsTimer(new QTimer(this))
    , m_peakMemoryUsage(0)
    , m_memoryWarningThreshold(512 * 1024 * 1024) // 512MB
    , m_lazyLoadingEnabled(true)
{
    s_instance = this;
    
    // 设置内存监控定时器
    m_memoryMonitorTimer->setInterval(5000); // 每5秒检查一次
    connect(m_memoryMonitorTimer, &QTimer::timeout, this, &PerformanceManager::updateMemoryMetrics);
    
    // 设置性能指标更新定时器
    m_metricsTimer->setInterval(1000); // 每秒更新一次
    connect(m_metricsTimer, &QTimer::timeout, this, &PerformanceManager::updatePerformanceMetrics);
    
    initializeOptimizations();
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
    qDebug() << "Performance: Startup timer started";
}

void PerformanceManager::endStartupTimer()
{
    if (m_startupTimer.isValid()) {
        auto elapsed = std::chrono::milliseconds(m_startupTimer.elapsed());
        
        QMutexLocker locker(&m_metricsMutex);
        m_currentMetrics.startupTime = elapsed;
        m_currentMetrics.timestamp = std::chrono::steady_clock::now();
        
        qDebug() << "Performance: Startup completed in" << elapsed.count() << "ms";
        
        // 记录启动时间指标
        recordMetric(MetricType::StartupTime, elapsed.count());
    }
}

std::chrono::milliseconds PerformanceManager::getStartupTime() const
{
    QMutexLocker locker(&m_metricsMutex);
    return m_currentMetrics.startupTime;
}

void PerformanceManager::startMemoryMonitoring()
{
    if (!m_memoryMonitorTimer->isActive()) {
        m_memoryMonitorTimer->start();
        m_metricsTimer->start();
        qDebug() << "Performance: Memory monitoring started";
    }
}

void PerformanceManager::stopMemoryMonitoring()
{
    m_memoryMonitorTimer->stop();
    m_metricsTimer->stop();
    qDebug() << "Performance: Memory monitoring stopped";
}

size_t PerformanceManager::getCurrentMemoryUsage() const
{
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
#endif
    return 0;
}

size_t PerformanceManager::getPeakMemoryUsage() const
{
    return m_peakMemoryUsage;
}

void PerformanceManager::recordMetric(MetricType type, double value)
{
    QMutexLocker locker(&m_metricsMutex);
    
    // 记录到历史数据
    m_metricHistory[type].push_back(value);
    
    // 保持历史数据在合理范围内
    if (m_metricHistory[type].size() > 100) {
        m_metricHistory[type].erase(m_metricHistory[type].begin());
    }
    
    // 更新当前指标
    switch (type) {
        case MetricType::StartupTime:
            m_currentMetrics.startupTime = std::chrono::milliseconds(static_cast<int64_t>(value));
            break;
        case MetricType::MemoryUsage:
            m_currentMetrics.memoryUsageMB = static_cast<size_t>(value);
            break;
        case MetricType::NetworkLatency:
            m_currentMetrics.networkLatency = std::chrono::milliseconds(static_cast<int64_t>(value));
            break;
        case MetricType::VideoFrameRate:
            m_currentMetrics.videoFrameRate = value;
            break;
        case MetricType::AudioLatency:
            m_currentMetrics.audioLatency = std::chrono::milliseconds(static_cast<int64_t>(value));
            break;
        case MetricType::CPUUsage:
            m_currentMetrics.cpuUsagePercent = value;
            break;
    }
    
    m_currentMetrics.timestamp = std::chrono::steady_clock::now();
    
    // 检查性能警告阈值
    checkPerformanceThresholds(type, value);
}

PerformanceManager::PerformanceMetrics PerformanceManager::getCurrentMetrics() const
{
    QMutexLocker locker(&m_metricsMutex);
    return m_currentMetrics;
}

void PerformanceManager::preloadResources()
{
    qDebug() << "Performance: Preloading critical resources";
    
    // 在后台线程中预加载资源
    QThread::create([this]() {
        // 预加载样式表
        if (auto app = qobject_cast<QApplication*>(QApplication::instance())) {
            app->setStyleSheet("");
        }
        
        // 预加载字体
        // 这里可以添加字体预加载逻辑
        
        // 预加载图标资源
        // 这里可以添加图标预加载逻辑
        
        qDebug() << "Performance: Resource preloading completed";
    })->start();
}

void PerformanceManager::enableLazyLoading(bool enabled)
{
    m_lazyLoadingEnabled = enabled;
    qDebug() << "Performance: Lazy loading" << (enabled ? "enabled" : "disabled");
}

void PerformanceManager::optimizeForLargeConference(int participantCount)
{
    qDebug() << "Performance: Optimizing for large conference with" << participantCount << "participants";
    
    if (participantCount > 20) {
        // 大型会议优化
        setVideoQualityMode("low");
        
        // 减少更新频率
        m_metricsTimer->setInterval(2000); // 2秒更新一次
        
        // 启用更激进的内存管理
        m_memoryWarningThreshold = 256 * 1024 * 1024; // 256MB
        
    } else if (participantCount > 10) {
        // 中型会议优化
        setVideoQualityMode("medium");
        m_metricsTimer->setInterval(1500);
        
    } else {
        // 小型会议，使用默认设置
        setVideoQualityMode("high");
        m_metricsTimer->setInterval(1000);
        m_memoryWarningThreshold = 512 * 1024 * 1024; // 512MB
    }
}

void PerformanceManager::setVideoQualityMode(const QString& mode)
{
    qDebug() << "Performance: Setting video quality mode to" << mode;
    
    // 这里可以与MediaManager集成，调整视频质量
    // 例如：调整分辨率、帧率、比特率等
}

void PerformanceManager::updateMemoryMetrics()
{
    size_t currentUsage = getCurrentMemoryUsage();
    
    if (currentUsage > m_peakMemoryUsage) {
        m_peakMemoryUsage = currentUsage;
    }
    
    // 转换为MB
    size_t usageMB = currentUsage / (1024 * 1024);
    recordMetric(MetricType::MemoryUsage, static_cast<double>(usageMB));
    
    // 检查内存警告
    if (currentUsage > m_memoryWarningThreshold) {
        emit memoryWarning(currentUsage, m_memoryWarningThreshold);
        qWarning() << "Performance: Memory usage warning -" << usageMB << "MB";
    }
}

void PerformanceManager::updatePerformanceMetrics()
{
    // 更新CPU使用率
#ifdef Q_OS_WIN
    // 简化的CPU使用率计算
    static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
    static int numProcessors = 0;
    static bool first = true;
    
    if (first) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        numProcessors = sysInfo.dwNumberOfProcessors;
        
        FILETIME ftime, fsys, fuser;
        GetSystemTimeAsFileTime(&ftime);
        memcpy(&lastCPU, &ftime, sizeof(FILETIME));
        
        GetProcessTimes(GetCurrentProcess(), &ftime, &ftime, &fsys, &fuser);
        memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
        memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
        first = false;
        return;
    }
    
    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now, sys, user;
    
    GetSystemTimeAsFileTime(&ftime);
    memcpy(&now, &ftime, sizeof(FILETIME));
    
    GetProcessTimes(GetCurrentProcess(), &ftime, &ftime, &fsys, &fuser);
    memcpy(&sys, &fsys, sizeof(FILETIME));
    memcpy(&user, &fuser, sizeof(FILETIME));
    
    double percent = (sys.QuadPart - lastSysCPU.QuadPart) + (user.QuadPart - lastUserCPU.QuadPart);
    percent /= (now.QuadPart - lastCPU.QuadPart);
    percent /= numProcessors;
    percent *= 100;
    
    recordMetric(MetricType::CPUUsage, percent);
    
    lastCPU = now;
    lastUserCPU = user;
    lastSysCPU = sys;
#endif
    
    emit metricsUpdated(getCurrentMetrics());
}

void PerformanceManager::initializeOptimizations()
{
    qDebug() << "Performance: Initializing optimizations";
    
    // High DPI support is enabled by default in Qt 6
    
    setupMemoryThresholds();
}

void PerformanceManager::setupMemoryThresholds()
{
    // 根据系统内存设置阈值
#ifdef Q_OS_WIN
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
        
        // 设置警告阈值为系统内存的10%
        m_memoryWarningThreshold = static_cast<size_t>(totalPhysMem * 0.1);
        
        qDebug() << "Performance: Memory warning threshold set to" 
                 << (m_memoryWarningThreshold / (1024 * 1024)) << "MB";
    }
#endif
}

void PerformanceManager::checkPerformanceThresholds(MetricType type, double value)
{
    bool shouldWarn = false;
    
    switch (type) {
        case MetricType::StartupTime:
            shouldWarn = value > 5000; // 5秒
            break;
        case MetricType::NetworkLatency:
            shouldWarn = value > 500; // 500ms
            break;
        case MetricType::VideoFrameRate:
            shouldWarn = value < 15; // 低于15fps
            break;
        case MetricType::AudioLatency:
            shouldWarn = value > 150; // 150ms
            break;
        case MetricType::CPUUsage:
            shouldWarn = value > 80; // 80%
            break;
        default:
            break;
    }
    
    if (shouldWarn) {
        emit performanceWarning(type, value);
    }
}