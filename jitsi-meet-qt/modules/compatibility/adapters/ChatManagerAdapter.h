#ifndef CHATMANAGERADAPTER_H
#define CHATMANAGERADAPTER_H

#include "ICompatibilityAdapter.h"
#include <QObject>

// 前向声明
class ChatManager;
class ChatModule;

/**
 * @brief 聊天管理器适配器
 * 
 * 提供旧的ChatManager API到新的聊天模块的适配。
 */
class ChatManagerAdapter : public ICompatibilityAdapter
{
    Q_OBJECT

public:
    explicit ChatManagerAdapter(QObject *parent = nullptr);
    ~ChatManagerAdapter();

    // ICompatibilityAdapter 接口实现
    bool initialize() override;
    AdapterStatus status() const override;
    QString adapterName() const override;
    QString targetModule() const override;
    CompatibilityLevel checkCompatibility() override;
    bool enable() override;
    void disable() override;
    QVariantMap getConfiguration() const override;
    bool setConfiguration(const QVariantMap& config) override;
    QStringList validateFunctionality() override;

    // 特定功能
    ChatManager* getLegacyManager() const;
    void setChatModule(ChatModule* chatModule);

private slots:
    void onChatModuleStatusChanged();

private:
    void createLegacyChatManager();
    bool validateChatIntegration();

    AdapterStatus m_status;
    QVariantMap m_config;
    
    ChatManager* m_legacyManager;
    ChatModule* m_chatModule;
    
    bool m_chatIntegrationValid;
};

#endif // CHATMANAGERADAPTER_H