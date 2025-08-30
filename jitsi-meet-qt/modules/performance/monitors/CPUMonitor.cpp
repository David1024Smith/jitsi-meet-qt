#include "CPUMonitor.h"
#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#include <pdh.h>
#include <psapi.h>
#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "psapi.lib")
#endif

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#include <sys/times.h>
#include <fstream>
#include <sstream>
#endif

#ifdef Q_OS_MACOS
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>
#endif

CPUMonitor::CPUMonitor(QObject *parent)
    : BaseMonitor("CPUMonitor", parent)
    , m_monitoringMode(BasicMode)
    , m_coreCount(QThread::idealThreadCount())
    , m_baseClock(0.0)
    , m_overheatThreshold(85.0)
    , m_fileWatcher(nullptr)
    , m_systemProcess(nullptr)
    , m_lastCPUUsage(0.0)
    , m_lastTemperature(0.0)
    , m_lastFrequency(0.0)
{
#ifdef Q_OS_WIN
    m_pdhQuery = nullptr;
    m_cpuCounter = nullptr;
#endif

#ifdef Q_OS_LINUX
    m_lastStatTime = QDateTime::currentDateTime();
#endif

    // 初始化历史数据容器
    m_cpuUsageHistory.reserve(1440); // 24小时的分钟数据
    m_coreUsageHistory.reserve(1440);
    m_temperatureHistory.reserve(1440);
    m_frequencyHistory.reserve(1440);
}

CPUMonitor::~CPUMonitor()
{
    if (isTracking()) {
        stopTracking();
    }
    cleanupPlatformSpecific();
}

QString CPUMonitor::version() const
{
    return "1.0.0";
}

QString CPUMonitor::description() const
{
    return "CPU performance monitor for tracking CPU usage, temperature, and frequency";
}

void CPUMonitor::setMonitoringMode(MonitoringMode mode)
{
    QMutexLocker locker(&m_dataMutex);
    if (m_monitoringMode != mode) {
        m_monitoringMode = mode;
        qDebug() << "CPUMonitor: Monitoring mode changed to" << mode;
    }
}

CPUMonitor::MonitoringMode CPUMonitor::monitoringMode() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_monitoringMode;
}

int CPUMonitor::getCoreCount() const
{
    return m_coreCount;
}

QString CPUMonitor::getCPUArchitecture() const
{
    return m_cpuArchitecture;
}

QString CPUMonitor::getCPUModel() const
{
    return m_cpuModel;
}

double CPUMonitor::getBaseClock() const
{
    return m_baseClock;
}

double CPUMonitor::getCurrentClock() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_lastFrequency;
}

double CPUMonitor::getCPUTemperature() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_lastTemperature;
}

QList<double> CPUMonitor::getCoreUsages() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_lastCoreUsages;
}

double CPUMonitor::getProcessCPUUsage(qint64 processId) const
{
    return readProcessCPUUsage(processId);
}

double CPUMonitor::getCurrentProcessCPUUsage() const
{
    return getProcessCPUUsage(QCoreApplication::applicationPid());
}

QList<double> CPUMonitor::getLoadAverages() const
{
    return readLoadAverages();
}

QList<double> CPUMonitor::getCPUUsageHistory(int minutes) const
{
    QMutexLocker locker(&m_dataMutex);
    int count = qMin(minutes, m_cpuUsageHistory.size());
    return m_cpuUsageHistory.mid(m_cpuUsageHistory.size() - count);
}

bool CPUMonitor::isCPUOverheating() const
{
    return getCPUTemperature() > m_overheatThreshold;
}

void CPUMonitor::setOverheatThreshold(double threshold)
{
    m_overheatThreshold = threshold;
}

double CPUMonitor::overheatThreshold() const
{
    return m_overheatThreshold;
}

bool CPUMonitor::initializeMonitor()
{
    qDebug() << "CPUMonitor: Initializing CPU monitor...";
    
    // 获取CPU信息
#ifdef Q_OS_WIN
    // Windows CPU信息获取
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    m_coreCount = sysInfo.dwNumberOfProcessors;
    
    // 获取CPU型号
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t buffer[256];
        DWORD bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"ProcessorNameString", nullptr, nullptr, 
                           (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            m_cpuModel = QString::fromWCharArray(buffer);
        }
        RegCloseKey(hKey);
    }
    
    m_cpuArchitecture = (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ? "x64" : "x86";
#endif

#ifdef Q_OS_LINUX
    // Linux CPU信息获取
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                m_cpuModel = QString::fromStdString(line.substr(pos + 2));
                break;
            }
        }
    }
    
    // 获取架构信息
    struct utsname unameData;
    if (uname(&unameData) == 0) {
        m_cpuArchitecture = QString::fromLocal8Bit(unameData.machine);
    }
#endif

#ifdef Q_OS_MACOS
    // macOS CPU信息获取
    size_t size = sizeof(m_coreCount);
    sysctlbyname("hw.ncpu", &m_coreCount, &size, nullptr, 0);
    
    char buffer[256];
    size = sizeof(buffer);
    if (sysctlbyname("machdep.cpu.brand_string", buffer, &size, nullptr, 0) == 0) {
        m_cpuModel = QString::fromLocal8Bit(buffer);
    }
    
    size = sizeof(buffer);
    if (sysctlbyname("hw.machine", buffer, &size, nullptr, 0) == 0) {
        m_cpuArchitecture = QString::fromLocal8Bit(buffer);
    }
#endif

    // 初始化平台特定功能
    if (!initializePlatformSpecific()) {
        addError("Failed to initialize platform-specific CPU monitoring");
        return false;
    }

    // 初始化文件监控器(Linux)
#ifdef Q_OS_LINUX
    m_fileWatcher = new QFileSystemWatcher(this);
    m_fileWatcher->addPath("/proc/stat");
    m_fileWatcher->addPath("/proc/loadavg");
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &CPUMonitor::handleSystemInfoUpdate);
#endif

    qDebug() << "CPUMonitor: Initialized successfully";
    qDebug() << "  CPU Model:" << m_cpuModel;
    qDebug() << "  Architecture:" << m_cpuArchitecture;
    qDebug() << "  Core Count:" << m_coreCount;
    
    return true;
}

ResourceUsage CPUMonitor::collectResourceUsage()
{
    ResourceUsage usage;
    usage.timestamp = QDateTime::currentDateTime();
    usage.resourceType = IResourceTracker::CPU;
    
    // 收集CPU使用率
    double cpuUsage = readCPUUsage();
    usage.cpuUsage = cpuUsage;
    
    // 根据监控模式收集不同级别的数据
    if (m_monitoringMode >= DetailedMode) {
        QList<double> coreUsages = readCoreUsages();
        QVariantList coreUsageList;
        for (double coreUsage : coreUsages) {
            coreUsageList.append(coreUsage);
        }
        usage.additionalData["coreUsages"] = coreUsageList;
        
        // 更新缓存
        QMutexLocker locker(&m_dataMutex);
        m_lastCoreUsages = coreUsages;
    }
    
    if (m_monitoringMode >= AdvancedMode) {
        double frequency = readCPUFrequency();
        double temperature = readCPUTemperature();
        
        usage.additionalData["frequency"] = frequency;
        usage.additionalData["temperature"] = temperature;
        
        // 更新缓存
        QMutexLocker locker(&m_dataMutex);
        m_lastFrequency = frequency;
        m_lastTemperature = temperature;
    }
    
    if (m_monitoringMode == ProcessMode || m_monitoringMode == AdvancedMode) {
        double processUsage = getCurrentProcessCPUUsage();
        usage.additionalData["processUsage"] = processUsage;
        
        QList<double> loadAverages = readLoadAverages();
        QVariantList loadList;
        for (double load : loadAverages) {
            loadList.append(load);
        }
        usage.additionalData["loadAverages"] = loadList;
    }
    
    // 更新历史数据
    QMutexLocker locker(&m_dataMutex);
    m_lastCPUUsage = cpuUsage;
    m_lastUpdateTime = usage.timestamp;
    
    // 添加到历史记录
    m_cpuUsageHistory.append(cpuUsage);
    if (m_cpuUsageHistory.size() > 1440) { // 保持24小时数据
        m_cpuUsageHistory.removeFirst();
    }
    
    if (m_monitoringMode >= AdvancedMode) {
        m_temperatureHistory.append(m_lastTemperature);
        m_frequencyHistory.append(m_lastFrequency);
        
        if (m_temperatureHistory.size() > 1440) {
            m_temperatureHistory.removeFirst();
        }
        if (m_frequencyHistory.size() > 1440) {
            m_frequencyHistory.removeFirst();
        }
    }
    
    return usage;
}

QList<IResourceTracker::ResourceType> CPUMonitor::supportedResourceTypes() const
{
    return {IResourceTracker::CPU};
}

void CPUMonitor::handleSystemInfoUpdate()
{
    // 文件系统变化时触发数据更新
    if (isTracking()) {
        // 触发一次数据收集
        QTimer::singleShot(100, this, [this]() {
            collectResourceUsage();
        });
    }
}

bool CPUMonitor::initializePlatformSpecific()
{
#ifdef Q_OS_WIN
    // 初始化PDH查询
    PDH_STATUS status = PdhOpenQuery(nullptr, 0, &m_pdhQuery);
    if (status != ERROR_SUCCESS) {
        qWarning() << "CPUMonitor: Failed to open PDH query:" << status;
        return false;
    }
    
    // 添加CPU计数器
    status = PdhAddCounter(m_pdhQuery, L"\\Processor(_Total)\\% Processor Time", 0, &m_cpuCounter);
    if (status != ERROR_SUCCESS) {
        qWarning() << "CPUMonitor: Failed to add CPU counter:" << status;
        PdhCloseQuery(m_pdhQuery);
        m_pdhQuery = nullptr;
        return false;
    }
    
    // 添加各核心计数器
    if (m_monitoringMode >= DetailedMode) {
        m_coreCounters.clear();
        for (int i = 0; i < m_coreCount; ++i) {
            PDH_HCOUNTER coreCounter;
            QString counterPath = QString("\\Processor(%1)\\% Processor Time").arg(i);
            status = PdhAddCounter(m_pdhQuery, reinterpret_cast<const wchar_t*>(counterPath.utf16()), 0, &coreCounter);
            if (status == ERROR_SUCCESS) {
                m_coreCounters.append(coreCounter);
            }
        }
    }
    
    // 执行一次收集以初始化计数器
    PdhCollectQueryData(m_pdhQuery);
    
    return true;
#endif

#ifdef Q_OS_LINUX
    // Linux平台初始化
    // 检查必要的文件是否存在
    if (!QFile::exists("/proc/stat") || !QFile::exists("/proc/loadavg")) {
        qWarning() << "CPUMonitor: Required proc files not found";
        return false;
    }
    
    // 读取初始状态
    m_lastProcStat = parseProcStat();
    m_lastStatTime = QDateTime::currentDateTime();
    
    return true;
#endif

#ifdef Q_OS_MACOS
    // macOS平台初始化
    return true;
#endif

    return true;
}

void CPUMonitor::cleanupPlatformSpecific()
{
#ifdef Q_OS_WIN
    if (m_pdhQuery) {
        PdhCloseQuery(m_pdhQuery);
        m_pdhQuery = nullptr;
    }
    m_coreCounters.clear();
#endif
}

double CPUMonitor::readCPUUsage()
{
#ifdef Q_OS_WIN
    return readCPUUsageWindows();
#elif defined(Q_OS_LINUX)
    return readCPUUsageLinux();
#elif defined(Q_OS_MACOS)
    return readCPUUsageMacOS();
#else
    return 0.0;
#endif
}

QList<double> CPUMonitor::readCoreUsages()
{
    QList<double> coreUsages;
    
#ifdef Q_OS_WIN
    if (m_pdhQuery && !m_coreCounters.isEmpty()) {
        PdhCollectQueryData(m_pdhQuery);
        
        for (PDH_HCOUNTER counter : m_coreCounters) {
            PDH_FMT_COUNTERVALUE counterValue;
            PDH_STATUS status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &counterValue);
            if (status == ERROR_SUCCESS) {
                coreUsages.append(counterValue.doubleValue);
            } else {
                coreUsages.append(0.0);
            }
        }
    }
#endif

#ifdef Q_OS_LINUX
    // Linux下读取各核心使用率需要解析/proc/stat
    QFile file("/proc/stat");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        QString line;
        int coreIndex = 0;
        
        while (stream.readLineInto(&line) && coreIndex < m_coreCount) {
            if (line.startsWith(QString("cpu%1").arg(coreIndex))) {
                QStringList parts = line.split(' ', Qt::SkipEmptyParts);
                if (parts.size() >= 8) {
                    qint64 user = parts[1].toLongLong();
                    qint64 nice = parts[2].toLongLong();
                    qint64 system = parts[3].toLongLong();
                    qint64 idle = parts[4].toLongLong();
                    qint64 iowait = parts[5].toLongLong();
                    qint64 irq = parts[6].toLongLong();
                    qint64 softirq = parts[7].toLongLong();
                    
                    qint64 totalTime = user + nice + system + idle + iowait + irq + softirq;
                    qint64 idleTime = idle + iowait;
                    
                    double usage = totalTime > 0 ? (100.0 * (totalTime - idleTime) / totalTime) : 0.0;
                    coreUsages.append(usage);
                }
                coreIndex++;
            }
        }
    }
#endif

    // 如果没有获取到核心数据，填充默认值
    while (coreUsages.size() < m_coreCount) {
        coreUsages.append(0.0);
    }
    
    return coreUsages;
}

double CPUMonitor::readCPUFrequency()
{
#ifdef Q_OS_WIN
    // Windows下读取CPU频率
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD frequency = 0;
        DWORD dataSize = sizeof(frequency);
        if (RegQueryValueEx(hKey, L"~MHz", nullptr, nullptr, 
                           (LPBYTE)&frequency, &dataSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return frequency / 1000.0; // 转换为GHz
        }
        RegCloseKey(hKey);
    }
#endif

#ifdef Q_OS_LINUX
    // Linux下读取CPU频率
    QFile file("/proc/cpuinfo");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        QString line;
        while (stream.readLineInto(&line)) {
            if (line.contains("cpu MHz")) {
                QStringList parts = line.split(':');
                if (parts.size() >= 2) {
                    return parts[1].trimmed().toDouble() / 1000.0; // 转换为GHz
                }
            }
        }
    }
#endif

    return 0.0;
}

double CPUMonitor::readCPUTemperature()
{
#ifdef Q_OS_WIN
    return readCPUTemperatureWindows();
#elif defined(Q_OS_LINUX)
    return readCPUTemperatureLinux();
#elif defined(Q_OS_MACOS)
    return readCPUTemperatureMacOS();
#else
    return 0.0;
#endif
}

QList<double> CPUMonitor::readLoadAverages()
{
#ifdef Q_OS_LINUX
    return parseProcLoadavg();
#else
    // 其他平台返回空列表或模拟数据
    return {0.0, 0.0, 0.0};
#endif
}

double CPUMonitor::readProcessCPUUsage(qint64 processId) const
{
#ifdef Q_OS_WIN
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess) {
        FILETIME creationTime, exitTime, kernelTime, userTime;
        if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
            ULARGE_INTEGER kernel, user;
            kernel.LowPart = kernelTime.dwLowDateTime;
            kernel.HighPart = kernelTime.dwHighDateTime;
            user.LowPart = userTime.dwLowDateTime;
            user.HighPart = userTime.dwHighDateTime;
            
            qint64 totalTime = kernel.QuadPart + user.QuadPart;
            CloseHandle(hProcess);
            
            // 这里需要计算相对于系统时间的百分比
            // 简化实现，返回一个估算值
            return (totalTime / 10000000.0) / 100.0; // 转换并估算
        }
        CloseHandle(hProcess);
    }
#endif

#ifdef Q_OS_LINUX
    QString statPath = QString("/proc/%1/stat").arg(processId);
    QFile file(statPath);
    if (file.open(QIODevice::ReadOnly)) {
        QString content = file.readAll();
        QStringList parts = content.split(' ');
        if (parts.size() >= 17) {
            qint64 utime = parts[13].toLongLong();
            qint64 stime = parts[14].toLongLong();
            qint64 totalTime = utime + stime;
            
            // 简化计算，实际需要考虑时间差
            return (totalTime * 100.0) / sysconf(_SC_CLK_TCK);
        }
    }
#endif

    return 0.0;
}

#ifdef Q_OS_WIN
double CPUMonitor::readCPUUsageWindows()
{
    if (!m_pdhQuery || !m_cpuCounter) {
        return 0.0;
    }
    
    PDH_STATUS status = PdhCollectQueryData(m_pdhQuery);
    if (status != ERROR_SUCCESS) {
        return 0.0;
    }
    
    PDH_FMT_COUNTERVALUE counterValue;
    status = PdhGetFormattedCounterValue(m_cpuCounter, PDH_FMT_DOUBLE, nullptr, &counterValue);
    if (status == ERROR_SUCCESS) {
        return counterValue.doubleValue;
    }
    
    return 0.0;
}

double CPUMonitor::readCPUTemperatureWindows()
{
    // Windows下读取CPU温度比较复杂，需要WMI或特殊驱动
    // 这里返回一个模拟值
    return 45.0 + (qrand() % 20); // 45-65度的模拟温度
}
#endif

#ifdef Q_OS_LINUX
double CPUMonitor::readCPUUsageLinux()
{
    QVariantMap currentStat = parseProcStat();
    if (currentStat.isEmpty() || m_lastProcStat.isEmpty()) {
        return 0.0;
    }
    
    qint64 currentTotal = currentStat["total"].toLongLong();
    qint64 currentIdle = currentStat["idle"].toLongLong();
    qint64 lastTotal = m_lastProcStat["total"].toLongLong();
    qint64 lastIdle = m_lastProcStat["idle"].toLongLong();
    
    qint64 totalDiff = currentTotal - lastTotal;
    qint64 idleDiff = currentIdle - lastIdle;
    
    double usage = 0.0;
    if (totalDiff > 0) {
        usage = 100.0 * (totalDiff - idleDiff) / totalDiff;
    }
    
    // 更新上次数据
    const_cast<CPUMonitor*>(this)->m_lastProcStat = currentStat;
    
    return qMax(0.0, qMin(100.0, usage));
}

double CPUMonitor::readCPUTemperatureLinux()
{
    // 尝试从thermal_zone读取温度
    QDir thermalDir("/sys/class/thermal");
    QStringList thermalZones = thermalDir.entryList(QStringList("thermal_zone*"), QDir::Dirs);
    
    for (const QString& zone : thermalZones) {
        QString tempPath = QString("/sys/class/thermal/%1/temp").arg(zone);
        QFile tempFile(tempPath);
        if (tempFile.open(QIODevice::ReadOnly)) {
            QString tempStr = tempFile.readAll().trimmed();
            bool ok;
            int temp = tempStr.toInt(&ok);
            if (ok && temp > 0) {
                return temp / 1000.0; // 转换为摄氏度
            }
        }
    }
    
    // 如果无法读取，返回模拟值
    return 50.0;
}

QVariantMap CPUMonitor::parseProcStat()
{
    QVariantMap result;
    
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly)) {
        return result;
    }
    
    QTextStream stream(&file);
    QString line = stream.readLine();
    
    if (line.startsWith("cpu ")) {
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() >= 8) {
            qint64 user = parts[1].toLongLong();
            qint64 nice = parts[2].toLongLong();
            qint64 system = parts[3].toLongLong();
            qint64 idle = parts[4].toLongLong();
            qint64 iowait = parts[5].toLongLong();
            qint64 irq = parts[6].toLongLong();
            qint64 softirq = parts[7].toLongLong();
            
            qint64 total = user + nice + system + idle + iowait + irq + softirq;
            qint64 idleTotal = idle + iowait;
            
            result["user"] = user;
            result["nice"] = nice;
            result["system"] = system;
            result["idle"] = idle;
            result["iowait"] = iowait;
            result["irq"] = irq;
            result["softirq"] = softirq;
            result["total"] = total;
            result["idle_total"] = idleTotal;
        }
    }
    
    return result;
}

QList<double> CPUMonitor::parseProcLoadavg()
{
    QList<double> loadAverages;
    
    QFile file("/proc/loadavg");
    if (file.open(QIODevice::ReadOnly)) {
        QString content = file.readAll().trimmed();
        QStringList parts = content.split(' ');
        if (parts.size() >= 3) {
            loadAverages.append(parts[0].toDouble()); // 1分钟
            loadAverages.append(parts[1].toDouble()); // 5分钟
            loadAverages.append(parts[2].toDouble()); // 15分钟
        }
    }
    
    return loadAverages;
}
#endif

#ifdef Q_OS_MACOS
double CPUMonitor::readCPUUsageMacOS()
{
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, 
                       (host_info_t)&cpuinfo, &count) == KERN_SUCCESS) {
        
        unsigned int total_ticks = 0;
        for (int i = 0; i < CPU_STATE_MAX; i++) {
            total_ticks += cpuinfo.cpu_ticks[i];
        }
        
        if (total_ticks > 0) {
            double usage = 100.0 * (total_ticks - cpuinfo.cpu_ticks[CPU_STATE_IDLE]) / total_ticks;
            return usage;
        }
    }
    
    return 0.0;
}

double CPUMonitor::readCPUTemperatureMacOS()
{
    // macOS下读取CPU温度需要特殊权限和方法
    // 这里返回模拟值
    return 55.0;
}
#endif