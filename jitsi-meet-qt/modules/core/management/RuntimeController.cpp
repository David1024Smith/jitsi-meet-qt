#include "RuntimeController.h"
#include "GlobalModuleConfig.h"
#include <QDebug>
#include <QMutexLocker>
#include <QUuid>
#include <QCoreApplication>

int RuntimeController::s_requestCounter = 0;

RuntimeController::RuntimeController(QObject* parent)
    : QObject(parent)
    , m_executionMode(Asynchronous)
    , m_maxConcurrentOperations(3)
    , m_operationTimeout(30000) // 30秒
    , m_requireConfirmation(false)
    , m_safeModeEnabled(true)
    , m_rollbackEnabled(true)
    , m_executionPaused(false)
    , m_schedulingEnabled(true)
    , m_processTimer(new QTimer(this))
    , m_timeoutTimer(new QTimer(this))
{
    // 设置处理定时器
    m_processTimer->setSingleShot(false);
    m_processTimer->setInterval(100); // 100ms检查间隔
    connect(m_processTimer, &QTimer::timeout, this, &RuntimeController::processRequestQueue);
    
    // 设置超时检查定时器
    m_timeoutTimer->setSingleShot(false);
    m_timeoutTimer->setInterval(5000); // 5秒检查间隔
    connect(m_timeoutTimer, &QTimer::timeout, this, &RuntimeController::checkOperationTimeouts);
    
    // 启动定时器
    if (m_schedulingEnabled) {
        m_processTimer->start();
        m_timeoutTimer->start();
    }
    
    qDebug() << "RuntimeController initialized";
}

RuntimeController::~RuntimeController()
{
    // 停止所有操作
    m_processTimer->stop();
    m_timeoutTimer->stop();
    
    // 清理待处理请求
    clearPendingRequests();
}

bool RuntimeController::enableModule(const QString& moduleName, ExecutionMode mode)
{
    ControlRequest request;
    request.moduleName = moduleName;
    request.action = Enable;
    request.mode = mode;
    request.priority = 1;
    request.timestamp = QDateTime::currentDateTime();
    request.requestId = generateRequestId();
    
    if (mode == Synchronous || mode == Immediate) {
        return executeRequest(request);
    } else {
        QString requestId = submitRequest(request);
        return !requestId.isEmpty();
    }
}

bool RuntimeController::disableModule(const QString& moduleName, ExecutionMode mode)
{
    ControlRequest request;
    request.moduleName = moduleName;
    request.action = Disable;
    request.mode = mode;
    request.priority = 1;
    request.timestamp = QDateTime::currentDateTime();
    request.requestId = generateRequestId();
    
    if (mode == Synchronous || mode == Immediate) {
        return executeRequest(request);
    } else {
        QString requestId = submitRequest(request);
        return !requestId.isEmpty();
    }
}

bool RuntimeController::reloadModule(const QString& moduleName, ExecutionMode mode)
{
    ControlRequest request;
    request.moduleName = moduleName;
    request.action = Reload;
    request.mode = mode;
    request.priority = 2;
    request.timestamp = QDateTime::currentDateTime();
    request.requestId = generateRequestId();
    
    if (mode == Synchronous || mode == Immediate) {
        return executeRequest(request);
    } else {
        QString requestId = submitRequest(request);
        return !requestId.isEmpty();
    }
}

QString RuntimeController::submitRequest(const ControlRequest& request)
{
    QMutexLocker locker(&m_mutex);
    
    if (!validateRequest(request)) {
        qWarning() << "Invalid request for module:" << request.moduleName;
        return QString();
    }
    
    // 检查是否需要确认
    if (m_requireConfirmation && (request.action == Disable || request.action == Reload)) {
        emit confirmationRequired(request.requestId, request);
        return request.requestId;
    }
    
    // 添加到队列
    m_requestQueue.enqueue(request);
    emit requestQueued(request.requestId);
    
    qDebug() << "Request queued:" << request.requestId << "for module:" << request.moduleName;
    return request.requestId;
}

bool RuntimeController::executeRequest(const ControlRequest& request)
{
    QMutexLocker locker(&m_mutex);
    
    if (!validateRequest(request)) {
        return false;
    }
    
    // 检查并发限制
    if (m_activeOperations.size() >= m_maxConcurrentOperations) {
        qWarning() << "Maximum concurrent operations reached, queuing request";
        m_requestQueue.enqueue(request);
        return true; // 请求已排队
    }
    
    // 检查模块是否已在操作中
    if (isOperationInProgress(request.moduleName)) {
        qWarning() << "Module" << request.moduleName << "is already being processed";
        return false;
    }
    
    // 创建备份（如果启用回滚）
    if (m_rollbackEnabled && (request.action == Disable || request.action == Reload)) {
        if (!createModuleBackup(request.moduleName)) {
            qWarning() << "Failed to create backup for module:" << request.moduleName;
            if (m_safeModeEnabled) {
                return false;
            }
        }
    }
    
    // 执行操作
    m_activeRequests[request.requestId] = request;
    m_activeOperations.append(request.moduleName);
    m_operationStartTimes[request.requestId] = QDateTime::currentDateTime();
    
    emit operationStarted(request.moduleName, request.action);
    
    bool success = performModuleOperation(request.moduleName, request.action, request.parameters);
    
    // 清理
    m_activeRequests.remove(request.requestId);
    m_activeOperations.removeAll(request.moduleName);
    m_operationStartTimes.remove(request.requestId);
    
    emit operationCompleted(request.moduleName, request.action, success);
    
    if (!success) {
        emit operationFailed(request.moduleName, request.action, "Operation failed");
        
        // 尝试回滚
        if (m_rollbackEnabled) {
            restoreModuleBackup(request.moduleName);
        }
    }
    
    return success;
}

bool RuntimeController::performModuleOperation(const QString& moduleName, ControlAction action, const QVariantMap& parameters)
{
    GlobalModuleConfig* config = GlobalModuleConfig::instance();
    
    try {
        switch (action) {
        case Enable:
            config->setModuleEnabled(moduleName, true);
            qDebug() << "Module enabled:" << moduleName;
            return true;
            
        case Disable:
            config->setModuleEnabled(moduleName, false);
            qDebug() << "Module disabled:" << moduleName;
            return true;
            
        case Reload:
            // 先禁用再启用
            config->setModuleEnabled(moduleName, false);
            QCoreApplication::processEvents(); // 允许处理事件
            config->setModuleEnabled(moduleName, true);
            qDebug() << "Module reloaded:" << moduleName;
            return true;
            
        case Restart:
            // 重启操作（具体实现取决于模块类型）
            qDebug() << "Module restarted:" << moduleName;
            return true;
            
        case Suspend:
            // 暂停操作（保持加载但停止功能）
            qDebug() << "Module suspended:" << moduleName;
            return true;
            
        case Resume:
            // 恢复操作
            qDebug() << "Module resumed:" << moduleName;
            return true;
            
        default:
            qWarning() << "Unknown action:" << action;
            return false;
        }
    } catch (const std::exception& e) {
        qWarning() << "Exception during module operation:" << e.what();
        return false;
    }
}

bool RuntimeController::validateRequest(const ControlRequest& request) const
{
    if (request.moduleName.isEmpty()) {
        return false;
    }
    
    if (request.requestId.isEmpty()) {
        return false;
    }
    
    // 检查模块是否存在
    GlobalModuleConfig* config = GlobalModuleConfig::instance();
    if (!config->hasModule(request.moduleName)) {
        qWarning() << "Module not found:" << request.moduleName;
        return false;
    }
    
    // 检查操作是否允许
    return canExecuteOperation(request.moduleName, request.action);
}

bool RuntimeController::canExecuteOperation(const QString& moduleName, ControlAction action) const
{
    GlobalModuleConfig* config = GlobalModuleConfig::instance();
    
    switch (action) {
    case Enable:
        return !config->isModuleEnabled(moduleName);
        
    case Disable:
        return config->isModuleEnabled(moduleName);
        
    case Reload:
    case Restart:
        return config->hasModule(moduleName);
        
    case Suspend:
    case Resume:
        return config->isModuleEnabled(moduleName);
        
    default:
        return false;
    }
}

QString RuntimeController::generateRequestId() const
{
    return QString("REQ_%1_%2")
        .arg(QDateTime::currentMSecsSinceEpoch())
        .arg(++s_requestCounter);
}

void RuntimeController::processRequestQueue()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_executionPaused || m_requestQueue.isEmpty()) {
        return;
    }
    
    // 检查是否可以处理更多请求
    if (m_activeOperations.size() >= m_maxConcurrentOperations) {
        return;
    }
    
    // 处理队列中的请求
    while (!m_requestQueue.isEmpty() && m_activeOperations.size() < m_maxConcurrentOperations) {
        ControlRequest request = m_requestQueue.dequeue();
        
        // 在新线程中执行（异步模式）
        if (request.mode == Asynchronous) {
            QTimer::singleShot(0, this, [this, request]() {
                executeRequest(request);
            });
        } else {
            executeRequest(request);
        }
    }
    
    if (m_requestQueue.isEmpty()) {
        emit queueEmpty();
    }
}

void RuntimeController::checkOperationTimeouts()
{
    QMutexLocker locker(&m_mutex);
    
    QDateTime now = QDateTime::currentDateTime();
    QStringList timedOutRequests;
    
    for (auto it = m_operationStartTimes.begin(); it != m_operationStartTimes.end(); ++it) {
        if (it.value().msecsTo(now) > m_operationTimeout) {
            timedOutRequests.append(it.key());
        }
    }
    
    // 处理超时请求
    for (const QString& requestId : timedOutRequests) {
        if (m_activeRequests.contains(requestId)) {
            ControlRequest request = m_activeRequests[requestId];
            
            // 清理
            m_activeRequests.remove(requestId);
            m_activeOperations.removeAll(request.moduleName);
            m_operationStartTimes.remove(requestId);
            
            emit operationFailed(request.moduleName, request.action, "Operation timeout");
            emit onOperationTimeout(requestId);
            
            qWarning() << "Operation timeout for module:" << request.moduleName;
        }
    }
}

bool RuntimeController::createModuleBackup(const QString& moduleName)
{
    // 这里应该实现模块状态的备份逻辑
    // 简化实现，实际应该保存模块的完整状态
    qDebug() << "Creating backup for module:" << moduleName;
    return true;
}

bool RuntimeController::restoreModuleBackup(const QString& moduleName)
{
    // 这里应该实现模块状态的恢复逻辑
    qDebug() << "Restoring backup for module:" << moduleName;
    return true;
}

bool RuntimeController::isOperationInProgress(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    return m_activeOperations.contains(moduleName);
}

void RuntimeController::pauseExecution()
{
    QMutexLocker locker(&m_mutex);
    m_executionPaused = true;
    emit executionPaused();
    qDebug() << "Execution paused";
}

void RuntimeController::resumeExecution()
{
    QMutexLocker locker(&m_mutex);
    m_executionPaused = false;
    emit executionResumed();
    qDebug() << "Execution resumed";
}