#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <QWidget>
#include <QString>
#include <QVariantMap>
#include <memory>

class BaseTheme;

/**
 * @brief 基础组件接口类
 * 
 * BaseWidget是所有UI组件的基础接口，提供统一的主题支持、
 * 配置管理和基础功能。不继承QWidget以避免多重继承问题。
 */
class BaseWidget
{
public:
    // 移除 Q_OBJECT 宏，因为不再继承 QObject
    // Q_PROPERTY 也需要移除，因为它们依赖于 QObject

public:
    BaseWidget();
    virtual ~BaseWidget();

    // 主题支持
    QString themeName() const;
    void setThemeName(const QString& themeName);
    bool isThemeEnabled() const;
    void setThemeEnabled(bool enabled);
    virtual void applyTheme(std::shared_ptr<BaseTheme> theme);
    virtual void refreshTheme();

    // 配置管理
    virtual QVariantMap getConfiguration() const;
    virtual void setConfiguration(const QVariantMap& config);
    virtual void resetConfiguration();

    // 样式管理
    virtual void setCustomStyleSheet(const QString& styleSheet);
    QString customStyleSheet() const;
    virtual void applyCustomStyle();

    // 组件状态
    virtual bool isConfigured() const;
    virtual bool validate() const;
    virtual QStringList validationErrors() const;

    // 组件信息
    virtual QString componentName() const;
    virtual QString componentVersion() const;
    virtual QString componentDescription() const;

    // 信号需要在具体的 QObject 子类中定义
    // signals:
    // void themeNameChanged(const QString& themeName);
    // void themeEnabledChanged(bool enabled);
    // void themeApplied();
    // void configurationChanged();
    // void styleSheetChanged();
    // void validationFailed(const QStringList& errors);

protected:
    // 主题相关虚函数
    virtual void onThemeChanged(std::shared_ptr<BaseTheme> theme);
    virtual QString getDefaultStyleSheet() const;
    virtual void updateThemeColors();
    virtual void updateThemeFonts();
    virtual void updateThemeSizes();

    // 配置相关虚函数
    virtual void onConfigurationChanged(const QVariantMap& config);
    virtual QVariantMap getDefaultConfiguration() const;
    virtual bool validateConfiguration(const QVariantMap& config) const;

    // 事件处理 - 这些需要在具体的 QWidget 子类中实现
    // virtual void paintEvent(QPaintEvent *event);
    // virtual void resizeEvent(QResizeEvent *event);
    // virtual void changeEvent(QEvent *event);

    // 槽函数需要在具体的 QObject 子类中定义
    // private slots:
    // void onGlobalThemeChanged();

private:
    void setupWidget();
    void connectSignals();
    void applyDefaultConfiguration();

    QString m_themeName;
    bool m_themeEnabled;
    QString m_customStyleSheet;
    QVariantMap m_configuration;
    std::shared_ptr<BaseTheme> m_currentTheme;
};

#endif // BASEWIDGET_H