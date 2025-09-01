#include "../include/ScreenShareModule.h"
#include "../include/ScreenShareManager.h"
#include "../config/ScreenShareConfig.h"
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

#ifndef SCREENSHARE_MODULE_VERSION
#define SCREENSHARE_MODULE_VERSION "1.0.0"
#endif

class ScreenShareModule::Private
{
public:
    Private() 
        : status(NotLoaded)
        , initialized(false)
        , enabled(true)
        , manager(nullptr)
        , config(nullptr)
    {
    }

    ModuleStatus status;
    bool initialized;
    bool enabled;
    ScreenShareManager* manager;
    ScreenShareConfig* config;
    QVariantMap configuration;
    QStringList errors;
    
    static ScreenShareModule* instance;
};

ScreenShareModule* ScreenShareModule::Private::instance = nullptr;

ScreenShareModule::ScreenShareModule(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    if (!Private::instance) {
        Private::instance = this;
    }
}

ScreenShareModule::~ScreenShareModule()
{
    shutdown();
    if (Private::instance == this) {
        Private::instance = nullptr;
    }
    delete d;
}

ScreenShareModule* ScreenShareModule::instance()
{
    return Private::instance;
}

bool ScreenShareModule::initialize(const QVariantMap& config)
{
    if (d->initialized) {
        return true;
    }

    updateStatus(Initializing);
    
    try {
        // 设置配置
        if (!config.isEmpty()) {
            setConfiguration(config);
        }
        
        // 初始化组件
        initializeComponents();
        
        d->initialized = true;
        updateStatus(Ready);
        
        emit moduleReady();
        return true;
        
    } catch (const std::exception& e) {
        d->errors.append(QString("Initialization failed: %1").arg(e.what()));
        updateStatus(Error);
        emit moduleError(d->errors.last());
        return false;
    }
}

void ScreenShareModule::shutdown()
{
    if (!d->initialized) {
        return;
    }
    
    cleanupComponents();
    d->initialized = false;
    updateStatus(NotLoaded);
    
    emit moduleShutdown();
}

bool ScreenShareModule::isInitialized() const
{
    return d->initialized;
}

ScreenShareModule::ModuleStatus ScreenShareModule::status() const
{
    return d->status;
}

QString ScreenShareModule::version() const
{
    return SCREENSHARE_MODULE_VERSION;
}

bool ScreenShareModule::isEnabled() const
{
    return d->enabled;
}

void ScreenShareModule::setEnabled(bool enabled)
{
    if (d->enabled != enabled) {
        d->enabled = enabled;
        emit enabledChanged(enabled);
    }
}

void ScreenShareModule::reload()
{
    if (d->initialized) {
        shutdown();
    }
    initialize(d->configuration);
}

void ScreenShareModule::reset()
{
    shutdown();
    d->configuration.clear();
    d->errors.clear();
}

void ScreenShareModule::setConfiguration(const QVariantMap& config)
{
    if (validateConfiguration(config)) {
        d->configuration = config;
        emit configurationChanged(config);
    }
}

QVariantMap ScreenShareModule::configuration() const
{
    return d->configuration;
}

IScreenShareManager* ScreenShareModule::screenShareManager() const
{
    return d->manager;
}

QString ScreenShareModule::moduleName() const
{
    return "ScreenShare";
}

QString ScreenShareModule::moduleDescription() const
{
    return "Screen sharing and capture module for Jitsi Meet Qt";
}

QStringList ScreenShareModule::dependencies() const
{
    return QStringList() << "Qt5Core" << "Qt5Gui" << "Qt5Widgets" << "Qt5Multimedia";
}

QVariantMap ScreenShareModule::moduleInfo() const
{
    QVariantMap info;
    info["name"] = moduleName();
    info["version"] = version();
    info["description"] = moduleDescription();
    info["dependencies"] = dependencies();
    info["status"] = static_cast<int>(status());
    info["initialized"] = isInitialized();
    info["enabled"] = isEnabled();
    return info;
}

bool ScreenShareModule::selfTest()
{
    // 基础自检
    if (!d->manager) {
        d->errors.append("ScreenShareManager not initialized");
        return false;
    }
    
    if (!d->config) {
        d->errors.append("ScreenShareConfig not initialized");
        return false;
    }
    
    // 功能自检
    if (!d->manager->isReady()) {
        d->errors.append("ScreenShareManager not ready");
        return false;
    }
    
    return true;
}

QStringList ScreenShareModule::getLastErrors() const
{
    return d->errors;
}

void ScreenShareModule::clearErrors()
{
    d->errors.clear();
}

void ScreenShareModule::start()
{
    if (!d->initialized) {
        initialize();
    }
    setEnabled(true);
}

void ScreenShareModule::stop()
{
    setEnabled(false);
}

void ScreenShareModule::restart()
{
    stop();
    start();
}

void ScreenShareModule::onManagerStatusChanged(IScreenShareManager::ManagerStatus status)
{
    Q_UNUSED(status)
    // 处理管理器状态变化
}

void ScreenShareModule::onManagerError(const QString& error)
{
    d->errors.append(error);
    emit moduleError(error);
}

void ScreenShareModule::initializeComponents()
{
    // 创建配置对象
    if (!d->config) {
        d->config = new ScreenShareConfig(this);
    }
    
    // 创建管理器
    if (!d->manager) {
        d->manager = new ScreenShareManager(this);
        connect(d->manager, &IScreenShareManager::statusChanged,
                this, &ScreenShareModule::onManagerStatusChanged);
        connect(d->manager, &IScreenShareManager::shareError,
                this, &ScreenShareModule::onManagerError);
    }
    
    // 初始化管理器
    if (!d->manager->initialize()) {
        throw std::runtime_error("Failed to initialize ScreenShareManager");
    }
}

void ScreenShareModule::cleanupComponents()
{
    if (d->manager) {
        d->manager->shutdown();
    }
}

bool ScreenShareModule::validateConfiguration(const QVariantMap& config) const
{
    Q_UNUSED(config)
    // 配置验证逻辑
    return true;
}

void ScreenShareModule::updateStatus(ModuleStatus newStatus)
{
    if (d->status != newStatus) {
        d->status = newStatus;
        emit statusChanged(newStatus);
    }
}