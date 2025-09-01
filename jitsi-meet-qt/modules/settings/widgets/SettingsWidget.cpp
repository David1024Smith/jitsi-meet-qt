#include "SettingsWidget.h"
#include "../include/PreferencesHandler.h"
#include "../include/SettingsManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QLabel>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QShowEvent>
#include <QVariant>
#include <QStringList>

class SettingsWidget::Private
{
public:
    QLineEdit* searchEdit;
    QTreeWidget* settingsTree;
    QPushButton* expandAllButton;
    QPushButton* collapseAllButton;
    QPushButton* helpButton;
    QSplitter* splitter;
    QString currentSearchText;
    PreferencesHandler* preferencesHandler;
    SettingsManager* settingsManager;
    
    Private() : preferencesHandler(nullptr), settingsManager(nullptr) {}
};

SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent)
    , d(new Private)
{
    setupUI();
    connectSignals();
}

SettingsWidget::~SettingsWidget() = default;

void SettingsWidget::setSearchText(const QString& text)
{
    if (d->currentSearchText != text) {
        d->currentSearchText = text;
        d->searchEdit->setText(text);
        filterSettings(text);
    }
}

void SettingsWidget::expandAll()
{
    d->settingsTree->expandAll();
}

void SettingsWidget::collapseAll()
{
    d->settingsTree->collapseAll();
}

void SettingsWidget::showHelp()
{
    // Show help dialog or documentation
}

void SettingsWidget::onSearchTextChanged(const QString& text)
{
    d->currentSearchText = text;
    filterSettings(text);
}

void SettingsWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    
    // Search bar
    auto* searchLayout = new QHBoxLayout;
    d->searchEdit = new QLineEdit;
    d->searchEdit->setPlaceholderText("Search settings...");
    searchLayout->addWidget(new QLabel("Search:"));
    searchLayout->addWidget(d->searchEdit);
    
    // Buttons
    auto* buttonLayout = new QHBoxLayout;
    d->expandAllButton = new QPushButton("Expand All");
    d->collapseAllButton = new QPushButton("Collapse All");
    d->helpButton = new QPushButton("Help");
    
    buttonLayout->addWidget(d->expandAllButton);
    buttonLayout->addWidget(d->collapseAllButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(d->helpButton);
    
    // Settings tree
    d->settingsTree = new QTreeWidget;
    d->settingsTree->setHeaderLabel("Settings");
    
    // Splitter
    d->splitter = new QSplitter(Qt::Horizontal);
    d->splitter->addWidget(d->settingsTree);
    
    layout->addLayout(searchLayout);
    layout->addLayout(buttonLayout);
    layout->addWidget(d->splitter);
}

void SettingsWidget::connectSignals()
{
    connect(d->searchEdit, &QLineEdit::textChanged,
            this, &SettingsWidget::onSearchTextChanged);
    connect(d->expandAllButton, &QPushButton::clicked,
            this, &SettingsWidget::expandAll);
    connect(d->collapseAllButton, &QPushButton::clicked,
            this, &SettingsWidget::collapseAll);
    connect(d->helpButton, &QPushButton::clicked,
            this, &SettingsWidget::showHelp);
}

void SettingsWidget::filterSettings(const QString& text)
{
    // Filter settings based on search text
    Q_UNUSED(text)
    // Implementation would filter the tree widget items
}

void SettingsWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void SettingsWidget::closeEvent(QCloseEvent* event)
{
    // Handle close event
    event->accept();
}

void SettingsWidget::showEvent(QShowEvent* event)
{
    // Handle show event
    QWidget::showEvent(event);
}

void SettingsWidget::changeEvent(QEvent* event)
{
    // Handle change events (like language changes)
    QWidget::changeEvent(event);
}

// Public slots implementation
void SettingsWidget::refresh()
{
    /**
     * @brief 刷新设置界面，重新加载所有设置值
     */
    // 重新加载设置值
    if (d->settingsTree) {
        // 遍历所有设置项并更新显示值
        for (int i = 0; i < d->settingsTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem* item = d->settingsTree->topLevelItem(i);
            updateTreeItemValues(item);
        }
    }
    
    // 清除搜索文本
    if (d->searchEdit) {
        d->searchEdit->clear();
        d->currentSearchText.clear();
    }
    
    // 发送刷新完成信号
    emit settingsLoaded(true);
}

void SettingsWidget::applySettings()
{
    /**
     * @brief 应用当前设置
     */
    // 验证设置
    if (!validateSettings()) {
        emit settingsSaved(false);
        return;
    }
    
    // 保存设置
    bool success = saveSettings();
    emit settingsSaved(success);
}

void SettingsWidget::cancelChanges()
{
    /**
     * @brief 取消更改，恢复到上次保存的状态
     */
    // 重新加载设置
    loadSettings();
    refresh();
}

void SettingsWidget::restoreDefaults()
{
    /**
     * @brief 恢复所有设置为默认值
     */
    // 重置所有设置为默认值
    resetAll();
    
    // 刷新界面
    refresh();
    
    // 发送设置变化信号
    emit settingChanged(QString(), QVariant());
}

void SettingsWidget::addSetting(const SettingDescriptor& descriptor)
{
    /**
     * @brief 添加设置项
     * @param descriptor 设置描述符
     */
    Q_UNUSED(descriptor)
    // 这里应该实现添加设置项的逻辑
    // 目前为空实现
}

QString SettingsWidget::currentCategory() const
{
    /**
     * @brief 获取当前选中的类别
     */
    if (d->settingsTree && d->settingsTree->currentItem()) {
        QTreeWidgetItem* item = d->settingsTree->currentItem();
        // 如果是子项，返回父项的文本
        if (item->parent()) {
            return item->parent()->text(0);
        }
        return item->text(0);
    }
    return QString();
}

void SettingsWidget::setCurrentCategory(const QString& category)
{
    /**
     * @brief 设置当前类别
     */
    if (!d->settingsTree) {
        return;
    }
    
    // 查找对应的类别项
    for (int i = 0; i < d->settingsTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = d->settingsTree->topLevelItem(i);
        if (item->text(0) == category) {
            d->settingsTree->setCurrentItem(item);
            emit currentCategoryChanged(category);
            break;
        }
    }
}

bool SettingsWidget::validateSettings()
{
    /**
     * @brief 验证所有设置
     */
    QStringList errors;
    
    // 这里应该实现具体的验证逻辑
    // 目前返回true表示验证通过
    
    bool isValid = errors.isEmpty();
    emit validationCompleted(isValid, errors);
    return isValid;
}

QStringList SettingsWidget::validationErrors() const
{
    /**
     * @brief 获取验证错误列表
     */
    // 返回空列表，表示没有错误
    return QStringList();
}

bool SettingsWidget::saveSettings()
{
    /**
     * @brief 保存设置
     */
    // 这里应该实现具体的保存逻辑
    // 目前返回true表示保存成功
    return true;
}

bool SettingsWidget::loadSettings()
{
    /**
     * @brief 加载设置
     */
    // 这里应该实现具体的加载逻辑
    // 目前返回true表示加载成功
    return true;
}

bool SettingsWidget::hasUnsavedChanges() const
{
    /**
     * @brief 检查是否有未保存的更改
     */
    // 这里应该实现具体的检查逻辑
    // 目前返回false表示没有未保存的更改
    return false;
}

void SettingsWidget::resetAll()
{
    /**
     * @brief 重置所有设置为默认值
     */
    // 这里应该实现具体的重置逻辑
}

// Private slots implementation
void SettingsWidget::onSettingValueChanged()
{
    /**
     * @brief 处理设置值变化
     */
    QObject* sender = this->sender();
    if (!sender) {
        return;
    }
    
    // 获取设置键和新值
    QString key = sender->property("settingKey").toString();
    QVariant value = getWidgetValue(key);
    
    // 发送设置变化信号
    emit settingChanged(key, value);
}

void SettingsWidget::onSettingValueChanged(const QString& key, const QVariant& value)
{
    /**
     * @brief 处理设置值变化（重载版本）
     * @param key 设置键
     * @param value 新值
     */
    // 发送设置变化信号
    emit settingChanged(key, value);
    
    // 刷新相关的界面元素
    refresh();
}

void SettingsWidget::onCategorySelectionChanged()
{
    /**
     * @brief 处理类别选择变化
     */
    QString category = currentCategory();
    if (!category.isEmpty()) {
        emit currentCategoryChanged(category);
    }
}

void SettingsWidget::onResetButtonClicked()
{
    /**
     * @brief 处理重置按钮点击
     */
    restoreDefaults();
}

void SettingsWidget::onApplyButtonClicked()
{
    /**
     * @brief 处理应用按钮点击
     */
    applySettings();
}

void SettingsWidget::onCancelButtonClicked()
{
    /**
     * @brief 处理取消按钮点击
     */
    cancelChanges();
}

// Helper methods
void SettingsWidget::updateTreeItemValues(QTreeWidgetItem* item)
{
    /**
     * @brief 更新树项的值显示
     */
    if (!item) {
        return;
    }
    
    // 更新子项
    for (int i = 0; i < item->childCount(); ++i) {
        updateTreeItemValues(item->child(i));
    }
}

QVariant SettingsWidget::getWidgetValue(const QString& key) const
{
    /**
     * @brief 从控件获取设置值
     */
    Q_UNUSED(key)
    // 这里应该实现具体的获取逻辑
    return QVariant();
}

bool SettingsWidget::isAutoSaveEnabled() const
{
    /**
     * @brief 检查是否启用自动保存
     * @return 是否启用自动保存
     */
    return true; // 默认启用
}

void SettingsWidget::setAutoSaveEnabled(bool enabled)
{
    /**
     * @brief 启用/禁用自动保存
     * @param enabled 是否启用
     */
    Q_UNUSED(enabled)
    emit autoSaveChanged(enabled);
}

bool SettingsWidget::isShowAdvancedEnabled() const
{
    /**
     * @brief 检查是否显示高级设置
     * @return 是否显示高级设置
     */
    return false; // 默认不显示
}

void SettingsWidget::setShowAdvancedEnabled(bool enabled)
{
    /**
     * @brief 启用/禁用显示高级设置
     * @param enabled 是否启用
     */
    Q_UNUSED(enabled)
    emit showAdvancedChanged(enabled);
}

SettingsWidget::ViewMode SettingsWidget::viewMode() const
{
    /**
     * @brief 获取当前视图模式
     * @return 视图模式
     */
    return TabView; // 默认标签页视图
}

void SettingsWidget::setViewMode(ViewMode mode)
{
    /**
     * @brief 设置视图模式
     * @param mode 视图模式
     */
    Q_UNUSED(mode)
    emit viewModeChanged(mode);
}

void SettingsWidget::setPreferencesHandler(PreferencesHandler* handler)
{
    /**
     * @brief 设置偏好处理器
     * @param handler 偏好处理器实例
     */
    d->preferencesHandler = handler;
    
    // 如果有新的处理器，连接信号
    if (d->preferencesHandler) {
        connect(d->preferencesHandler, &PreferencesHandler::preferenceChanged,
                this, &SettingsWidget::onPreferenceChanged);
    }
}

PreferencesHandler* SettingsWidget::preferencesHandler() const
{
    /**
     * @brief 获取偏好处理器
     * @return 偏好处理器实例
     */
    return d->preferencesHandler;
}

void SettingsWidget::setSettingsManager(SettingsManager* manager)
{
    /**
     * @brief 设置设置管理器
     * @param manager 设置管理器实例
     */
    d->settingsManager = manager;
    
    // 如果有新的管理器，连接信号
    if (d->settingsManager) {
        connect(d->settingsManager, QOverload<const QString&, const QVariant&, ISettingsManager::SettingsScope>::of(&SettingsManager::valueChanged),
                this, [this](const QString& key, const QVariant& value, ISettingsManager::SettingsScope scope) {
                    Q_UNUSED(scope)
                    onSettingValueChanged(key, value);
                });
    }
}

SettingsManager* SettingsWidget::settingsManager() const
{
    /**
     * @brief 获取设置管理器
     * @return 设置管理器实例
     */
    return d->settingsManager;
}

void SettingsWidget::onPreferenceChanged(const QString& category, const QString& key, const QVariant& value)
{
    /**
     * @brief 处理偏好设置变化
     * @param category 类别
     * @param key 键
     * @param value 值
     */
    Q_UNUSED(category)
    Q_UNUSED(key)
    Q_UNUSED(value)
    
    // 刷新界面以反映变化
    refresh();
}