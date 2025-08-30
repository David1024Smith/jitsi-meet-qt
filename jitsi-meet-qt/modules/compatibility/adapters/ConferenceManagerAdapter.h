#ifndef CONFERENCEMANAGERADAPTER_H
#define CONFERENCEMANAGERADAPTER_H

#include "ICompatibilityAdapter.h"
#include <QObject>

// 前向声明
class ConferenceManager;
class MeetingModule;

/**
 * @brief 会议管理器适配器
 */
class ConferenceManagerAdapter : public ICompatibilityAdapter
{
    Q_OBJECT

public:
    explicit ConferenceManagerAdapter(QObject *parent = nullptr);
    ~ConferenceManagerAdapter();

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
    ConferenceManager* getLegacyManager() const;

private:
    void createLegacyConferenceManager();

    AdapterStatus m_status;
    QVariantMap m_config;
    ConferenceManager* m_legacyManager;
};

#endif // CONFERENCEMANAGERADAPTER_H