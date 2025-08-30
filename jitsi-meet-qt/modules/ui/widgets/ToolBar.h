#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "BaseWidget.h"
#include <QToolBar>
#include <QAction>
#include <QToolButton>
#include <QButtonGroup>

class CustomButton;

/**
 * @brief 自定义工具栏组件
 * 
 * ToolBar继承自QToolBar，提供增强的工具栏功能，
 * 包括自定义按钮、分组管理和主题集成。
 */
class ToolBar : public QToolBar
{
    Q_OBJECT
    Q_PROPERTY(ToolBarStyle toolBarStyle READ toolBarStyle WRITE setToolBarStyle NOTIFY toolBarStyleChanged)
    Q_PROPERTY(bool iconsVisible READ areIconsVisible WRITE setIconsVisible NOTIFY iconsVisibleChanged)
    Q_PROPERTY(bool textVisible READ isTextVisible WRITE setTextVisible NOTIFY textVisibleChanged)
    Q_PROPERTY(int buttonSize READ buttonSize WRITE setButtonSize NOTIFY buttonSizeChanged)

public:
    enum ToolBarStyle {
        IconOnlyStyle,
        TextOnlyStyle,
        IconAndTextStyle,
        IconAboveTextStyle,
        IconBesideTextStyle
    };
    Q_ENUM(ToolBarStyle)

    explicit ToolBar(QWidget *parent = nullptr);
    explicit ToolBar(const QString& title, QWidget *parent = nullptr);
    ~ToolBar() override;

    // 工具栏样式
    ToolBarStyle toolBarStyle() const;
    void setToolBarStyle(ToolBarStyle style);

    // 图标和文本显示
    bool areIconsVisible() const;
    void setIconsVisible(bool visible);
    bool isTextVisible() const;
    void setTextVisible(bool visible);

    // 按钮尺寸
    int buttonSize() const;
    void setButtonSize(int size);

    // 动作管理
    QAction* addAction(const QString& text);
    QAction* addAction(const QIcon& icon, const QString& text);
    QAction* addAction(const QString& text, const QObject* receiver, const char* member);
    QAction* addAction(const QIcon& icon, const QString& text, const QObject* receiver, const char* member);
    
    // 自定义按钮
    CustomButton* addCustomButton(const QString& text);
    CustomButton* addCustomButton(const QIcon& icon, const QString& text);
    void removeCustomButton(CustomButton* button);

    // 分组管理
    void addActionGroup(const QString& groupName, const QList<QAction*>& actions);
    void removeActionGroup(const QString& groupName);
    QStringList actionGroups() const;
    QList<QAction*> getActionGroup(const QString& groupName) const;

    // 分隔符管理
    QAction* addSeparator();
    QAction* addSeparator(const QString& name);
    void removeSeparator(const QString& name);

    // 工具栏状态
    void setActionsEnabled(bool enabled);
    void setActionGroupEnabled(const QString& groupName, bool enabled);
    void setActionEnabled(const QString& actionName, bool enabled);

    // 主题支持
    void applyTheme(std::shared_ptr<BaseTheme> theme);

    // 配置管理
    QVariantMap getConfiguration() const;
    void setConfiguration(const QVariantMap& config);

    // 组件信息
    QString componentName() const;

signals:
    void toolBarStyleChanged(ToolBarStyle style);
    void iconsVisibleChanged(bool visible);
    void textVisibleChanged(bool visible);
    void buttonSizeChanged(int size);
    void actionGroupAdded(const QString& groupName);
    void actionGroupRemoved(const QString& groupName);
    void customButtonAdded(CustomButton* button);
    void customButtonRemoved(CustomButton* button);

protected:
    // 主题相关
    void onThemeChanged(std::shared_ptr<BaseTheme> theme);
    QString getDefaultStyleSheet() const;
    void updateThemeColors();
    void updateThemeFonts();
    void updateThemeSizes();

    // 配置相关
    QVariantMap getDefaultConfiguration() const;
    bool validateConfiguration(const QVariantMap& config) const;

    // 事件处理
    void actionEvent(QActionEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onActionTriggered();
    void onCustomButtonClicked();

private:
    void setupToolBar();
    void updateToolBarStyle();
    void updateButtonAppearance();
    void updateActionAppearance(QAction* action);
    void updateCustomButtonAppearance(CustomButton* button);
    void arrangeActions();

    ToolBarStyle m_toolBarStyle;
    bool m_iconsVisible;
    bool m_textVisible;
    int m_buttonSize;

    // 动作分组
    QMap<QString, QList<QAction*>> m_actionGroups;
    QMap<QString, QButtonGroup*> m_buttonGroups;

    // 自定义按钮
    QList<CustomButton*> m_customButtons;

    // 分隔符
    QMap<QString, QAction*> m_separators;

    std::shared_ptr<BaseTheme> m_currentTheme;
};

#endif // TOOLBAR_H