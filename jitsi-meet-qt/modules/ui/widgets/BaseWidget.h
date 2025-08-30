#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <QWidget>
#include <QString>
#include <QVariantMap>
#include <memory>

class BaseTheme;

/**
 * @brief 基础组件类
 * 
 * BaseWidget是所有UI组件的基类，提供统一的主题支持、
 * 配置管理和基础功能。
 */
class BaseWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString themeName READ themeName WRITE setThemeName NOTIFY themeNameChanged)
    Q_PROPERTY(bool themeEnabled READ isThemeEnabled WRITE setThemeEnabled NOTIFY themeEnabledChanged)

public:
    explicit BaseWidget(QWidget *parent = nullptr);
    ~BaseWidget() override;

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

signals:
    void themeNameChanged(const QString& themeName);
    void themeEnabledChanged(bool enabled);
    void themeApplied();
    void configurationChanged();
    void styleSheetChanged();
    void validationFailed(const QStringList& errors);

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

    // 事件处理
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void onGlobalThemeChanged();

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