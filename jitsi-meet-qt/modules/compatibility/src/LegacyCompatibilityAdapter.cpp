#include "LegacyCompatibilityAdapter.h"
#include "RollbackManager.h"
#include "CompatibilityValidator.h"
#include "ICompatibilityAdapter.h"

// 适配器实现
#include "MediaManagerAdapter.h"
#include "ChatManagerAdapter.h"
#include "ScreenShareManagerAdapter.h"
#include "ConferenceManagerAdapter.h"

#include <QDebug>
#include <QMutexLocker>
#include <QCoreApplication>

// 静态成员初始化
LegacyCompatibilityAdapter* LegacyCompatibilityAdapter::s_instance = nullptr;
QMutex LegacyCompatibilityAdapter::s_mutex;

LegacyCompatibilityAdapter::LegacyCompatibilityAdapter(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_rollbackManager(nullptr)
    , m_validator(nullptr)
{
    setupDefaultConfig();
}

LegacyCompatibilityAdapter::~LegacyCompatibilityAdapter()
{
    // 清理适配器
    for (auto adapter : m_adapters) {
        if (adapter) {
            adapter->disable();
            adapter->deleteLater();
        }
    }
    m_adapters.clear();

    // 清理管理器
    if (m_rollbackManager) {
        m_rollbackManager->deleteLater();
    }
    if (m_validator) {
        m_validator->deleteLater();
    }
}

bool LegacyCompatibilityAdapter::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing LegacyCompatibilityAdapter...";

    // 初始化回滚管理器
    m_rollbackManager = new RollbackManager(this);
    if (!m_rollbackManager->initialize()) {
        qWarning() << "Failed to initialize RollbackManager";
        return false;
    }

    // 初始化验证器
    m_validator = new CompatibilityValidator(this);
    if (!m_validator->initialize()) {
        qWarning() << "Failed to initialize CompatibilityValidator";
        return false;
    }

    // 连接信号
    connect(m_rollbackManager, &RollbackManager::rollbackCompleted,
            this, &LegacyCompatibilityAdapter::onRollbackCompleted);
    connect(m_validator, &CompatibilityValidator::validationCompleted,
            this, &LegacyCompatibilityAdapter::onValidationCompleted);

    // 初始化迁移状态
    m_migrationStatus[MediaAdapter] = NotStarted;
    m_migrationStatus[ChatAdapter] = NotStarted;
    m_migrationStatus[ScreenShareAdapter] = NotStarted;
    m_migrationStatus[ConferenceAdapter] = NotStarted;

    m_initialized = true;
    qDebug() << "LegacyCompatibilityAdapter initialized successfully";
    
    return true;
}

bool LegacyCompatibilityAdapter::isInitialized() const
{
    QMutexLocker locker(&m_mutex);
    return m_initialized;
}

// 静态工厂方法实现
MediaManager* LegacyCompatibilityAdapter::createLegacyMediaManager()
{
    QMutexLocker locker(&s_mutex);
    
    if (!s_instance) {
        s_instance = new LegacyCompatibilityAdapter();
        s_instance->initialize();
    }

    auto adapter = new MediaManagerAdapter();
    if (adapter->initialize()) {
        s_instance->registerAdapter(MediaAdapter, adapter);
        return adapter->getLegacyManager();
    }
    
    delete adapter;
    return nullptr;
}Ch
atManager* LegacyCompatibilityAdapter::createLegacyChatManager()
{
    QMutexLocker locker(&s_mutex);
    
    if (!s_instance) {
        s_instance = new LegacyCompatibilityAdapter();
        s_instance->initialize();
    }

    auto adapter = new ChatManagerAdapter();
    if (adapter->initialize()) {
        s_instance->registerAdapter(ChatAdapter, adapter);
        return adapter->getLegacyManager();
    }
    
    delete adapter;
    return nullptr;
}

ScreenShareManager* LegacyCompatibilityAdapter::createLegacyScreenShareManager()
{
    QMutexLocker locker(&s_mutex);
    
    if (!s_instance) {
        s_instance = new LegacyCompatibilityAdapter();
        s_instance->initialize();
    }

    auto adapter = new ScreenShareManagerAdapter();
    if (adapter->initialize()) {
        s_instance->registerAdapter(ScreenShareAdapter, adapter);
        return adapter->getLegacyManager();
    }
    
    delete adapter;
    return nullptr;
}

ConferenceManager* LegacyCompatibilityAdapter::createLegacyConferenceManager()
{
    QMutexLocker locker(&s_mutex);
    
    if (!s_instance) {
        s_instance = new LegacyCompatibilityAdapter();
        s_instance->initialize();
    }

    auto adapter = new ConferenceManagerAdapter();
    if (adapter->initialize()) {
        s_instance->registerAdapter(ConferenceAdapter, adapter);
        return adapter->getLegacyManager();
    }
    
    delete adapter;
    return nullptr;
}

bool LegacyCompatibilityAdapter::validateFunctionality(const QString& moduleName)
{
    QMutexLocker locker(&s_mutex);
    
    if (!s_instance) {
        s_instance = new LegacyCompatibilityAdapter();
        s_instance->initialize();
    }

    if (!s_instance->m_validator) {
        qWarning() << "Validator not initialized";
        return false;
    }

    auto result = s_instance->m_validator->validateFunctionality(moduleName);
    return result == ICompatibilityValidator::Passed;
}

QStringList LegacyCompatibilityAdapter::runCompatibilityTests()
{
    QMutexLocker locker(&s_mutex);
    
    if (!s_instance) {
        s_instance = new LegacyCompatibilityAdapter();
        s_instance->initialize();
    }

    if (!s_instance->m_validator) {
        return QStringList() << "Validator not initialized";
    }

    return s_instance->m_validator->runCompatibilityTests();
}

QVariantMap LegacyCompatibilityAdapter::getValidationReport(const QString& moduleName)
{
    QMutexLocker locker(&s_mutex);
    
    if (!s_instance) {
        s_instance = new LegacyCompatibilityAdapter();
        s_instance->initialize();
    }

    QVariantMap report;
    report["module"] = moduleName;
    report["timestamp"] = QDateTime::currentDateTime();
    
    if (s_instance->m_validator) {
        auto detailedReport = s_instance->m_validator->getDetailedReport();
        QVariantList tests;
        
        for (const auto& testReport : detailedReport) {
            QVariantMap testMap;
            testMap["name"] = testReport.testName;
            testMap["result"] = testReport.result;
            testMap["message"] = testReport.message;
            testMap["executionTime"] = testReport.executionTime;
            testMap["details"] = testReport.details;
            tests.append(testMap);
        }
        
        report["tests"] = tests;
    }
    
    return report;
}

bool LegacyCompatibilityAdapter::registerAdapter(AdapterType type, ICompatibilityAdapter* adapter)
{
    QMutexLocker locker(&m_mutex);
    
    if (!adapter || !validateAdapterType(type)) {
        return false;
    }

    // 如果已存在适配器，先清理
    if (m_adapters.contains(type)) {
        auto oldAdapter = m_adapters[type];
        oldAdapter->disable();
        oldAdapter->deleteLater();
    }

    m_adapters[type] = adapter;
    
    // 连接信号
    connect(adapter, &ICompatibilityAdapter::statusChanged,
            this, &LegacyCompatibilityAdapter::onAdapterStatusChanged);
    connect(adapter, &ICompatibilityAdapter::errorOccurred,
            this, &LegacyCompatibilityAdapter::errorOccurred);

    qDebug() << "Registered adapter for type:" << adapterTypeToString(type);
    return true;
}

ICompatibilityAdapter* LegacyCompatibilityAdapter::getAdapter(AdapterType type) const
{
    QMutexLocker locker(&m_mutex);
    return m_adapters.value(type, nullptr);
}

QList<LegacyCompatibilityAdapter::AdapterType> LegacyCompatibilityAdapter::getRegisteredAdapters() const
{
    QMutexLocker locker(&m_mutex);
    return m_adapters.keys();
}

bool LegacyCompatibilityAdapter::startMigration(AdapterType type)
{
    QMutexLocker locker(&m_mutex);
    
    if (!validateAdapterType(type) || !m_adapters.contains(type)) {
        return false;
    }

    if (m_migrationStatus[type] == InProgress) {
        qWarning() << "Migration already in progress for type:" << adapterTypeToString(type);
        return false;
    }

    // 创建检查点
    QString checkpointName = QString("migration_%1_%2")
                           .arg(adapterTypeToString(type))
                           .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    
    if (!m_rollbackManager->createCheckpoint(checkpointName, 
                                           QString("Migration checkpoint for %1").arg(adapterTypeToString(type)))) {
        qWarning() << "Failed to create checkpoint for migration";
        return false;
    }

    m_migrationStatus[type] = InProgress;
    emit migrationStarted(type);
    
    qDebug() << "Started migration for type:" << adapterTypeToString(type);
    return true;
}

bool LegacyCompatibilityAdapter::completeMigration(AdapterType type)
{
    QMutexLocker locker(&m_mutex);
    
    if (!validateAdapterType(type) || m_migrationStatus[type] != InProgress) {
        return false;
    }

    auto adapter = m_adapters.value(type);
    if (!adapter) {
        return false;
    }

    // 验证迁移结果
    auto validationResults = adapter->validateFunctionality();
    bool hasErrors = false;
    
    for (const QString& result : validationResults) {
        if (result.contains("ERROR") || result.contains("FAILED")) {
            hasErrors = true;
            break;
        }
    }

    if (hasErrors) {
        qWarning() << "Migration validation failed for type:" << adapterTypeToString(type);
        m_migrationStatus[type] = Failed;
        emit migrationCompleted(type, false);
        return false;
    }

    m_migrationStatus[type] = Completed;
    emit migrationCompleted(type, true);
    
    qDebug() << "Completed migration for type:" << adapterTypeToString(type);
    return true;
}

bool LegacyCompatibilityAdapter::rollbackMigration(AdapterType type)
{
    QMutexLocker locker(&m_mutex);
    
    if (!validateAdapterType(type)) {
        return false;
    }

    // 查找最近的检查点
    QString checkpointPattern = QString("migration_%1_").arg(adapterTypeToString(type));
    auto checkpoints = m_rollbackManager->availableCheckpoints();
    
    QString targetCheckpoint;
    for (const QString& checkpoint : checkpoints) {
        if (checkpoint.startsWith(checkpointPattern)) {
            if (targetCheckpoint.isEmpty() || checkpoint > targetCheckpoint) {
                targetCheckpoint = checkpoint;
            }
        }
    }

    if (targetCheckpoint.isEmpty()) {
        qWarning() << "No checkpoint found for rollback of type:" << adapterTypeToString(type);
        return false;
    }

    if (!m_rollbackManager->rollbackToCheckpoint(targetCheckpoint)) {
        qWarning() << "Failed to rollback to checkpoint:" << targetCheckpoint;
        return false;
    }

    m_migrationStatus[type] = RolledBack;
    qDebug() << "Rolled back migration for type:" << adapterTypeToString(type);
    return true;
}

LegacyCompatibilityAdapter::MigrationStatus LegacyCompatibilityAdapter::getMigrationStatus(AdapterType type) const
{
    QMutexLocker locker(&m_mutex);
    return m_migrationStatus.value(type, NotStarted);
}

void LegacyCompatibilityAdapter::setGlobalConfig(const QVariantMap& config)
{
    QMutexLocker locker(&m_mutex);
    m_globalConfig = config;
}

QVariantMap LegacyCompatibilityAdapter::getGlobalConfig() const
{
    QMutexLocker locker(&m_mutex);
    return m_globalConfig;
}

void LegacyCompatibilityAdapter::setAdapterConfig(AdapterType type, const QVariantMap& config)
{
    QMutexLocker locker(&m_mutex);
    m_adapterConfigs[type] = config;
    
    auto adapter = m_adapters.value(type);
    if (adapter) {
        adapter->setConfiguration(config);
    }
}

QVariantMap LegacyCompatibilityAdapter::getAdapterConfig(AdapterType type) const
{
    QMutexLocker locker(&m_mutex);
    return m_adapterConfigs.value(type);
}

void LegacyCompatibilityAdapter::onAdapterStatusChanged()
{
    auto adapter = qobject_cast<ICompatibilityAdapter*>(sender());
    if (!adapter) {
        return;
    }

    // 查找适配器类型
    AdapterType type = AllAdapters;
    for (auto it = m_adapters.begin(); it != m_adapters.end(); ++it) {
        if (it.value() == adapter) {
            type = it.key();
            break;
        }
    }

    if (type != AllAdapters) {
        qDebug() << "Adapter status changed for type:" << adapterTypeToString(type) 
                 << "Status:" << adapter->status();
    }
}

void LegacyCompatibilityAdapter::onValidationCompleted(const QStringList& results)
{
    qDebug() << "Validation completed with results:" << results;
}

void LegacyCompatibilityAdapter::onRollbackCompleted(const QString& checkpointName, bool success)
{
    qDebug() << "Rollback completed for checkpoint:" << checkpointName << "Success:" << success;
}

void LegacyCompatibilityAdapter::setupDefaultConfig()
{
    m_globalConfig["validation_enabled"] = true;
    m_globalConfig["performance_check_enabled"] = true;
    m_globalConfig["auto_rollback_enabled"] = false;
    m_globalConfig["checkpoint_retention_days"] = 30;
    m_globalConfig["max_rollback_attempts"] = 3;
}

bool LegacyCompatibilityAdapter::validateAdapterType(AdapterType type) const
{
    return type >= MediaAdapter && type < AllAdapters;
}

QString LegacyCompatibilityAdapter::adapterTypeToString(AdapterType type) const
{
    switch (type) {
    case MediaAdapter: return "Media";
    case ChatAdapter: return "Chat";
    case ScreenShareAdapter: return "ScreenShare";
    case ConferenceAdapter: return "Conference";
    case AllAdapters: return "All";
    default: return "Unknown";
    }
}