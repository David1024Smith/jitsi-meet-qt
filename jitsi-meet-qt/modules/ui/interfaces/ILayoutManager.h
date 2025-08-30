#ifndef ILAYOUTMANAGER_H
#define ILAYOUTMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSize>
#include <QRect>
#include <QVariantMap>

class QWidget;
class QLayout;

/**
 * @brief 布局管理器接口
 * 
 * ILayoutManager定义了布局管理的标准接口，包括布局创建、
 * 应用、切换和响应式设计等功能。
 */
class ILayoutManager
{
public:
    enum LayoutType {
        MainLayout,
        ConferenceLayout,
        SettingsLayout,
        CustomLayout
    };

    enum LayoutStatus {
        NotInitialized,
        Initializing,
        Ready,
        Applying,
        Error
    };

    virtual ~ILayoutManager() = default;

    // 生命周期管理
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual LayoutStatus status() const = 0;

    // 布局管理
    virtual bool setLayout(const QString& layoutName) = 0;
    virtual bool setLayout(LayoutType layoutType) = 0;
    virtual QString currentLayout() const = 0;
    virtual LayoutType currentLayoutType() const = 0;

    // 布局查询
    virtual QStringList availableLayouts() const = 0;
    virtual QStringList supportedLayoutTypes() const = 0;
    virtual bool hasLayout(const QString& layoutName) const = 0;
    virtual bool isLayoutSupported(LayoutType layoutType) const = 0;

    // 布局应用
    virtual bool applyLayout(QWidget* widget) = 0;
    virtual bool applyLayoutToWindow(QWidget* window) = 0;
    virtual bool updateLayout() = 0;
    virtual bool refreshLayout() = 0;

    // 响应式设计
    virtual bool setResponsiveMode(bool enabled) = 0;
    virtual bool isResponsiveModeEnabled() const = 0;
    virtual bool adaptToSize(const QSize& size) = 0;
    virtual bool adaptToGeometry(const QRect& geometry) = 0;

    // 布局配置
    virtual bool setLayoutProperty(const QString& property, const QVariant& value) = 0;
    virtual QVariant getLayoutProperty(const QString& property) const = 0;
    virtual bool applyLayoutConfiguration(const QVariantMap& config) = 0;
    virtual QVariantMap getLayoutConfiguration() const = 0;

    // 布局组件
    virtual bool addLayoutComponent(const QString& name, QWidget* widget) = 0;
    virtual bool removeLayoutComponent(const QString& name) = 0;
    virtual QWidget* getLayoutComponent(const QString& name) const = 0;
    virtual QStringList getLayoutComponents() const = 0;

    // 布局约束
    virtual bool setLayoutConstraints(const QString& componentName, const QVariantMap& constraints) = 0;
    virtual QVariantMap getLayoutConstraints(const QString& componentName) const = 0;
    virtual bool validateLayoutConstraints() const = 0;

    // 布局信息
    virtual QString getLayoutDisplayName(const QString& layoutName) const = 0;
    virtual QString getLayoutDescription(const QString& layoutName) const = 0;
    virtual QVariantMap getLayoutMetadata(const QString& layoutName) const = 0;

    // 信号接口 (需要在实现类中定义为signals)
    virtual void layoutChanged(const QString& layoutName) = 0;
    virtual void layoutApplied(const QString& layoutName) = 0;
    virtual void layoutUpdated() = 0;
    virtual void responsiveModeChanged(bool enabled) = 0;
    virtual void sizeAdapted(const QSize& size) = 0;
    virtual void componentAdded(const QString& name) = 0;
    virtual void componentRemoved(const QString& name) = 0;
    virtual void constraintsChanged(const QString& componentName) = 0;
    virtual void errorOccurred(const QString& error) = 0;
};

Q_DECLARE_INTERFACE(ILayoutManager, "org.jitsi.LayoutManager/1.0")

#endif // ILAYOUTMANAGER_H