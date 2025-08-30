#include "ToolBar.h"
#include "CustomButton.h"
#include "../themes/BaseTheme.h"
#include <QActionEvent>
#include <QDebug>

ToolBar::ToolBar(QWidget *parent)
    : QToolBar(parent)
    , m_toolBarStyle(IconAndTextStyle)
    , m_iconsVisible(true)
    , m_textVisible(true)
    , m_buttonSize(32)
{
    setupToolBar();
}

ToolBar::ToolBar(const QString& title, QWidget *parent)
    : QToolBar(title, parent)
    , m_toolBarStyle(IconAndTextStyle)
    , m_iconsVisible(true)
    , m_textVisible(true)
    , m_buttonSize(32)
{
    setupToolBar();
}

ToolBar::~ToolBar() = default;

ToolBar::ToolBarStyle ToolBar::toolBarStyle() const
{
    return m_toolBarStyle;
}

void ToolBar::setToolBarStyle(ToolBarStyle style)
{
    if (m_toolBarStyle != style) {
        m_toolBarStyle = style;
        updateToolBarStyle();
        emit toolBarStyleChanged(style);
    }
}

bool ToolBar::areIconsVisible() const
{
    return m_iconsVisible;
}

void ToolBar::setIconsVisible(bool visible)
{
    if (m_iconsVisible != visible) {
        m_iconsVisible = visible;
        updateButtonAppearance();
        emit iconsVisibleChanged(visible);
    }
}

bool ToolBar::isTextVisible() const
{
    return m_textVisible;
}

void ToolBar::setTextVisible(bool visible)
{
    if (m_textVisible != visible) {
        m_textVisible = visible;
        updateButtonAppearance();
        emit textVisibleChanged(visible);
    }
}

int ToolBar::buttonSize() const
{
    return m_buttonSize;
}

void ToolBar::setButtonSize(int size)
{
    if (m_buttonSize != size) {
        m_buttonSize = size;
        setIconSize(QSize(size, size));
        updateButtonAppearance();
        emit buttonSizeChanged(size);
    }
}

QAction* ToolBar::addAction(const QString& text)
{
    QAction* action = QToolBar::addAction(text);
    connect(action, &QAction::triggered, this, &ToolBar::onActionTriggered);
    updateActionAppearance(action);
    return action;
}

QAction* ToolBar::addAction(const QIcon& icon, const QString& text)
{
    QAction* action = QToolBar::addAction(icon, text);
    connect(action, &QAction::triggered, this, &ToolBar::onActionTriggered);
    updateActionAppearance(action);
    return action;
}

QAction* ToolBar::addAction(const QString& text, const QObject* receiver, const char* member)
{
    QAction* action = QToolBar::addAction(text, receiver, member);
    connect(action, &QAction::triggered, this, &ToolBar::onActionTriggered);
    updateActionAppearance(action);
    return action;
}

QAction* ToolBar::addAction(const QIcon& icon, const QString& text, const QObject* receiver, const char* member)
{
    QAction* action = QToolBar::addAction(icon, text, receiver, member);
    connect(action, &QAction::triggered, this, &ToolBar::onActionTriggered);
    updateActionAppearance(action);
    return action;
}

CustomButton* ToolBar::addCustomButton(const QString& text)
{
    CustomButton* button = new CustomButton(text, this);
    addWidget(button);
    m_customButtons.append(button);
    
    connect(button, &CustomButton::clicked, this, &ToolBar::onCustomButtonClicked);
    updateCustomButtonAppearance(button);
    
    emit customButtonAdded(button);
    return button;
}

CustomButton* ToolBar::addCustomButton(const QIcon& icon, const QString& text)
{
    CustomButton* button = new CustomButton(icon, text, this);
    addWidget(button);
    m_customButtons.append(button);
    
    connect(button, &CustomButton::clicked, this, &ToolBar::onCustomButtonClicked);
    updateCustomButtonAppearance(button);
    
    emit customButtonAdded(button);
    return button;
}

void ToolBar::removeCustomButton(CustomButton* button)
{
    if (m_customButtons.contains(button)) {
        m_customButtons.removeOne(button);
        removeWidget(button);
        button->deleteLater();
        emit customButtonRemoved(button);
    }
}

void ToolBar::addActionGroup(const QString& groupName, const QList<QAction*>& actions)
{
    if (m_actionGroups.contains(groupName)) {
        removeActionGroup(groupName);
    }
    
    m_actionGroups[groupName] = actions;
    
    QButtonGroup* buttonGroup = new QButtonGroup(this);
    m_buttonGroups[groupName] = buttonGroup;
    
    for (QAction* action : actions) {
        QWidget* widget = widgetForAction(action);
        if (QAbstractButton* button = qobject_cast<QAbstractButton*>(widget)) {
            buttonGroup->addButton(button);
        }
    }
    
    emit actionGroupAdded(groupName);
}

void ToolBar::removeActionGroup(const QString& groupName)
{
    if (m_actionGroups.contains(groupName)) {
        m_actionGroups.remove(groupName);
        
        if (m_buttonGroups.contains(groupName)) {
            delete m_buttonGroups[groupName];
            m_buttonGroups.remove(groupName);
        }
        
        emit actionGroupRemoved(groupName);
    }
}

QStringList ToolBar::actionGroups() const
{
    return m_actionGroups.keys();
}

QList<QAction*> ToolBar::getActionGroup(const QString& groupName) const
{
    return m_actionGroups.value(groupName);
}

QAction* ToolBar::addSeparator()
{
    return QToolBar::addSeparator();
}

QAction* ToolBar::addSeparator(const QString& name)
{
    QAction* separator = QToolBar::addSeparator();
    m_separators[name] = separator;
    return separator;
}

void ToolBar::removeSeparator(const QString& name)
{
    if (m_separators.contains(name)) {
        QAction* separator = m_separators[name];
        removeAction(separator);
        m_separators.remove(name);
        delete separator;
    }
}

void ToolBar::setActionsEnabled(bool enabled)
{
    for (QAction* action : actions()) {
        action->setEnabled(enabled);
    }
    
    for (CustomButton* button : m_customButtons) {
        button->setEnabled(enabled);
    }
}

void ToolBar::setActionGroupEnabled(const QString& groupName, bool enabled)
{
    if (m_actionGroups.contains(groupName)) {
        const QList<QAction*>& actions = m_actionGroups[groupName];
        for (QAction* action : actions) {
            action->setEnabled(enabled);
        }
    }
}

void ToolBar::setActionEnabled(const QString& actionName, bool enabled)
{
    for (QAction* action : actions()) {
        if (action->text() == actionName || action->objectName() == actionName) {
            action->setEnabled(enabled);
            break;
        }
    }
}

void ToolBar::applyTheme(std::shared_ptr<BaseTheme> theme)
{
    if (!theme) {
        return;
    }

    m_currentTheme = theme;
    onThemeChanged(theme);
    updateThemeColors();
    updateThemeFonts();
    updateThemeSizes();
}

QVariantMap ToolBar::getConfiguration() const
{
    QVariantMap config = getDefaultConfiguration();
    config["toolBarStyle"] = static_cast<int>(m_toolBarStyle);
    config["iconsVisible"] = m_iconsVisible;
    config["textVisible"] = m_textVisible;
    config["buttonSize"] = m_buttonSize;
    config["windowTitle"] = windowTitle();
    return config;
}

void ToolBar::setConfiguration(const QVariantMap& config)
{
    if (!validateConfiguration(config)) {
        qWarning() << "Invalid configuration for ToolBar";
        return;
    }

    if (config.contains("toolBarStyle")) {
        setToolBarStyle(static_cast<ToolBarStyle>(config["toolBarStyle"].toInt()));
    }
    if (config.contains("iconsVisible")) {
        setIconsVisible(config["iconsVisible"].toBool());
    }
    if (config.contains("textVisible")) {
        setTextVisible(config["textVisible"].toBool());
    }
    if (config.contains("buttonSize")) {
        setButtonSize(config["buttonSize"].toInt());
    }
    if (config.contains("windowTitle")) {
        setWindowTitle(config["windowTitle"].toString());
    }
}

QString ToolBar::componentName() const
{
    return "ToolBar";
}

void ToolBar::onThemeChanged(std::shared_ptr<BaseTheme> theme)
{
    Q_UNUSED(theme)
    // 主题变化处理
}

QString ToolBar::getDefaultStyleSheet() const
{
    return "QToolBar { background-color: #F8F9FA; border: 1px solid #DEE2E6; spacing: 2px; }";
}

void ToolBar::updateThemeColors()
{
    setStyleSheet(getDefaultStyleSheet());
}

void ToolBar::updateThemeFonts()
{
    // 更新字体
}

void ToolBar::updateThemeSizes()
{
    setIconSize(QSize(m_buttonSize, m_buttonSize));
}

QVariantMap ToolBar::getDefaultConfiguration() const
{
    QVariantMap config;
    config["toolBarStyle"] = static_cast<int>(IconAndTextStyle);
    config["iconsVisible"] = true;
    config["textVisible"] = true;
    config["buttonSize"] = 32;
    config["windowTitle"] = QString();
    return config;
}

bool ToolBar::validateConfiguration(const QVariantMap& config) const
{
    if (config.contains("toolBarStyle")) {
        int style = config["toolBarStyle"].toInt();
        if (style < IconOnlyStyle || style > IconBesideTextStyle) {
            return false;
        }
    }
    
    if (config.contains("buttonSize")) {
        int size = config["buttonSize"].toInt();
        if (size < 16 || size > 64) {
            return false;
        }
    }
    
    return true;
}

void ToolBar::actionEvent(QActionEvent *event)
{
    QToolBar::actionEvent(event);
    
    if (event->type() == QEvent::ActionAdded) {
        updateActionAppearance(event->action());
    }
}

void ToolBar::resizeEvent(QResizeEvent *event)
{
    QToolBar::resizeEvent(event);
    arrangeActions();
}

void ToolBar::onActionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        qDebug() << "Action triggered:" << action->text();
    }
}

void ToolBar::onCustomButtonClicked()
{
    CustomButton* button = qobject_cast<CustomButton*>(sender());
    if (button) {
        qDebug() << "Custom button clicked:" << button->text();
    }
}

void ToolBar::setupToolBar()
{
    setIconSize(QSize(m_buttonSize, m_buttonSize));
    setMovable(true);
    setFloatable(false);
    updateToolBarStyle();
}

void ToolBar::updateToolBarStyle()
{
    Qt::ToolButtonStyle qtStyle;
    
    switch (m_toolBarStyle) {
        case IconOnlyStyle:
            qtStyle = Qt::ToolButtonIconOnly;
            m_iconsVisible = true;
            m_textVisible = false;
            break;
        case TextOnlyStyle:
            qtStyle = Qt::ToolButtonTextOnly;
            m_iconsVisible = false;
            m_textVisible = true;
            break;
        case IconAboveTextStyle:
            qtStyle = Qt::ToolButtonTextUnderIcon;
            m_iconsVisible = true;
            m_textVisible = true;
            break;
        case IconBesideTextStyle:
            qtStyle = Qt::ToolButtonTextBesideIcon;
            m_iconsVisible = true;
            m_textVisible = true;
            break;
        default: // IconAndTextStyle
            qtStyle = Qt::ToolButtonTextUnderIcon;
            m_iconsVisible = true;
            m_textVisible = true;
            break;
    }
    
    setToolButtonStyle(qtStyle);
    updateButtonAppearance();
}

void ToolBar::updateButtonAppearance()
{
    // 更新所有动作的外观
    for (QAction* action : actions()) {
        updateActionAppearance(action);
    }
    
    // 更新所有自定义按钮的外观
    for (CustomButton* button : m_customButtons) {
        updateCustomButtonAppearance(button);
    }
}

void ToolBar::updateActionAppearance(QAction* action)
{
    if (!action) {
        return;
    }
    
    // 根据设置更新动作的图标和文本显示
    QWidget* widget = widgetForAction(action);
    if (QToolButton* toolButton = qobject_cast<QToolButton*>(widget)) {
        toolButton->setToolButtonStyle(toolButtonStyle());
    }
}

void ToolBar::updateCustomButtonAppearance(CustomButton* button)
{
    if (!button) {
        return;
    }
    
    // 更新自定义按钮的外观
    button->setIconVisible(m_iconsVisible);
    
    if (m_currentTheme) {
        button->applyTheme(m_currentTheme);
    }
}

void ToolBar::arrangeActions()
{
    // 重新排列动作
}