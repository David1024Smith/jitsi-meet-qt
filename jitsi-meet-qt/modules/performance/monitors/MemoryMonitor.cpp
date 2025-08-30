#include "MemoryMonitor.h"
#include <QDebug>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#endif

#ifdef Q_OS_MACOS
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_host.h>
#endif

MemoryMonitor::MemoryMonitor(QObject *parent)
    : BaseMonitor("MemoryMonitor", parent)
    , m_monitoringMode(SystemMode)
    , m_leakDetectionThreshold(1024.0) // 1KB/s
    , m_fileWatcher(nullptr)
    , m_systemProcess(nullptr)
    , m_leakDetectionTimer(nullptr)
    , m_totalPhysicalMemory(0)
    , m_totalVirtualMemory(0)
    , m_totalSwapMemory(0)
{
    // 初始化历史数据容器
    m_physicalMemoryHistory.reserve(1440); // 24小时的分钟数据
    m_virtualMemoryHistory.reserve(1440);
    m_swapMemoryHistory.reserve(1440);
    
    // 初始化泄漏检测定时器
    m_leakDetectionTimer = new QTimer(this);
    m_leakDetectionTimer->setInterval(60000); // 每分钟检测一次
    connect(m_leakDetectionTimer, &QTimer::timeout,
            this, &MemoryMonitor::performLeakDetection);
}

MemoryMonitor::~MemoryMonitor()
{
    if (isTracking()) {
        stopTracking();
    }
    cleanupPlatformSpecific();
}

QString MemoryMonitor::version() const
{
    return "1.0.0";
}

QString MemoryMonitor::description() const
{
    return "Memory performance monitor for tracking system and process memory usage";
}v
oid MemoryMonitor::setMonitoringMode(MonitoringMode mode)
{
    QMutexLocker locker(&m_dataMutex);
    if (m_monitoringMode != mode) {
        m_monitoringMode = mode;
        
        // 根据模式启用/禁用泄漏检测
        if (mode == LeakDetectionMode) {
            if (!m_leakDetectionTimer->isActive()) {
                m_leakDetectionTimer->start();
            }
        } else {
            if (m_leakDetectionTimer->isActive()) {
                m_leakDetectionTimer->stop();
            }
        }
        
        qDebug() << "MemoryMonitor: Monitoring mode changed to" << mode;
    }
}

MemoryMonitor::MonitoringMode MemoryMonitor::monitoringMode() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_monitoringMode;
}

qint64 MemoryMonitor::getTotalPhysicalMemory() const
{
    return m_totalPhysicalMemory;
}

qint64 MemoryMonitor::getAvailablePhysicalMemory() const
{
    QVariantMap memInfo = readSystemMemoryInfo();
    return memInfo.value("availablePhysical", 0).toLongLong();
}

qint64 MemoryMonitor::getUsedPhysicalMemory() const
{
    return getTotalPhysicalMemory() - getAvailablePhysicalMemory();
}

double MemoryMonitor::getPhysicalMemoryUsage() const
{
    qint64 total = getTotalPhysicalMemory();
    if (total > 0) {
        qint64 used = getUsedPhysicalMemory();
        return (100.0 * used) / total;
    }
    return 0.0;
}

qint64 MemoryMonitor::getTotalVirtualMemory() const
{
    return m_totalVirtualMemory;
}

qint64 MemoryMonitor::getUsedVirtualMemory() const
{
    QVariantMap memInfo = readSystemMemoryInfo();
    return memInfo.value("usedVirtual", 0).toLongLong();
}

double MemoryMonitor::getVirtualMemoryUsage() const
{
    qint64 total = getTotalVirtualMemory();
    if (total > 0) {
        qint64 used = getUsedVirtualMemory();
        return (100.0 * used) / total;
    }
    return 0.0;
}

qint64 MemoryMonitor::getSwapSize() const
{
    return m_totalSwapMemory;
}

qint64 MemoryMonitor::getUsedSwap() const
{
    QVariantMap memInfo = readSystemMemoryInfo();
    return memInfo.value("usedSwap", 0).toLongLong();
}

double MemoryMonitor::getSwapUsage() const
{
    qint64 total = getSwapSize();
    if (total > 0) {
        qint64 used = getUsedSwap();
        return (100.0 * used) / total;
    }
    return 0.0;
}

qint64 MemoryMonitor::getCachedMemory() const
{
    QVariantMap memInfo = readSystemMemoryInfo();
    return memInfo.value("cached", 0).toLongLong();
}

qint64 MemoryMonitor::getBufferMemory() const
{
    QVariantMap memInfo = readSystemMemoryInfo();
    return memInfo.value("buffers", 0).toLongLong();
}

qint64 MemoryMonitor::getProcessMemoryUsage(qint64 processId) const
{
    QVariantMap processInfo = readProcessMemoryInfo(processId);
    return processInfo.value("rss", 0).toLongLong();
}

qint64 MemoryMonitor::getCurrentProcessMemoryUsage() const
{
    return getProcessMemoryUsage(QCoreApplication::applicationPid());
}

qint64 MemoryMonitor::getProcessVirtualMemoryUsage(qint64 processId) const
{
    QVariantMap processInfo = readProcessMemoryInfo(processId);
    return processInfo.value("vms", 0).toLongLong();
}bool Memor
yMonitor::initializeMonitor()
{
    qDebug() << "MemoryMonitor: Initializing memory monitor...";
    
    // 初始化平台特定功能
    if (!initializePlatformSpecific()) {
        addError("Failed to initialize platform-specific memory monitoring");
        return false;
    }
    
    // 获取系统内存信息
    QVariantMap memInfo = readSystemMemoryInfo();
    m_totalPhysicalMemory = memInfo.value("totalPhysical", 0).toLongLong();
    m_totalVirtualMemory = memInfo.value("totalVirtual", 0).toLongLong();
    m_totalSwapMemory = memInfo.value("totalSwap", 0).toLongLong();
    
    qDebug() << "MemoryMonitor: Initialized successfully";
    qDebug() << "  Total Physical Memory:" << m_totalPhysicalMemory / (1024*1024) << "MB";
    
    return true;
}

ResourceUsage MemoryMonitor::collectResourceUsage()
{
    ResourceUsage usage;
    usage.timestamp = QDateTime::currentDateTime();
    usage.resourceType = IResourceTracker::Memory;
    
    // 收集内存使用数据
    qint64 usedMemory = getUsedPhysicalMemory();
    qint64 totalMemory = getTotalPhysicalMemory();
    
    usage.memoryUsage = totalMemory > 0 ? (100.0 * usedMemory / totalMemory) : 0.0;
    usage.memoryUsed = usedMemory;
    usage.memoryTotal = totalMemory;
    
    // 根据监控模式收集不同级别的数据
    if (m_monitoringMode >= DetailedMode) {
        usage.additionalData["virtualUsage"] = getVirtualMemoryUsage();
        usage.additionalData["swapUsage"] = getSwapUsage();
        usage.additionalData["cached"] = getCachedMemory();
        usage.additionalData["buffers"] = getBufferMemory();
    }
    
    if (m_monitoringMode == ProcessMode || m_monitoringMode == LeakDetectionMode) {
        qint64 processMemory = getCurrentProcessMemoryUsage();
        usage.additionalData["processMemory"] = processMemory;
    }
    
    return usage;
}

QList<IResourceTracker::ResourceType> MemoryMonitor::supportedResourceTypes() const
{
    return {IResourceTracker::Memory};
}

void MemoryMonitor::handleMemoryInfoUpdate()
{
    if (isTracking()) {
        QTimer::singleShot(100, this, [this]() {
            collectResourceUsage();
        });
    }
}

void MemoryMonitor::performLeakDetection()
{
    if (m_monitoringMode != LeakDetectionMode) {
        return;
    }
    
    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 currentPid = QCoreApplication::applicationPid();
    qint64 currentMemory = getCurrentProcessMemoryUsage();
    
    QMutexLocker locker(&m_dataMutex);
    
    LeakDetectionData& data = m_leakDetectionData[currentPid];
    
    if (data.lastCheckTime.isValid()) {
        qint64 timeDiff = data.lastCheckTime.msecsTo(currentTime);
        if (timeDiff > 0) {
            qint64 memoryDiff = currentMemory - data.lastMemoryUsage;
            data.leakRate = (1000.0 * memoryDiff) / timeDiff; // 字节/秒
            
            // 检查是否超过阈值
            if (data.leakRate > m_leakDetectionThreshold) {
                qWarning() << "MemoryMonitor: Memory leak detected for process" << currentPid;
                emit thresholdExceeded(IResourceTracker::Memory, data.leakRate, m_leakDetectionThreshold);
            }
        }
    }
    
    data.lastMemoryUsage = currentMemory;
    data.lastCheckTime = currentTime;
}

bool MemoryMonitor::initializePlatformSpecific()
{
#ifdef Q_OS_LINUX
    if (!QFile::exists("/proc/meminfo")) {
        qWarning() << "MemoryMonitor: /proc/meminfo not found";
        return false;
    }
#endif
    return true;
}

void MemoryMonitor::cleanupPlatformSpecific()
{
    // 清理平台特定资源
}

QVariantMap MemoryMonitor::readSystemMemoryInfo()
{
#ifdef Q_OS_WIN
    QVariantMap result;
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    
    if (GlobalMemoryStatusEx(&memStatus)) {
        result["totalPhysical"] = static_cast<qint64>(memStatus.ullTotalPhys);
        result["availablePhysical"] = static_cast<qint64>(memStatus.ullAvailPhys);
        result["totalVirtual"] = static_cast<qint64>(memStatus.ullTotalVirtual);
        result["usedVirtual"] = static_cast<qint64>(memStatus.ullTotalVirtual - memStatus.ullAvailVirtual);
        result["totalSwap"] = static_cast<qint64>(memStatus.ullTotalPageFile);
        result["usedSwap"] = static_cast<qint64>(memStatus.ullTotalPageFile - memStatus.ullAvailPageFile);
    }
    return result;
#elif defined(Q_OS_LINUX)
    QVariantMap result;
    QFile file("/proc/meminfo");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        QString line;
        
        while (stream.readLineInto(&line)) {
            QStringList parts = line.split(':', Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                QString key = parts[0].trimmed();
                QString valueStr = parts[1].trimmed().split(' ')[0];
                qint64 value = valueStr.toLongLong() * 1024; // 转换为字节
                
                if (key == "MemTotal") {
                    result["totalPhysical"] = value;
                } else if (key == "MemAvailable") {
                    result["availablePhysical"] = value;
                } else if (key == "SwapTotal") {
                    result["totalSwap"] = value;
                } else if (key == "SwapFree") {
                    result["usedSwap"] = result.value("totalSwap", 0).toLongLong() - value;
                } else if (key == "Cached") {
                    result["cached"] = value;
                } else if (key == "Buffers") {
                    result["buffers"] = value;
                }
            }
        }
    }
    return result;
#else
    return QVariantMap();
#endif
}

QVariantMap MemoryMonitor::readProcessMemoryInfo(qint64 processId)
{
#ifdef Q_OS_WIN
    QVariantMap result;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess) {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            result["rss"] = static_cast<qint64>(pmc.WorkingSetSize);
            result["vms"] = static_cast<qint64>(pmc.PagefileUsage);
        }
        CloseHandle(hProcess);
    }
    return result;
#elif defined(Q_OS_LINUX)
    QVariantMap result;
    QString statusPath = QString("/proc/%1/status").arg(processId);
    QFile file(statusPath);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        QString line;
        
        while (stream.readLineInto(&line)) {
            QStringList parts = line.split(':', Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                QString key = parts[0].trimmed();
                QString valueStr = parts[1].trimmed().split(' ')[0];
                qint64 value = valueStr.toLongLong() * 1024; // 转换为字节
                
                if (key == "VmRSS") {
                    result["rss"] = value;
                } else if (key == "VmSize") {
                    result["vms"] = value;
                }
            }
        }
    }
    return result;
#else
    Q_UNUSED(processId)
    return QVariantMap();
#endif
}