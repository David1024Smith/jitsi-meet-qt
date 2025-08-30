#ifndef BASELAYOUT_H
#define BASELAYOUT_H

#include <QObject>
#include <QString>
#include <QSize>
#include <QVariantMap>
#include <memory>

class BaseTheme;

/**
 * @brief 布局基类
 * 
 * BaseLayout定义了所有布局管理器必须实现的基础接口，
 * 包括布局应用、配置管理和主题支持。
 */
class BaseLayout : public QObject
{
    Q_OBJECT

public:
    explicit BaseLayout(QObject *parent = nullptr);
    virtual ~BaseLayout();

    // 布局信息
    virtual QString layoutName() const = 0;
    virtual QString layoutDisplayName() const = 0;
    virtual QString layoutDescription() const = 0;
    virtual QString layoutVersion() const;

    // 布局生命周期
    virtual bool initialize() = 0;
    virtual bool apply(QWidget* widget) = 0;
    virtual void cleanup() = 0;
    virtual bool isInitialized() const;
    virtual bool isApplied() const;

    // 布局配置
    virtual QVariantMap getLayoutConfiguration() const = 0;
    virtual void setLayoutConfiguration(const QVariantMap& config) = 0;
    virtual void resetConfiguration();
    virtual bool validateConfiguration(const QVariantMap& config) const;

    // 响应式设计
    virtual bool adaptToSize(const QSize& size) = 0;
    virtual bool isResponsive() const = 0;
    virtual void setResponsive(bool responsive) = 0;

    // 主题支持
    virtual void applyTheme(std::shared_ptr<BaseTheme> theme);
    virtual void refreshTheme();
    std::shared_ptr<BaseTheme> currentTheme() const;

    // 布局状态
    bool isEnabled() const;
    void setEnabled(bool enabled);
    bool isVisible() const;
    void setVisible(bool visible);

    // 布局验证
    virtual bool validate() const;
    virtual QStringList validationErrors() const;

signals:
    void layoutApplied();
    void layoutCleanedUp();
    void configurationChanged();
    void themeChanged();
    void sizeAdapted(const QSize& size);
    void enabledChanged(bool enabled);
    void visibleChanged(bool visible);
    void errorOccurred(const QString& error);

protected:
    // 主题相关虚函数
    virtual void onThemeChanged(std::shared_ptr<BaseTheme> theme);
    virtual void updateThemeColors();
    virtual void updateThemeFonts();
    virtual void updateThemeSizes();

    // 配置相关虚函数
    virtual void onConfigurationChanged(const QVariantMap& config);
    virtual QVariantMap getDefaultConfiguration() const;

    // 布局更新虚函数
    virtual void updateLayout();
    virtual void updateGeometry();
    virtual void updateSpacing();
    virtual void updateMargins();

    // 状态管理
    void setInitialized(bool initialized);
    void setApplied(bool applied);

private:
    void setupLayout();
    void connectThemeSignals();

    bool m_initialized;
    bool m_applied;
    bool m_enabled;
    bool m_visible;
    bool m_responsive;
    
    std::shared_ptr<BaseTheme> m_currentTheme;
    QVariantMap m_configuration;
};

#endif // BASELAYOUT_H