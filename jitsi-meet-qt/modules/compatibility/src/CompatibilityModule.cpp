#include "CompatibilityModule.h"
#include "LegacyCompatibilityAdapter.h"
#include "RollbackManager.h"
#include "CompatibilityValidator.h"
#include "CompatibilityConfig.h"

#include <QDebug>
#include <QCoreApplication>

CompatibilityModule::CompatibilityModule(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_adapter(nullptr)
    , m_rollbackManager(nullptr)
    , m_validator(nullptr)
    , m_config(nullptr)
{
}

CompatibilityModule::~CompatibilityModule()
{
    if (m_adapter) {
        m_adapter->deleteLater();
    }
    if (m_rollbackManager) {
        m_rollbackManager->deleteLater();
    }
    if (m_validator) {
        m_validator->deleteLater();
    }
    if (m_config) {
        m_config->deleteLater();
    }
}

bool CompatibilityModule::initialize()
{
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing CompatibilityModule...";

    // 初始化配置
    m_config = new CompatibilityConfig(this);
    if (!m_config->loadConfiguration()) {
        qWarning() << "Failed to load compatibility configuration";
        // 继续使用默认配置
    }

    // 初始化回滚管理器
    m_rollbackManager = new RollbackManager(this);
    if (!m_rollbackManager->initialize()) {
        qWarning() << "Failed to initialize RollbackManager";
        emit moduleError("Failed to initialize RollbackManager");
        return false;
    }

    // 初始化验证器
    m_validator = new CompatibilityValidator(this);
    if (!m_validator->initialize()) {
        qWarning() << "Failed to initialize CompatibilityValidator";
        emit moduleError("Failed to initialize CompatibilityValidator");
        return false;
    }

    // 初始化适配器
    m_adapter = new LegacyCompatibilityAdapter(this);
    if (!m_adapter->initialize()) {
        qWarning() << "Failed to initialize LegacyCompatibilityAdapter";
        emit moduleError("Failed to initialize LegacyCompatibilityAdapter");
        return false;
    }

    // 应用配置
    if (m_config) {
        m_validator->setValidationConfig(m_config->getValidatorConfig());
        m_adapter->setGlobalConfig(m_config->getValidatorConfig());
    }

    m_initialized = true;
    emit moduleInitialized();
    
    qDebug() << "CompatibilityModule initialized successfully";
    return true;
}

bool CompatibilityModule::isInitialized() const
{
    return m_initialized;
}

LegacyCompatibilityAdapter* CompatibilityModule::getAdapter() const
{
    return m_adapter;
}

RollbackManager* CompatibilityModule::getRollbackManager() const
{
    return m_rollbackManager;
}

CompatibilityValidator* CompatibilityModule::getValidator() const
{
    return m_validator;
}

CompatibilityConfig* CompatibilityModule::getConfig() const
{
    return m_config;
}

QString CompatibilityModule::getModuleName() const
{
    return "CompatibilityModule";
}

QString CompatibilityModule::getModuleVersion() const
{
    return "1.0.0";
}

QVariantMap CompatibilityModule::getModuleInfo() const
{
    QVariantMap info;
    info["name"] = getModuleName();
    info["version"] = getModuleVersion();
    info["description"] = "Compatibility adapter system for safe modular refactoring";
    info["initialized"] = m_initialized;
    info["application_version"] = QCoreApplication::applicationVersion();
    
    if (m_initialized) {
        info["adapter_available"] = (m_adapter != nullptr);
        info["rollback_available"] = (m_rollbackManager != nullptr);
        info["validator_available"] = (m_validator != nullptr);
        info["config_available"] = (m_config != nullptr);
    }
    
    return info;
}