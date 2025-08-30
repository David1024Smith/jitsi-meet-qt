#include "ChatModule.h"
#include "ChatManager.h"
#include "MessageHandler.h"
#include "ChatConfig.h"

#include <QDebug>
#include <QVariantMap>
#include <QTimer>

class ChatModule::Private
{
public:
    Private(ChatModule* q) : q_ptr(q) {}
    
    ChatModule* q_ptr;
    
    QString version = CHAT_MODULE_VERSION;
    Status status = NotInitialized;
    bool initialized = false;
    
    ChatManager* chatManager = nullptr;
    MessageHandler* messageHandler = nullptr;
    ChatConfig* config = nullptr;
    
    QTimer* statusTimer = nullptr;
    
    void setStatus(Status newStatus) {
        if (status != newStatus) {
            status = newStatus;
            emit q_ptr->moduleStatusChanged(newStatus);
            emit q_ptr->statusChanged(statusString());
        }
    }
    
    QString statusString() const {
        switch (status) {
        case NotInitialized: return "Not Initialized";
        case Initializing: return "Initializing";
        case Ready: return "Ready";
        case Error: return "Error";
        case Shutting_Down: return "Shutting Down";
        default: return "Unknown";
        }
    }
};

ChatModule::ChatModule(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>(this))
{
    qDebug() << "ChatModule created";
}

ChatModule::~ChatModule()
{
    shutdown();
    qDebug() << "ChatModule destroyed";
}

QString ChatModule::version() const
{
    return d->version;
}

bool ChatModule::initialize(const QVariantMap& config)
{
    if (d->initialized) {
        qWarning() << "ChatModule already initialized";
        return true;
    }
    
    qDebug() << "Initializing ChatModule...";
    d->setStatus(Initializing);
    
    try {
        // 验证配置
        if (!validateConfiguration(config)) {
            qCritical() << "Invalid configuration";
            d->setStatus(Error);
            return false;
        }
        
        // 初始化组件
        if (!initializeComponents()) {
            qCritical() << "Failed to initialize components";
            d->setStatus(Error);
            return false;
        }
        
        d->initialized = true;
        d->setStatus(Ready);
        
        qDebug() << "ChatModule initialized successfully";
        emit initializedChanged(true);
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during initialization:" << e.what();
        d->setStatus(Error);
        return false;
    }
}

void ChatModule::shutdown()
{
    if (!d->initialized) {
        return;
    }
    
    qDebug() << "Shutting down ChatModule...";
    d->setStatus(Shutting_Down);
    
    cleanup();
    
    d->initialized = false;
    d->setStatus(NotInitialized);
    
    emit initializedChanged(false);
    qDebug() << "ChatModule shutdown complete";
}

bool ChatModule::isInitialized() const
{
    return d->initialized;
}

QString ChatModule::status() const
{
    return d->statusString();
}

ChatModule::Status ChatModule::moduleStatus() const
{
    return d->status;
}

ChatManager* ChatModule::chatManager() const
{
    return d->chatManager;
}

MessageHandler* ChatModule::messageHandler() const
{
    return d->messageHandler;
}

void ChatModule::setConfiguration(const ChatConfig& config)
{
    if (d->config) {
        *d->config = config;
        emit configurationChanged();
    }
}

ChatConfig ChatModule::configuration() const
{
    if (d->config) {
        return *d->config;
    }
    return ChatConfig();
}

bool ChatModule::reloadConfiguration()
{
    // TODO: 实现配置重新加载
    emit configurationChanged();
    return true;
}

QVariantMap ChatModule::moduleInfo() const
{
    QVariantMap info;
    info["name"] = "Chat Module";
    info["version"] = d->version;
    info["status"] = d->statusString();
    info["initialized"] = d->initialized;
    return info;
}

QVariantMap ChatModule::statistics() const
{
    QVariantMap stats;
    stats["uptime"] = 0; // TODO: 实现运行时间统计
    stats["messages_processed"] = 0; // TODO: 实现消息统计
    return stats;
}

void ChatModule::start()
{
    if (!d->initialized) {
        qWarning() << "Cannot start uninitialized module";
        return;
    }
    
    // TODO: 实现启动逻辑
    emit started();
}

void ChatModule::stop()
{
    // TODO: 实现停止逻辑
    emit stopped();
}

void ChatModule::restart()
{
    stop();
    start();
}

void ChatModule::reset()
{
    shutdown();
    // TODO: 重置到默认状态
}

void ChatModule::handleInternalError(const QString& error)
{
    qCritical() << "Internal error:" << error;
    d->setStatus(Error);
    emit errorOccurred(error);
}

bool ChatModule::initializeComponents()
{
    // 创建配置对象
    d->config = new ChatConfig(this);
    
    // 创建消息处理器
    d->messageHandler = new MessageHandler(this);
    if (!d->messageHandler->initialize()) {
        qCritical() << "Failed to initialize MessageHandler";
        return false;
    }
    
    // 创建聊天管理器
    d->chatManager = new ChatManager(this);
    if (!d->chatManager->initialize()) {
        qCritical() << "Failed to initialize ChatManager";
        return false;
    }
    
    // 连接信号槽
    connect(d->chatManager, &ChatManager::errorOccurred,
            this, &ChatModule::handleInternalError);
    connect(d->messageHandler, &MessageHandler::processingError,
            this, &ChatModule::handleInternalError);
    
    return true;
}

void ChatModule::cleanup()
{
    if (d->chatManager) {
        d->chatManager->disconnect();
        d->chatManager = nullptr;
    }
    
    if (d->messageHandler) {
        d->messageHandler->stopProcessing();
        d->messageHandler = nullptr;
    }
    
    if (d->config) {
        d->config = nullptr;
    }
}

void ChatModule::setStatus(Status status)
{
    d->setStatus(status);
}

bool ChatModule::validateConfiguration(const QVariantMap& config)
{
    Q_UNUSED(config)
    // TODO: 实现配置验证
    return true;
}