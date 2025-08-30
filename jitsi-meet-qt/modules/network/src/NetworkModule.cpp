#include "../include/NetworkModule.h"
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

class NetworkModule::Private
{
public:
    ModuleStatus status;
    QString moduleName;
    QString moduleVersion;
    QVariantMap configuration;
    QMutex mutex;
    
    Private() 
        : status(NotInitialized)
        , moduleName("NetworkModule")
        , moduleVersion("1.0.0")
    {
    }
};

// 静态实例指针
static NetworkModule* s_instance = nullptr;
static QMutex s_instanceMutex;

NetworkModule::NetworkModule(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

NetworkModule::~NetworkModule()
{
    if (d->status != NotInitialized && d->status != Shutdown) {
        shutdown();
    }
    delete d;
}

NetworkModule* NetworkModule::instance()
{
    QMutexLocker locker(&s_instanceMutex);
    if (!s_instance) {
        s_instance = new NetworkModule();
    }
    return s_instance;
}

bool NetworkModule::initialize(const QVariantMap& config)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != NotInitialized) {
        qWarning() << "NetworkModule: Already initialized";
        return false;
    }
    
    d->status = Initializing;
    emit statusChanged(d->status);
    
    // 验证配置
    if (!validateConfiguration(config)) {
        d->status = Error;
        emit statusChanged(d->status);
        emit errorOccurred("Invalid configuration");
        return false;
    }
    
    // 设置配置
    d->configuration = config;
    
    // 执行实际初始化
    if (!doInitialize()) {
        d->status = Error;
        emit statusChanged(d->status);
        emit errorOccurred("Initialization failed");
        return false;
    }
    
    d->status = Ready;
    emit statusChanged(d->status);
    emit initialized();
    
    qDebug() << "NetworkModule: Initialized successfully";
    return true;
}

void NetworkModule::shutdown()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status == Shutdown || d->status == NotInitialized) {
        return;
    }
    
    qDebug() << "NetworkModule: Shutting down...";
    
    doShutdown();
    
    d->status = Shutdown;
    emit statusChanged(d->status);
    emit shutdownCompleted();
    
    qDebug() << "NetworkModule: Shutdown completed";
}

NetworkModule::ModuleStatus NetworkModule::status() const
{
    QMutexLocker locker(&d->mutex);
    return d->status;
}

QString NetworkModule::moduleName() const
{
    return d->moduleName;
}

QString NetworkModule::moduleVersion() const
{
    return d->moduleVersion;
}

bool NetworkModule::isInitialized() const
{
    QMutexLocker locker(&d->mutex);
    return d->status == Ready;
}

QVariantMap NetworkModule::configuration() const
{
    QMutexLocker locker(&d->mutex);
    return d->configuration;
}

void NetworkModule::setConfiguration(const QVariantMap& config)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->configuration != config) {
        d->configuration = config;
        
        // 如果模块已初始化，重新应用配置
        if (d->status == Ready) {
            // 这里可以添加热重载配置的逻辑
            qDebug() << "NetworkModule: Configuration updated";
        }
    }
}

void NetworkModule::handleStatusChange()
{
    // 处理状态变化的内部逻辑
    qDebug() << "NetworkModule: Status changed to" << d->status;
}

bool NetworkModule::doInitialize()
{
    // 执行实际的初始化工作
    try {
        // 初始化网络组件
        qDebug() << "NetworkModule: Initializing network components...";
        
        // 这里可以添加具体的初始化逻辑
        // 例如：初始化网络管理器、连接工厂等
        
        return true;
    } catch (const std::exception& e) {
        qCritical() << "NetworkModule: Initialization exception:" << e.what();
        return false;
    }
}

void NetworkModule::doShutdown()
{
    // 执行实际的关闭工作
    try {
        qDebug() << "NetworkModule: Cleaning up network components...";
        
        // 这里可以添加具体的清理逻辑
        // 例如：关闭连接、清理资源等
        
    } catch (const std::exception& e) {
        qCritical() << "NetworkModule: Shutdown exception:" << e.what();
    }
}

bool NetworkModule::validateConfiguration(const QVariantMap& config)
{
    // 验证配置参数的有效性
    
    // 检查必需的配置项
    if (config.contains("serverUrl")) {
        QString serverUrl = config["serverUrl"].toString();
        if (serverUrl.isEmpty()) {
            qWarning() << "NetworkModule: Invalid server URL";
            return false;
        }
    }
    
    if (config.contains("serverPort")) {
        int port = config["serverPort"].toInt();
        if (port <= 0 || port > 65535) {
            qWarning() << "NetworkModule: Invalid server port:" << port;
            return false;
        }
    }
    
    if (config.contains("connectionTimeout")) {
        int timeout = config["connectionTimeout"].toInt();
        if (timeout <= 0) {
            qWarning() << "NetworkModule: Invalid connection timeout:" << timeout;
            return false;
        }
    }
    
    return true;
}