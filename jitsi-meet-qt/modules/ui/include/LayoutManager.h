#ifndef LAYOUTMANAGER_H
#define LAYOUTMANAGER_H

#include "interfaces/ILayoutManager.h"
#include <QObject>
#include <QString>
#include <QMap>
#include <memory>

class BaseLayout;
class MainLayout;
class ConferenceLayout;
class SettingsLayout;

/**
 * @brief 布局管理器
 * 
 * LayoutManager负责管理应用程序中的各种布局，包括主布局、
 * 会议布局和设置布局等。
 */
class LayoutManager : public QObject, public ILayoutManager
{
    Q_OBJECT

public:
    explicit LayoutManager(QObject *parent = nullptr);
    ~LayoutManager() override;

    // ILayoutManager接口实现
    bool initialize() override;
    void shutdown() override;
    LayoutStatus status() const override;

    bool setLayout(const QString& layoutName) override;
    QString currentLayout() const override;
    QStringList availableLayouts() const override;
    bool updateLayout() override;

    bool applyLayoutToWidget(const QString& layoutName, QWidget* widget) override;
    bool removeLayoutFromWidget(QWidget* widget) override;

    std::shared_ptr<BaseLayout> getLayout(const QString& layoutName) const override;
    bool registerLayout(const QString& layoutName, std::shared_ptr<BaseLayout> layout) override;
    bool unregisterLayout(const QString& layoutName) override;

    bool isLayoutRegistered(const QString& layoutName) const override;
    QStringList registeredLayouts() const override;

    bool setLayoutProperty(const QString& layoutName, const QString& property, const QVariant& value) override;
    QVariant getLayoutProperty(const QString& layoutName, const QString& property) const override;

    bool saveLayoutConfiguration(const QString& layoutName) override;
    bool loadLayoutConfiguration(const QString& layoutName) override;

    bool validateLayout(const QString& layoutName) const override;
    QStringList getLayoutValidationErrors(const QString& layoutName) const override;

signals:
    void layoutChanged(const QString& layoutName);
    void layoutApplied(const QString& layoutName, QWidget* widget);
    void layoutRemoved(QWidget* widget);
    void layoutRegistered(const QString& layoutName);
    void layoutUnregistered(const QString& layoutName);
    void layoutPropertyChanged(const QString& layoutName, const QString& property);
    void errorOccurred(const QString& error);

private slots:
    void onLayoutError(const QString& error);

private:
    void setupDefaultLayouts();
    void connectLayoutSignals(std::shared_ptr<BaseLayout> layout);
    void disconnectLayoutSignals(std::shared_ptr<BaseLayout> layout);
    bool validateLayoutName(const QString& layoutName) const;

    LayoutStatus m_status;
    QString m_currentLayoutName;
    
    QMap<QString, std::shared_ptr<BaseLayout>> m_registeredLayouts;
    QMap<QWidget*, std::shared_ptr<BaseLayout>> m_appliedLayouts;
    
    // 默认布局实例
    std::shared_ptr<MainLayout> m_mainLayout;
    std::shared_ptr<ConferenceLayout> m_conferenceLayout;
    std::shared_ptr<SettingsLayout> m_settingsLayout;
};

#endif // LAYOUTMANAGER_H