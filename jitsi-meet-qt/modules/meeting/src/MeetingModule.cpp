#include "MeetingModule.h"
#include "MeetingManager.h"
#include "LinkHandler.h"
#include "MeetingConfig.h"
#include <QDebug>
#include <QTimer>

// 静态成员初始化
MeetingModule* MeetingModule::s_instance = nullptr;

class MeetingModule::Private
{
public:
    ModuleStatus status = Uninitialized;
    QString version = "1.0.0";
    QString moduleName = "Meeting";
    
    std::unique_ptr<MeetingManager> meetingManager;
    std::unique_ptr<LinkHandler> linkHandler;
    std::unique_ptr<MeetingConfig> config;
    
    QVariantMap configuration;
    QVariantMap statistics;
    
    QTimer* healthCheckTimer = nullptr;
};

MeetingModule::MeetingModule(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->healthCheckTimer = new QTimer(this);
    d->healthCheckTimer->setInterval(30000); // 30秒健康检查
    connect(d->healthCheckTimer, &QTimer::timeout, this, [this]() {
        // 执行健康检查逻辑
        auto health = healthCheck();
        if (health.value("status").toString() != "healthy") {
            emit errorOccurred("Module health check failed");
        }
    });
}

MeetingModule::~MeetingModule()
{
    shutdown();
}

MeetingModule* MeetingModule::instance()
{
    if (!s_instance) {
        s_instance = new MeetingModule();
    }
    return s_instance;
}

bool MeetingModule::initialize(const QVariantMap& config)
{
    if (d->status != Uninitialized) {
        qWarning() << "Meeting module already initialized";
        return false;
    }
    
    setStatus(Initializing);
    
    try {
        // 设置配置
        d->configuration = config;
        
        // 初始化组件
        if (!initializeComponents()) {
            setStatus(Error);
            return false;
        }
        
        // 验证依赖
        if (!validateDependencies()) {
            setStatus(Error);
            emit errorOccurred("Module dependencies validation failed");
            return false;
        }
        
        // 启动健康检查
        d->healthCheckTimer->start();
        
        setStatus(Ready);
        emit initialized(true);
        
        qDebug() << "Meeting module initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        setStatus(Error);
        emit errorOccurred(QString("Initialization failed: %1").arg(e.what()));
        return false;
    }
}

void MeetingModule::shutdown()
{
    if (d->status == Shutdown) {
        return;
    }
    
    setStatus(Shutdown);
    
    // 停止健康检查
    if (d->healthCheckTimer) {
        d->healthCheckTimer->stop();
    }
    
    // 清理资源
    cleanup();
    
    emit shutdownCompleted();
    qDebug() << "Meeting module shutdown completed";
}

MeetingModule::ModuleStatus MeetingModule::status() const
{
    return d->status;
}

QString MeetingModule::version() const
{
    return d->version;
}

QString MeetingModule::moduleName() const
{
    return d->moduleName;
}

MeetingManager* MeetingModule::meetingManager() const
{
    return d->meetingManager.get();
}

LinkHandler* MeetingModule::linkHandler() const
{
    return d->linkHandler.get();
}

MeetingConfig* MeetingModule::config() const
{
    return d->config.get();
}

void MeetingModule::setConfiguration(const QVariantMap& config)
{
    d->configuration = config;
    
    // 更新子组件配置
    if (d->config) {
        d->config->fromVariantMap(config);
    }
    
    if (d->meetingManager) {
        d->meetingManager->setConfiguration(config);
    }
    
    emit configurationChanged(config);
}

QVariantMap MeetingModule::getConfiguration() const
{
    return d->configuration;
}

bool MeetingModule::reloadConfiguration()
{
    if (d->config) {
        return d->config->loadConfiguration();
    }
    return false;
}

bool MeetingModule::validateDependencies() const
{
    // 检查Qt版本
    // 检查网络连接
    // 检查其他必需模块
    return true;
}

QVariantMap MeetingModule::getStatistics() const
{
    QVariantMap stats = d->statistics;
    stats["status"] = static_cast<int>(d->status);
    stats["version"] = d->version;
    stats["uptime"] = 0; // 计算运行时间
    
    if (d->meetingManager) {
        stats["meetings"] = d->meetingManager->getMeetingStatistics();
    }
    
    return stats;
}

void MeetingModule::reset()
{
    if (d->status == Active) {
        qWarning() << "Cannot reset active module";
        return;
    }
    
    cleanup();
    setStatus(Uninitialized);
    
    qDebug() << "Meeting module reset completed";
}

QVariantMap MeetingModule::healthCheck() const
{
    QVariantMap health;
    health["timestamp"] = QDateTime::currentDateTime();
    health["module"] = d->moduleName;
    health["version"] = d->version;
    health["status"] = "healthy";
    
    // 检查组件状态
    if (d->meetingManager) {
        health["meetingManager"] = "active";
    } else {
        health["meetingManager"] = "inactive";
        health["status"] = "degraded";
    }
    
    if (d->linkHandler) {
        health["linkHandler"] = "active";
    } else {
        health["linkHandler"] = "inactive";
        health["status"] = "degraded";
    }
    
    return health;
}

void MeetingModule::handleInternalError(const QString& error)
{
    qWarning() << "Meeting module internal error:" << error;
    emit errorOccurred(error);
}

bool MeetingModule::initializeComponents()
{
    try {
        // 初始化配置
        d->config = std::make_unique<MeetingConfig>(this);
        if (!d->config->loadConfiguration()) {
            qWarning() << "Failed to load meeting configuration";
        }
        
        // 初始化链接处理器
        d->linkHandler = std::make_unique<LinkHandler>(this);
        connect(d->linkHandler.get(), &LinkHandler::errorOccurred,
                this, &MeetingModule::handleInternalError);
        
        // 初始化会议管理器
        d->meetingManager = std::make_unique<MeetingManager>(this);
        d->meetingManager->setLinkHandler(d->linkHandler.get());
        d->meetingManager->setMeetingConfig(d->config.get());
        
        connect(d->meetingManager.get(), &MeetingManager::errorOccurred,
                this, &MeetingModule::handleInternalError);
        
        // 初始化管理器
        if (!d->meetingManager->initialize()) {
            qWarning() << "Failed to initialize meeting manager";
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Component initialization failed:" << e.what();
        return false;
    }
}

void MeetingModule::cleanup()
{
    d->meetingManager.reset();
    d->linkHandler.reset();
    d->config.reset();
    
    d->statistics.clear();
}

void MeetingModule::setStatus(ModuleStatus status)
{
    if (d->status != status) {
        d->status = status;
        emit statusChanged(status);
    }
}