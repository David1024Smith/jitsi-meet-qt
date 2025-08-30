#ifndef SCREENSHAREMANAGERADAPTER_H
#define SCREENSHAREMANAGERADAPTER_H

#include "ICompatibilityAdapter.h"
#include <QObject>

// 前向声明
class ScreenShareManager;
class ScreenShareModule;

/**
 * @brief 屏幕共享管理器适配器
 */
class ScreenShareManagerAdapter : public ICompatibilityAdapter
{
    Q_OBJECT

public:
    explicit ScreenShareManagerAdapter(QObject *parent = nullptr);
    ~ScreenShareManagerAdapter();

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
    ScreenShareManager* getLegacyManager() const;

private:
    void createLegacyScreenShareManager();

    AdapterStatus m_status;
    QVariantMap m_config;
    ScreenShareManager* m_legacyManager;
};

#endif // SCREENSHAREMANAGERADAPTER_H