#ifndef LAYOUTMANAGER_H
#define LAYOUTMANAGER_H

#include "../interfaces/ILayoutManager.h"
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
    Q_INTERFACES(ILayoutManager)

public:
    explicit LayoutManager(QObject *parent = nullptr);
    ~LayoutManager() override;

    // ILayoutManager接口实现
    bool initialize() override;
    void shutdown() override;
    LayoutStatus status() const override;

    // 布局管理
    bool setLayout(const QString& layoutName) override;
    bool setLayout(LayoutType layoutType) override;
    QString currentLayout() const override;
    LayoutType currentLayoutType() const override;

    // 布局查询
    QStringList availableLayouts() const override;
    QStringList supportedLayoutTypes() const override;
    bool hasLayout(const QString& layoutName) const override;
    bool isLayoutSupported(LayoutType layoutType) const override;

    // 布局应用
    bool applyLayout(QWidget* widget) override;
    bool applyLayoutToWindow(QWidget* window) override;
    bool updateLayout() override;
    bool refreshLayout() override;

    // 响应式设计
    bool setResponsiveMode(bool enabled) override;
    bool isResponsiveModeEnabled() const override;
    bool adaptToSize(const QSize& size) override;
    bool adaptToGeometry(const QRect& geometry) override;

    // 布局配置
    bool setLayoutProperty(const QString& property, const QVariant& value) override;
    QVariant getLayoutProperty(const QString& property) const override;
    bool applyLayoutConfiguration(const QVariantMap& config) override;
    QVariantMap getLayoutConfiguration() const override;

    // 布局组件
    bool addLayoutComponent(const QString& name, QWidget* widget) override;
    bool removeLayoutComponent(const QString& name) override;
    QWidget* getLayoutComponent(const QString& name) const override;
    QStringList getLayoutComponents() const override;

    // 布局约束
    bool setLayoutConstraints(const QString& componentName, const QVariantMap& constraints) override;
    QVariantMap getLayoutConstraints(const QString& componentName) const override;
    bool validateLayoutConstraints() const override;

    // 布局信息
    QString getLayoutDisplayName(const QString& layoutName) const override;
    QString getLayoutDescription(const QString& layoutName) const override;
    QVariantMap getLayoutMetadata(const QString& layoutName) const override;

    // 其他方法

    bool applyLayoutToWidget(const QString& layoutName, QWidget* widget);
    bool removeLayoutFromWidget(QWidget* widget);

    std::shared_ptr<BaseLayout> getLayout(const QString& layoutName) const;
    bool registerLayout(const QString& layoutName, std::shared_ptr<BaseLayout> layout);
    bool unregisterLayout(const QString& layoutName);

    bool isLayoutRegistered(const QString& layoutName) const;
    QStringList registeredLayouts() const;

    bool setLayoutProperty(const QString& layoutName, const QString& property, const QVariant& value);
    QVariant getLayoutProperty(const QString& layoutName, const QString& property) const;

    bool saveLayoutConfiguration(const QString& layoutName);
    bool loadLayoutConfiguration(const QString& layoutName);

    bool validateLayout(const QString& layoutName) const;
    QStringList getLayoutValidationErrors(const QString& layoutName) const;

signals:
    // ILayoutManager 接口信号
    void layoutChanged(const QString& layoutName);
    void layoutApplied(const QString& layoutName);
    void layoutUpdated();
    void responsiveModeChanged(bool enabled);
    void sizeAdapted(const QSize& size);
    void componentAdded(const QString& name);
    void componentRemoved(const QString& name);
    void constraintsChanged(const QString& componentName);
    void errorOccurred(const QString& error);
    
    // 额外的信号
    void layoutAppliedToWidget(const QString& layoutName, QWidget* widget);
    void layoutRemovedFromWidget(QWidget* widget);
    void layoutRegistered(const QString& layoutName);
    void layoutUnregistered(const QString& layoutName);
    void layoutPropertyChanged(const QString& layoutName, const QString& property);

private slots:
    void onLayoutError(const QString& error);

private:
    void setupDefaultLayouts();
    void connectLayoutSignals(std::shared_ptr<BaseLayout> layout);
    void disconnectLayoutSignals(std::shared_ptr<BaseLayout> layout);
    bool validateLayoutName(const QString& layoutName) const;

    LayoutStatus m_status;
    QString m_currentLayoutName;
    LayoutType m_currentLayoutType;
    bool m_responsiveModeEnabled;
    
    QMap<QString, std::shared_ptr<BaseLayout>> m_registeredLayouts;
    QMap<QWidget*, std::shared_ptr<BaseLayout>> m_appliedLayouts;
    QMap<QString, QWidget*> m_layoutComponents;
    QMap<QString, QVariantMap> m_layoutConstraints;
    QVariantMap m_layoutConfiguration;
    
    // 默认布局实例
    std::shared_ptr<class MainLayout> m_mainLayout;
    std::shared_ptr<class ConferenceLayout> m_conferenceLayout;
    std::shared_ptr<class SettingsLayout> m_settingsLayout;
};

#endif // LAYOUTMANAGER_H