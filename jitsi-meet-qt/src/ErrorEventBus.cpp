#include "../include/ErrorEventBus.h"
#include "../include/ModuleError.h"
#include <QDebug>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(errorBus, "jitsi.errorbus")

// 静态成员初始化
ErrorEventBus* ErrorEventBus::s_instance = nullptr;

ErrorEventBus::ErrorEventBus(QObject *parent)
    : QObject(parent)
    , m_globalRecoveryStrategy(nullptr)
    , m_autoRecoveryEnabled(true)
    , m_maxHistorySize(DEFAULT_MAX_HISTORY_SIZE)
    , m_processTimer(new QTimer(this))
    , m_statisticsTimer(new QTimer(this))
    , m_errorLoggingEnabled(true)
    , m_initialized(false)
{
    // 设置处理定时器
    m_processTimer->setSingleShot(false);
    m_processTimer->setInterval(PROCESS_INTERVAL);
    connect(m_processTimer, &QTimer::timeout, this, &ErrorEventBus::processErrorQueue);
    
    // 设置统计更新定时器
    m_statisticsTimer->setSingleShot(false);
    m_statisticsTimer->setInterval(STATISTICS_INTERVAL);
    connect(m_statisticsTimer, &QTimer::timeout, this, &ErrorEventBus::updateStatistics);
    
    // 创建默认恢复策略
    m_globalRecoveryStrategy = new DefaultErrorRecoveryStrategy(this);
}

ErrorEventBus::~ErrorEventBus()
{
    shutdown();
}

ErrorEventBus* ErrorEventBus::instance()
{
    if (!s_instance) {
        s_instance = new ErrorEventBus();
    }
    return s_instance;
}

bool ErrorEventBus::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    qCDebug(errorBus) << "ErrorEventBus: Initializing error event bus...";
    
    // 启动定时器
    m_processTimer->start();
    m_statisticsTimer->start();
    
    m_initialized = true;
    
    qCDebug(errorBus) << "ErrorEventBus: Error event bus initialized successfully";
    
    return true;
}

void ErrorEventBus::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    qCDebug(errorBus) << "ErrorEventBus: Shutting down error event bus...";
    
    // 停止定时器
    m_processTimer->stop();
    m_statisticsTimer->stop();
    
    // 处理剩余的错误队列
    while (!m_errorQueue.isEmpty()) {
        processErrorQueue();
    }
    
    // 清理订阅者
    m_moduleSubscribers.clear();
    m_typeSubscribers.clear();
    m_severitySubscribers.clear();
    m_globalSubscribers.clear();
    m_errorFilters.clear();
    
    // 清理恢复策略
    for (auto strategy : m_recoveryStrategies) {
        if (strategy && strategy->parent() == this) {
            delete strategy;
        }
    }
    m_recoveryStrategies.clear();
    
    m_initialized = false;
    
    qCDebug(errorBus) << "ErrorEventBus: Error event bus shut down";
}

void ErrorEventBus::reportError(const ModuleError& error)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        qWarning() << "ErrorEventBus: Cannot report error - bus not initialized";
        return;
    }
    
    // 添加到错误队列
    m_errorQueue.enqueue(error);
    
    // 记录错误日志
    if (m_errorLoggingEnabled) {
        logError(error);
    }
    
    // 更新统计信息
    updateErrorStatistics(error);
    
    // 立即处理高优先级错误
    if (error.severity() >= ModuleError::Critical) {
        processErrorQueue();
    }
}

void ErrorEventBus::subscribeToErrors(QObject* subscriber, const QString& moduleName)
{
    if (!subscriber) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    if (moduleName.isEmpty()) {
        if (!m_globalSubscribers.contains(subscriber)) {
            m_globalSubscribers.append(subscriber);
            connect(subscriber, &QObject::destroyed, this, &ErrorEventBus::onSubscriberDestroyed);
        }
    } else {
        if (!m_moduleSubscribers[moduleName].contains(subscriber)) {
            m_moduleSubscribers[moduleName].append(subscriber);
            connect(subscriber, &QObject::destroyed, this, &ErrorEventBus::onSubscriberDestroyed);
        }
    }
    
    qCDebug(errorBus) << "ErrorEventBus: Subscriber registered for module:" << moduleName;
}

void ErrorEventBus::unsubscribeFromErrors(QObject* subscriber, const QString& moduleName)
{
    if (!subscriber) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    if (moduleName.isEmpty()) {
        m_globalSubscribers.removeAll(subscriber);
        
        // 从所有模块订阅中移除
        for (auto& subscribers : m_moduleSubscribers) {
            subscribers.removeAll(subscriber);
        }
        
        // 从类型订阅中移除
        for (auto& subscribers : m_typeSubscribers) {
            subscribers.removeAll(subscriber);
        }
        
        // 从严重程度订阅中移除
        for (auto& subscribers : m_severitySubscribers) {
            subscribers.removeAll(subscriber);
        }
        
        // 移除过滤器
        m_errorFilters.remove(subscriber);
        
        disconnect(subscriber, &QObject::destroyed, this, &ErrorEventBus::onSubscriberDestroyed);
    } else {
        m_moduleSubscribers[moduleName].removeAll(subscriber);
        
        if (m_moduleSubscribers[moduleName].isEmpty()) {
            m_moduleSubscribers.remove(moduleName);
        }
    }
    
    qCDebug(errorBus) << "ErrorEventBus: Subscriber unregistered from module:" << moduleName;
}

void ErrorEventBus::subscribeToErrorType(QObject* subscriber, ModuleError::ErrorType errorType)
{
    if (!subscriber) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    if (!m_typeSubscribers[errorType].contains(subscriber)) {
        m_typeSubscribers[errorType].append(subscriber);
        connect(subscriber, &QObject::destroyed, this, &ErrorEventBus::onSubscriberDestroyed);
    }
    
    qCDebug(errorBus) << "ErrorEventBus: Subscriber registered for error type:" << errorType;
}

void ErrorEventBus::unsubscribeFromErrorType(QObject* subscriber, ModuleError::ErrorType errorType)
{
    if (!subscriber) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    m_typeSubscribers[errorType].removeAll(subscriber);
    
    if (m_typeSubscribers[errorType].isEmpty()) {
        m_typeSubscribers.remove(errorType);
    }
}

void ErrorEventBus::subscribeToSeverity(QObject* subscriber, ModuleError::Severity severity)
{
    if (!subscriber) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    if (!m_severitySubscribers[severity].contains(subscriber)) {
        m_severitySubscribers[severity].append(subscriber);
        connect(subscriber, &QObject::destroyed, this, &ErrorEventBus::onSubscriberDestroyed);
    }
    
    qCDebug(errorBus) << "ErrorEventBus: Subscriber registered for severity:" << severity;
}

void ErrorEventBus::unsubscribeFromSeverity(QObject* subscriber, ModuleError::Severity severity)
{
    if (!subscriber) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    m_severitySubscribers[severity].removeAll(subscriber);
    
    if (m_severitySubscribers[severity].isEmpty()) {
        m_severitySubscribers.remove(severity);
    }
}

void ErrorEventBus::addErrorFilter(ErrorFilter* filter, QObject* subscriber)
{
    if (!filter || !subscriber) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    if (!m_errorFilters[subscriber].contains(filter)) {
        m_errorFilters[subscriber].append(filter);
    }
    
    qCDebug(errorBus) << "ErrorEventBus: Error filter added:" << filter->name();
}

void ErrorEventBus::removeErrorFilter(ErrorFilter* filter, QObject* subscriber)
{
    if (!filter || !subscriber) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    m_errorFilters[subscriber].removeAll(filter);
    
    if (m_errorFilters[subscriber].isEmpty()) {
        m_errorFilters.remove(subscriber);
    }
}

void ErrorEventBus::setRecoveryStrategy(ErrorRecoveryStrategy* strategy, const QString& moduleName)
{
    QMutexLocker locker(&m_mutex);
    
    if (moduleName.isEmpty()) {
        if (m_globalRecoveryStrategy && m_globalRecoveryStrategy->parent() == this) {
            delete m_globalRecoveryStrategy;
        }
        m_globalRecoveryStrategy = strategy;
    } else {
        if (m_recoveryStrategies.contains(moduleName)) {
            ErrorRecoveryStrategy* oldStrategy = m_recoveryStrategies[moduleName];
            if (oldStrategy && oldStrategy->parent() == this) {
                delete oldStrategy;
            }
        }
        m_recoveryStrategies[moduleName] = strategy;
    }
    
    qCDebug(errorBus) << "ErrorEventBus: Recovery strategy set for module:" << moduleName;
}

ErrorRecoveryStrategy* ErrorEventBus::getRecoveryStrategy(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!moduleName.isEmpty() && m_recoveryStrategies.contains(moduleName)) {
        return m_recoveryStrategies[moduleName];
    }
    
    return m_globalRecoveryStrategy;
}

void ErrorEventBus::removeRecoveryStrategy(const QString& moduleName)
{
    QMutexLocker locker(&m_mutex);
    
    if (moduleName.isEmpty()) {
        if (m_globalRecoveryStrategy && m_globalRecoveryStrategy->parent() == this) {
            delete m_globalRecoveryStrategy;
        }
        m_globalRecoveryStrategy = nullptr;
    } else {
        if (m_recoveryStrategies.contains(moduleName)) {
            ErrorRecoveryStrategy* strategy = m_recoveryStrategies[moduleName];
            if (strategy && strategy->parent() == this) {
                delete strategy;
            }
            m_recoveryStrategies.remove(moduleName);
        }
    }
}

void ErrorEventBus::setAutoRecoveryEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_autoRecoveryEnabled = enabled;
    
    qCDebug(errorBus) << "ErrorEventBus: Auto recovery" << (enabled ? "enabled" : "disabled");
}

bool ErrorEventBus::isAutoRecoveryEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoRecoveryEnabled;
}

ErrorEventBus::ErrorStatistics ErrorEventBus::getStatistics() const
{
    QMutexLocker locker(&m_mutex);
    return m_globalStatistics;
}

ErrorEventBus::ErrorStatistics ErrorEventBus::getModuleStatistics(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    return m_moduleStatistics.value(moduleName);
}

void ErrorEventBus::resetStatistics()
{
    QMutexLocker locker(&m_mutex);
    
    m_globalStatistics = ErrorStatistics();
    m_moduleStatistics.clear();
    
    qCDebug(errorBus) << "ErrorEventBus: Statistics reset";
}

QList<ModuleError> ErrorEventBus::getRecentErrors(int count) const
{
    QMutexLocker locker(&m_mutex);
    
    int startIndex = qMax(0, m_errorHistory.size() - count);
    return m_errorHistory.mid(startIndex);
}

QList<ModuleError> ErrorEventBus::getModuleRecentErrors(const QString& moduleName, int count) const
{
    QMutexLocker locker(&m_mutex);
    
    QList<ModuleError> moduleErrors;
    
    for (const ModuleError& error : m_errorHistory) {
        if (error.moduleName() == moduleName) {
            moduleErrors.append(error);
        }
    }
    
    int startIndex = qMax(0, moduleErrors.size() - count);
    return moduleErrors.mid(startIndex);
}

void ErrorEventBus::clearErrorHistory()
{
    QMutexLocker locker(&m_mutex);
    
    m_errorHistory.clear();
    
    qCDebug(errorBus) << "ErrorEventBus: Error history cleared";
}

void ErrorEventBus::setMaxHistorySize(int maxSize)
{
    QMutexLocker locker(&m_mutex);
    
    m_maxHistorySize = qMax(0, maxSize);
    
    // 清理超出限制的历史记录
    while (m_errorHistory.size() > m_maxHistorySize) {
        m_errorHistory.removeFirst();
    }
    
    qCDebug(errorBus) << "ErrorEventBus: Max history size set to" << m_maxHistorySize;
}

int ErrorEventBus::maxHistorySize() const
{
    QMutexLocker locker(&m_mutex);
    return m_maxHistorySize;
}

void ErrorEventBus::setErrorLoggingEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_errorLoggingEnabled = enabled;
    
    qCDebug(errorBus) << "ErrorEventBus: Error logging" << (enabled ? "enabled" : "disabled");
}

bool ErrorEventBus::isErrorLoggingEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_errorLoggingEnabled;
}

void ErrorEventBus::handleErrorRecovery(const ModuleError& error)
{
    if (!m_autoRecoveryEnabled) {
        return;
    }
    
    ErrorRecoveryStrategy* strategy = getRecoveryStrategy(error.moduleName());
    if (!strategy) {
        return;
    }
    
    if (strategy->canRecover(error)) {
        QString strategyName = strategy->strategyName();
        
        qCDebug(errorBus) << "ErrorEventBus: Starting error recovery for" 
                         << error.moduleName() << "using strategy" << strategyName;
        
        emit errorRecoveryStarted(error, strategyName);
        
        bool success = strategy->executeRecovery(error);
        
        emit errorRecoveryCompleted(error, strategyName, success);
        
        if (success) {
            qCDebug(errorBus) << "ErrorEventBus: Error recovery successful for" << error.moduleName();
        } else {
            qCWarning(errorBus) << "ErrorEventBus: Error recovery failed for" << error.moduleName();
        }
    }
}

void ErrorEventBus::updateStatistics()
{
    QMutexLocker locker(&m_mutex);
    
    // 计算错误率
    QDateTime now = QDateTime::currentDateTime();
    QDateTime oneMinuteAgo = now.addSecs(-60);
    
    int recentErrors = 0;
    for (const ModuleError& error : m_errorHistory) {
        if (error.timestamp() >= oneMinuteAgo) {
            recentErrors++;
        }
    }
    
    m_globalStatistics.errorRate = recentErrors;
    
    emit statisticsUpdated(m_globalStatistics);
}

void ErrorEventBus::processErrorQueue()
{
    QMutexLocker locker(&m_mutex);
    
    while (!m_errorQueue.isEmpty()) {
        ModuleError error = m_errorQueue.dequeue();
        
        // 分发错误事件
        dispatchError(error);
        
        // 添加到历史记录
        m_errorHistory.append(error);
        
        // 清理过期历史记录
        if (m_errorHistory.size() > m_maxHistorySize) {
            m_errorHistory.removeFirst();
        }
        
        // 尝试错误恢复
        handleErrorRecovery(error);
    }
}

void ErrorEventBus::onSubscriberDestroyed(QObject* obj)
{
    unsubscribeFromErrors(obj);
}

void ErrorEventBus::dispatchError(const ModuleError& error)
{
    // 发送全局错误信号
    emit errorReported(error);
    
    // 发送模块特定错误信号
    if (!error.moduleName().isEmpty()) {
        emit moduleErrorReported(error.moduleName(), error);
    }
    
    // 发送错误类型信号
    emit errorTypeReported(error.type(), error);
    
    // 发送严重程度信号
    emit severityReported(error.severity(), error);
    
    // 通知全局订阅者
    for (QObject* subscriber : m_globalSubscribers) {
        if (matchesSubscriber(subscriber, error)) {
            // 这里可以通过信号槽或直接调用方法通知订阅者
            // 具体实现取决于订阅者的接口设计
        }
    }
    
    // 通知模块特定订阅者
    if (m_moduleSubscribers.contains(error.moduleName())) {
        for (QObject* subscriber : m_moduleSubscribers[error.moduleName()]) {
            if (matchesSubscriber(subscriber, error)) {
                // 通知订阅者
            }
        }
    }
    
    // 通知错误类型订阅者
    if (m_typeSubscribers.contains(error.type())) {
        for (QObject* subscriber : m_typeSubscribers[error.type()]) {
            if (matchesSubscriber(subscriber, error)) {
                // 通知订阅者
            }
        }
    }
    
    // 通知严重程度订阅者
    if (m_severitySubscribers.contains(error.severity())) {
        for (QObject* subscriber : m_severitySubscribers[error.severity()]) {
            if (matchesSubscriber(subscriber, error)) {
                // 通知订阅者
            }
        }
    }
}

bool ErrorEventBus::matchesSubscriber(QObject* subscriber, const ModuleError& error) const
{
    return applyFilters(subscriber, error);
}

bool ErrorEventBus::applyFilters(QObject* subscriber, const ModuleError& error) const
{
    if (!m_errorFilters.contains(subscriber)) {
        return true; // 没有过滤器，通过
    }
    
    const QList<ErrorFilter*>& filters = m_errorFilters[subscriber];
    
    for (ErrorFilter* filter : filters) {
        if (!filter->filter(error)) {
            return false; // 任何一个过滤器不通过，就不通过
        }
    }
    
    return true;
}

void ErrorEventBus::updateErrorStatistics(const ModuleError& error)
{
    // 更新全局统计
    m_globalStatistics.totalErrors++;
    m_globalStatistics.errorsByType[error.type()]++;
    m_globalStatistics.errorsBySeverity[error.severity()]++;
    m_globalStatistics.errorsByModule[error.moduleName()]++;
    m_globalStatistics.lastError = error.timestamp();
    
    // 更新模块统计
    if (!error.moduleName().isEmpty()) {
        ErrorStatistics& moduleStats = m_moduleStatistics[error.moduleName()];
        moduleStats.totalErrors++;
        moduleStats.errorsByType[error.type()]++;
        moduleStats.errorsBySeverity[error.severity()]++;
        moduleStats.lastError = error.timestamp();
    }
}

void ErrorEventBus::logError(const ModuleError& error)
{
    QString logMessage = QString("[%1] %2")
                        .arg(error.moduleName().isEmpty() ? "GLOBAL" : error.moduleName())
                        .arg(error.toString());
    
    switch (error.severity()) {
    case ModuleError::Fatal:
    case ModuleError::Critical:
        qCCritical(errorBus) << logMessage;
        break;
    case ModuleError::Error:
        qCWarning(errorBus) << logMessage;
        break;
    case ModuleError::Warning:
        qCWarning(errorBus) << logMessage;
        break;
    case ModuleError::Info:
    default:
        qCInfo(errorBus) << logMessage;
        break;
    }
}

void ErrorEventBus::cleanupErrorHistory()
{
    QDateTime cutoff = QDateTime::currentDateTime().addDays(-30); // 保留30天
    
    auto it = m_errorHistory.begin();
    while (it != m_errorHistory.end()) {
        if (it->timestamp() < cutoff) {
            it = m_errorHistory.erase(it);
        } else {
            ++it;
        }
    }
}

// 过滤器实现

ModuleNameFilter::ModuleNameFilter(const QString& moduleName)
    : m_moduleName(moduleName)
{
}

bool ModuleNameFilter::filter(const ModuleError& error) const
{
    return error.moduleName() == m_moduleName;
}

QString ModuleNameFilter::name() const
{
    return QString("ModuleNameFilter(%1)").arg(m_moduleName);
}

ErrorTypeFilter::ErrorTypeFilter(ModuleError::ErrorType errorType)
    : m_errorType(errorType)
{
}

bool ErrorTypeFilter::filter(const ModuleError& error) const
{
    return error.type() == m_errorType;
}

QString ErrorTypeFilter::name() const
{
    return QString("ErrorTypeFilter(%1)").arg(ModuleError::errorTypeName(m_errorType));
}

SeverityFilter::SeverityFilter(ModuleError::Severity severity)
    : m_severity(severity)
{
}

bool SeverityFilter::filter(const ModuleError& error) const
{
    return error.severity() >= m_severity;
}

QString SeverityFilter::name() const
{
    return QString("SeverityFilter(%1)").arg(ModuleError::severityName(m_severity));
}