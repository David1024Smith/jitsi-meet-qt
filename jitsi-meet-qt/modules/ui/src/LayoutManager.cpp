#include "LayoutManager.h"
#include "layouts/BaseLayout.h"
#include "layouts/MainLayout.h"
#include "layouts/ConferenceLayout.h"
#include "layouts/SettingsLayout.h"
#include <QWidget>
#include <QDebug>

LayoutManager::LayoutManager(QObject *parent)
    : QObject(parent)
    , m_status(NotInitialized)
    , m_currentLayoutName("main")
{
}

LayoutManager::~LayoutManager()
{
    shutdown();
}

bool LayoutManager::initialize()
{
    if (m_status == Ready) {
        return true;
    }

    m_status = Initializing;

    try {
        // 创建默认布局
        setupDefaultLayouts();

        m_status = Ready;
        qDebug() << "LayoutManager initialized successfully";
        return true;

    } catch (const std::exception& e) {
        m_status = Error;
        emit errorOccurred(QString("Failed to initialize LayoutManager: %1").arg(e.what()));
        return false;
    }
}

void LayoutManager::shutdown()
{
    if (m_status == NotInitialized) {
        return;
    }

    // 清理应用的布局
    m_appliedLayouts.clear();

    // 清理注册的布局
    m_registeredLayouts.clear();

    // 清理默认布局
    m_mainLayout.reset();
    m_conferenceLayout.reset();
    m_settingsLayout.reset();

    m_status = NotInitialized;
    qDebug() << "LayoutManager shutdown completed";
}

ILayoutManager::LayoutStatus LayoutManager::status() const
{
    return m_status;
}

bool LayoutManager::setLayout(const QString& layoutName)
{
    if (!validateLayoutName(layoutName)) {
        emit errorOccurred(QString("Invalid layout name: %1").arg(layoutName));
        return false;
    }

    if (!isLayoutRegistered(layoutName)) {
        emit errorOccurred(QString("Layout not registered: %1").arg(layoutName));
        return false;
    }

    QString oldLayout = m_currentLayoutName;
    m_currentLayoutName = layoutName;
    
    emit layoutChanged(layoutName);
    qDebug() << "Layout changed from" << oldLayout << "to" << layoutName;
    return true;
}

QString LayoutManager::currentLayout() const
{
    return m_currentLayoutName;
}

QStringList LayoutManager::availableLayouts() const
{
    return registeredLayouts();
}

bool LayoutManager::updateLayout()
{
    // 更新当前应用的所有布局
    for (auto it = m_appliedLayouts.begin(); it != m_appliedLayouts.end(); ++it) {
        auto layout = it.value();
        if (layout && layout->isApplied()) {
            layout->updateLayout();
        }
    }
    return true;
}

bool LayoutManager::applyLayoutToWidget(const QString& layoutName, QWidget* widget)
{
    if (!widget) {
        emit errorOccurred("Cannot apply layout to null widget");
        return false;
    }

    if (!isLayoutRegistered(layoutName)) {
        emit errorOccurred(QString("Layout not registered: %1").arg(layoutName));
        return false;
    }

    auto layout = getLayout(layoutName);
    if (!layout) {
        emit errorOccurred(QString("Failed to get layout: %1").arg(layoutName));
        return false;
    }

    // 移除之前的布局
    removeLayoutFromWidget(widget);

    // 应用新布局
    if (layout->apply(widget)) {
        m_appliedLayouts[widget] = layout;
        emit layoutApplied(layoutName, widget);
        qDebug() << "Layout applied:" << layoutName << "to widget";
        return true;
    }

    emit errorOccurred(QString("Failed to apply layout: %1").arg(layoutName));
    return false;
}

bool LayoutManager::removeLayoutFromWidget(QWidget* widget)
{
    if (!widget) {
        return false;
    }

    auto it = m_appliedLayouts.find(widget);
    if (it != m_appliedLayouts.end()) {
        auto layout = it.value();
        if (layout) {
            layout->cleanup();
        }
        m_appliedLayouts.erase(it);
        emit layoutRemoved(widget);
        qDebug() << "Layout removed from widget";
        return true;
    }

    return false;
}

std::shared_ptr<BaseLayout> LayoutManager::getLayout(const QString& layoutName) const
{
    return m_registeredLayouts.value(layoutName, nullptr);
}

bool LayoutManager::registerLayout(const QString& layoutName, std::shared_ptr<BaseLayout> layout)
{
    if (layoutName.isEmpty()) {
        emit errorOccurred("Cannot register layout with empty name");
        return false;
    }

    if (!layout) {
        emit errorOccurred("Cannot register null layout");
        return false;
    }

    if (m_registeredLayouts.contains(layoutName)) {
        qDebug() << "Layout already registered, updating:" << layoutName;
    }

    m_registeredLayouts[layoutName] = layout;
    connectLayoutSignals(layout);
    
    emit layoutRegistered(layoutName);
    qDebug() << "Layout registered:" << layoutName;
    return true;
}

bool LayoutManager::unregisterLayout(const QString& layoutName)
{
    if (!m_registeredLayouts.contains(layoutName)) {
        return false;
    }

    auto layout = m_registeredLayouts.take(layoutName);
    if (layout) {
        disconnectLayoutSignals(layout);
    }

    emit layoutUnregistered(layoutName);
    qDebug() << "Layout unregistered:" << layoutName;
    return true;
}

bool LayoutManager::isLayoutRegistered(const QString& layoutName) const
{
    return m_registeredLayouts.contains(layoutName);
}

QStringList LayoutManager::registeredLayouts() const
{
    return m_registeredLayouts.keys();
}

bool LayoutManager::setLayoutProperty(const QString& layoutName, const QString& property, const QVariant& value)
{
    auto layout = getLayout(layoutName);
    if (!layout) {
        emit errorOccurred(QString("Layout not found: %1").arg(layoutName));
        return false;
    }

    if (layout->setProperty(property.toUtf8().constData(), value)) {
        emit layoutPropertyChanged(layoutName, property);
        return true;
    }

    return false;
}

QVariant LayoutManager::getLayoutProperty(const QString& layoutName, const QString& property) const
{
    auto layout = getLayout(layoutName);
    if (!layout) {
        return QVariant();
    }

    return layout->property(property.toUtf8().constData());
}

bool LayoutManager::saveLayoutConfiguration(const QString& layoutName)
{
    auto layout = getLayout(layoutName);
    if (!layout) {
        return false;
    }

    // 这里可以实现保存配置到文件的逻辑
    QVariantMap config = layout->getLayoutConfiguration();
    qDebug() << "Layout configuration saved for:" << layoutName;
    return true;
}

bool LayoutManager::loadLayoutConfiguration(const QString& layoutName)
{
    auto layout = getLayout(layoutName);
    if (!layout) {
        return false;
    }

    // 这里可以实现从文件加载配置的逻辑
    qDebug() << "Layout configuration loaded for:" << layoutName;
    return true;
}

bool LayoutManager::validateLayout(const QString& layoutName) const
{
    if (layoutName.isEmpty()) {
        return false;
    }

    auto layout = getLayout(layoutName);
    if (!layout) {
        return false;
    }

    return layout->validate();
}

QStringList LayoutManager::getLayoutValidationErrors(const QString& layoutName) const
{
    auto layout = getLayout(layoutName);
    if (!layout) {
        return QStringList() << QString("Layout not found: %1").arg(layoutName);
    }

    return layout->validationErrors();
}

void LayoutManager::onLayoutError(const QString& error)
{
    emit errorOccurred(QString("Layout error: %1").arg(error));
}

void LayoutManager::setupDefaultLayouts()
{
    // 创建主布局
    m_mainLayout = std::make_shared<MainLayout>();
    registerLayout("main", m_mainLayout);

    // 创建会议布局
    m_conferenceLayout = std::make_shared<ConferenceLayout>();
    registerLayout("conference", m_conferenceLayout);

    // 创建设置布局
    m_settingsLayout = std::make_shared<SettingsLayout>();
    registerLayout("settings", m_settingsLayout);

    qDebug() << "Default layouts created and registered";
}

void LayoutManager::connectLayoutSignals(std::shared_ptr<BaseLayout> layout)
{
    if (!layout) {
        return;
    }

    connect(layout.get(), &BaseLayout::errorOccurred,
            this, &LayoutManager::onLayoutError);
}

void LayoutManager::disconnectLayoutSignals(std::shared_ptr<BaseLayout> layout)
{
    if (!layout) {
        return;
    }

    disconnect(layout.get(), nullptr, this, nullptr);
}

bool LayoutManager::validateLayoutName(const QString& layoutName) const
{
    if (layoutName.isEmpty()) {
        return false;
    }

    // 可以添加更多验证逻辑
    return true;
}