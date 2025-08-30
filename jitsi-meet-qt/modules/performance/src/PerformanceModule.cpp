#include "PerformanceModule.h"
#include "PerformanceManager.h"
#include "MetricsCollector.h"
#include "PerformanceConfig.h"
#include <QDebug>
#include <QCoreApplication>

#define PERFORMANCE_MODULE_VERSION "1.0.0"

// 静态成员初始化
PerformanceModule* PerformanceModule::s_instance = nullptr;
QMutex PerformanceModule::s_instanceMutex;

PerformanceModule::PerformanceModule(QObject *parent)
    : QObject(parent)
    , m_status(NotInitialized)
    , m_performanceManager(nullptr)
    , m_metricsCollector(nullptr)
    , m_config(nullptr)
    , m_statusTimer(new QTimer(this))
{
    // 连接状态更新定时器
    connect(m_statusTimer, &QTimer::timeout, this, &PerformanceModule::handleStatusUpdate);
    m_statusTimer->setInterval(1000); // 每秒更新一次状态
}

PerformanceModule::~PerformanceModule()
{
    shutdown();
    cleanup();
}

bool PerformanceModule::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != NotInitialized) {
        qWarning() << "PerformanceModule: Already initialized";
        return m_status == Ready;
    }
    
    setStatus(Initializing);
    
    try {
        // 验证依赖关系
        if (!validateDependencies()) {
            emit errorOccurred("Failed to validate dependencies");
            setStatus(Error);
            return false;
        }
        
        // 初始化配置
        m_config = new PerformanceConfig(this);
        if (!m_config->loadConfig()) {
            qWarning() << "PerformanceModule: Failed to load config, using defaults";
        }
        
        // 初始化组件
        if (!initializeComponents()) {
            emit errorOccurred("Failed to initialize components");
            setStatus(Error);
            return false;
        }
        
        setStatus(Ready);
        emit initialized(true);
        
        qDebug() << "PerformanceModule: Successfully initialized";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceModule: Exception during initialization:" << e.what();
        emit errorOccurred(QString("Initialization exception: %1").arg(e.what()));
        setStatus(Error);
        return false;
    }
}

bool PerformanceModule::start()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != Ready) {
        qWarning() << "PerformanceModule: Cannot start, not ready";
        return false;
    }
    
    try {
        // 启动指标收集器
        if (m_metricsCollector && !m_metricsCollector->start()) {
            emit errorOccurred("Failed to start metrics collector");
            return false;
        }
        
        // 启动性能管理器
        if (m_performanceManager && !m_performanceManager->start()) {
            emit errorOccurred("Failed to start performance manager");
            return false;
        }
        
        // 启动状态更新定时器
        m_statusTimer->start();
        
        setStatus(Running);
        emit started();
        
        qDebug() << "PerformanceModule: Successfully started";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceModule: Exception during start:" << e.what();
        emit errorOccurred(QString("Start exception: %1").arg(e.what()));
        return false;
    }
}

void PerformanceModule::stop()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != Running) {
        return;
    }
    
    try {
        // 停止状态更新定时器
        m_statusTimer->stop();
        
        // 停止性能管理器
        if (m_performanceManager) {
            m_performanceManager->stop();
        }
        
        // 停止指标收集器
        if (m_metricsCollector) {
            m_metricsCollector->stop();
        }
        
        setStatus(Ready);
        emit stopped();
        
        qDebug() << "PerformanceModule: Stopped";
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceModule: Exception during stop:" << e.what();
        emit errorOccurred(QString("Stop exception: %1").arg(e.what()));
    }
}

void PerformanceModule::pause()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != Running) {
        return;
    }
    
    // 暂停组件
    if (m_performanceManager) {
        // 性能管理器暂停逻辑
    }
    
    if (m_metricsCollector) {
        // 指标收集器暂停逻辑
    }
    
    setStatus(Paused);
    emit paused();
    
    qDebug() << "PerformanceModule: Paused";
}

void PerformanceModule::resume()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != Paused) {
        return;
    }
    
    // 恢复组件
    if (m_performanceManager) {
        // 性能管理器恢复逻辑
    }
    
    if (m_metricsCollector) {
        // 指标收集器恢复逻辑
    }
    
    setStatus(Running);
    emit resumed();
    
    qDebug() << "PerformanceModule: Resumed";
}

void PerformanceModule::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status == Shutdown) {
        return;
    }
    
    try {
        // 停止所有活动
        if (m_status == Running || m_status == Paused) {
            stop();
        }
        
        // 保存配置
        if (m_config) {
            m_config->saveConfig();
        }
        
        setStatus(Shutdown);
        emit shutdown();
        
        qDebug() << "PerformanceModule: Shutdown completed";
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceModule: Exception during shutdown:" << e.what();
    }
}

PerformanceModule::ModuleStatus PerformanceModule::status() const
{
    QMutexLocker locker(&m_mutex);
    return m_status;
}

QString PerformanceModule::version() const
{
    return PERFORMANCE_MODULE_VERSION;
}

PerformanceManager* PerformanceModule::performanceManager() const
{
    return m_performanceManager;
}

MetricsCollector* PerformanceModule::metricsCollector() const
{
    return m_metricsCollector;
}

PerformanceConfig* PerformanceModule::config() const
{
    return m_config;
}

bool PerformanceModule::isInitialized() const
{
    QMutexLocker locker(&m_mutex);
    return m_status != NotInitialized;
}

bool PerformanceModule::isRunning() const
{
    QMutexLocker locker(&m_mutex);
    return m_status == Running;
}

QVariantMap PerformanceModule::getStatistics() const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap stats;
    stats["status"] = QVariant::fromValue(m_status);
    stats["version"] = version();
    stats["initialized"] = isInitialized();
    stats["running"] = isRunning();
    
    if (m_performanceManager) {
        stats["performanceManager"] = m_performanceManager->getSystemInfo();
    }
    
    if (m_metricsCollector) {
        stats["metricsCollector"] = m_metricsCollector->getCollectorStatistics();
    }
    
    return stats;
}

void PerformanceModule::reset()
{
    QMutexLocker locker(&m_mutex);
    
    // 停止所有活动
    if (m_status == Running || m_status == Paused) {
        stop();
    }
    
    // 重置组件
    if (m_performanceManager) {
        // 重置性能管理器
    }
    
    if (m_metricsCollector) {
        m_metricsCollector->clearHistoricalData();
    }
    
    if (m_config) {
        m_config->resetToDefaults();
    }
    
    qDebug() << "PerformanceModule: Reset completed";
}

PerformanceModule* PerformanceModule::instance()
{
    QMutexLocker locker(&s_instanceMutex);
    
    if (!s_instance) {
        s_instance = new PerformanceModule();
    }
    
    return s_instance;
}

void PerformanceModule::setStatus(ModuleStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

bool PerformanceModule::initializeComponents()
{
    try {
        // 创建指标收集器
        m_metricsCollector = new MetricsCollector(this);
        m_metricsCollector->setConfig(m_config);
        
        if (!m_metricsCollector->initialize()) {
            qCritical() << "PerformanceModule: Failed to initialize metrics collector";
            return false;
        }
        
        // 创建性能管理器
        m_performanceManager = new PerformanceManager(this);
        m_performanceManager->setConfig(m_config);
        m_performanceManager->setMetricsCollector(m_metricsCollector);
        
        if (!m_performanceManager->initialize()) {
            qCritical() << "PerformanceModule: Failed to initialize performance manager";
            return false;
        }
        
        // 连接信号
        connect(m_performanceManager, &PerformanceManager::errorOccurred,
                this, &PerformanceModule::handleComponentError);
        connect(m_metricsCollector, &MetricsCollector::errorOccurred,
                this, &PerformanceModule::handleComponentError);
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceModule: Exception in initializeComponents:" << e.what();
        return false;
    }
}

void PerformanceModule::cleanup()
{
    // 清理资源
    if (m_performanceManager) {
        m_performanceManager->deleteLater();
        m_performanceManager = nullptr;
    }
    
    if (m_metricsCollector) {
        m_metricsCollector->deleteLater();
        m_metricsCollector = nullptr;
    }
    
    if (m_config) {
        m_config->deleteLater();
        m_config = nullptr;
    }
}

bool PerformanceModule::validateDependencies()
{
    // 检查Qt版本
    if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0)) {
        qCritical() << "PerformanceModule: Requires Qt 5.15 or higher";
        return false;
    }
    
    // 检查应用程序实例
    if (!QCoreApplication::instance()) {
        qCritical() << "PerformanceModule: Requires QCoreApplication instance";
        return false;
    }
    
    return true;
}

void PerformanceModule::handleStatusUpdate()
{
    // 定期状态检查和更新
    if (m_status == Running) {
        // 检查组件状态
        bool allComponentsRunning = true;
        
        if (m_performanceManager && !m_performanceManager->isRunning()) {
            allComponentsRunning = false;
        }
        
        if (m_metricsCollector && !m_metricsCollector->isCollecting()) {
            allComponentsRunning = false;
        }
        
        if (!allComponentsRunning) {
            qWarning() << "PerformanceModule: Some components are not running";
            emit errorOccurred("Component status check failed");
        }
    }
}

void PerformanceModule::handleComponentError(const QString& error)
{
    qWarning() << "PerformanceModule: Component error:" << error;
    emit errorOccurred(QString("Component error: %1").arg(error));
}