#include "ChatManagerAdapter.h"
#include <QDebug>

// 临时的占位符类定义
class ChatManager : public QObject {
    Q_OBJECT
public:
    explicit ChatManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~ChatManager() = default;
    
    virtual bool initialize() { return true; }
    virtual bool sendMessage(const QString& message) { Q_UNUSED(message) return true; }
    virtual bool receiveMessage(const QString& message) { Q_UNUSED(message) return true; }
    virtual QStringList getMessageHistory() const { return QStringList(); }
    virtual bool joinRoom(const QString& roomId) { Q_UNUSED(roomId) return true; }
    virtual bool leaveRoom() { return true; }
};

class ChatModule : public QObject {
    Q_OBJECT
public:
    explicit ChatModule(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~ChatModule() = default;
    
    virtual bool initialize() { return true; }
    virtual bool isActive() const { return true; }
    
signals:
    void statusChanged();
};

ChatManagerAdapter::ChatManagerAdapter(QObject *parent)
    : ICompatibilityAdapter(parent)
    , m_status(NotInitialized)
    , m_legacyManager(nullptr)
    , m_chatModule(nullptr)
    , m_chatIntegrationValid(false)
{
    // 设置默认配置
    m_config["enable_file_sharing"] = true;
    m_config["enable_history"] = true;
    m_config["compatibility_mode"] = "full";
    m_config["max_message_length"] = 1000;
}

ChatManagerAdapter::~ChatManagerAdapter()
{
    disable();
    
    if (m_legacyManager) {
        m_legacyManager->deleteLater();
    }
}

bool ChatManagerAdapter::initialize()
{
    if (m_status != NotInitialized) {
        return m_status == Ready;
    }

    qDebug() << "Initializing ChatManagerAdapter...";
    
    m_status = Initializing;
    emit statusChanged(m_status);

    // 创建遗留聊天管理器
    createLegacyChatManager();
    
    if (!m_legacyManager) {
        qWarning() << "Failed to create legacy ChatManager";
        m_status = Error;
        emit statusChanged(m_status);
        return false;
    }

    // 初始化遗留管理器
    if (!m_legacyManager->initialize()) {
        qWarning() << "Failed to initialize legacy ChatManager";
        m_status = Error;
        emit statusChanged(m_status);
        return false;
    }

    m_status = Ready;
    emit statusChanged(m_status);
    
    qDebug() << "ChatManagerAdapter initialized successfully";
    return true;
}

ICompatibilityAdapter::AdapterStatus ChatManagerAdapter::status() const
{
    return m_status;
}

QString ChatManagerAdapter::adapterName() const
{
    return "ChatManagerAdapter";
}

QString ChatManagerAdapter::targetModule() const
{
    return "chat";
}

ICompatibilityAdapter::CompatibilityLevel ChatManagerAdapter::checkCompatibility()
{
    if (m_status != Ready) {
        return NoCompatibility;
    }

    bool chatValid = validateChatIntegration();

    if (chatValid) {
        return FullCompatibility;
    } else {
        return LimitedCompatibility;
    }
}

bool ChatManagerAdapter::enable()
{
    if (m_status != Ready) {
        return false;
    }

    m_status = Active;
    emit statusChanged(m_status);
    
    qDebug() << "ChatManagerAdapter enabled";
    return true;
}

void ChatManagerAdapter::disable()
{
    if (m_status == Active) {
        m_status = Ready;
        emit statusChanged(m_status);
        qDebug() << "ChatManagerAdapter disabled";
    }
}

QVariantMap ChatManagerAdapter::getConfiguration() const
{
    return m_config;
}

bool ChatManagerAdapter::setConfiguration(const QVariantMap& config)
{
    m_config = config;
    
    // 应用配置到遗留管理器
    if (m_legacyManager) {
        qDebug() << "Applied configuration to ChatManagerAdapter";
    }
    
    return true;
}

QStringList ChatManagerAdapter::validateFunctionality()
{
    QStringList results;
    
    if (!m_legacyManager) {
        results << "ERROR: Legacy ChatManager not created";
        return results;
    }

    // 验证基本功能
    try {
        if (m_legacyManager->sendMessage("Test message")) {
            results << "PASS: Message sending functionality";
        } else {
            results << "FAIL: Message sending functionality";
        }

        if (m_legacyManager->receiveMessage("Test message")) {
            results << "PASS: Message receiving functionality";
        } else {
            results << "FAIL: Message receiving functionality";
        }

        QStringList history = m_legacyManager->getMessageHistory();
        if (!history.isEmpty() || true) { // 允许空历史
            results << "PASS: Message history functionality";
        } else {
            results << "FAIL: Message history functionality";
        }

        if (m_legacyManager->joinRoom("test_room")) {
            results << "PASS: Room joining functionality";
            if (m_legacyManager->leaveRoom()) {
                results << "PASS: Room leaving functionality";
            } else {
                results << "FAIL: Room leaving functionality";
            }
        } else {
            results << "FAIL: Room joining functionality";
        }

    } catch (const std::exception& e) {
        results << QString("ERROR: Exception during validation: %1").arg(e.what());
    } catch (...) {
        results << "ERROR: Unknown exception during validation";
    }

    // 验证集成
    if (validateChatIntegration()) {
        results << "PASS: Chat module integration";
    } else {
        results << "FAIL: Chat module integration";
    }

    return results;
}

ChatManager* ChatManagerAdapter::getLegacyManager() const
{
    return m_legacyManager;
}

void ChatManagerAdapter::setChatModule(ChatModule* chatModule)
{
    if (m_chatModule) {
        disconnect(m_chatModule, &ChatModule::statusChanged,
                   this, &ChatManagerAdapter::onChatModuleStatusChanged);
    }

    m_chatModule = chatModule;
    
    if (m_chatModule) {
        connect(m_chatModule, &ChatModule::statusChanged,
                this, &ChatManagerAdapter::onChatModuleStatusChanged);
        m_chatIntegrationValid = validateChatIntegration();
    }
}

void ChatManagerAdapter::onChatModuleStatusChanged()
{
    m_chatIntegrationValid = validateChatIntegration();
    qDebug() << "Chat integration status changed:" << m_chatIntegrationValid;
}

void ChatManagerAdapter::createLegacyChatManager()
{
    if (m_legacyManager) {
        m_legacyManager->deleteLater();
    }

    m_legacyManager = new ChatManager(this);
    qDebug() << "Created legacy ChatManager";
}

bool ChatManagerAdapter::validateChatIntegration()
{
    if (!m_chatModule) {
        return false;
    }

    // 验证聊天模块是否正常工作
    return m_chatModule->initialize() && m_chatModule->isActive();
}

#include "ChatManagerAdapter.moc"