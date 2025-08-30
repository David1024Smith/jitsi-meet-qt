#ifndef UICONFIG_H
#define UICONFIG_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QSize>
#include <QFont>
#include <QColor>

/**
 * @brief UI配置管理类
 * 
 * UIConfig负责管理UI模块的所有配置选项，包括主题设置、
 * 布局配置、字体设置、窗口状态等。
 */
class UIConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString layout READ layout WRITE setLayout NOTIFY layoutChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    Q_PROPERTY(bool darkMode READ isDarkMode WRITE setDarkMode NOTIFY darkModeChanged)

public:
    enum WindowState {
        Normal,
        Minimized,
        Maximized,
        FullScreen
    };
    Q_ENUM(WindowState)

    enum ScalingMode {
        NoScaling,
        AutoScaling,
        CustomScaling
    };
    Q_ENUM(ScalingMode)

    explicit UIConfig(QObject *parent = nullptr);
    UIConfig(const UIConfig& other);
    UIConfig& operator=(const UIConfig& other);
    ~UIConfig();

    // 主题配置
    QString theme() const;
    void setTheme(const QString& theme);
    QStringList availableThemes() const;
    void setAvailableThemes(const QStringList& themes);

    // 语言配置
    QString language() const;
    void setLanguage(const QString& language);
    QStringList availableLanguages() const;
    void setAvailableLanguages(const QStringList& languages);

    // 布局配置
    QString layout() const;
    void setLayout(const QString& layout);
    QStringList availableLayouts() const;
    void setAvailableLayouts(const QStringList& layouts);

    // 字体配置
    int fontSize() const;
    void setFontSize(int size);
    QString fontFamily() const;
    void setFontFamily(const QString& family);
    QFont font() const;
    void setFont(const QFont& font);

    // 颜色配置
    bool isDarkMode() const;
    void setDarkMode(bool enabled);
    QColor primaryColor() const;
    void setPrimaryColor(const QColor& color);
    QColor secondaryColor() const;
    void setSecondaryColor(const QColor& color);
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& color);

    // 窗口配置
    WindowState windowState() const;
    void setWindowState(WindowState state);
    QSize windowSize() const;
    void setWindowSize(const QSize& size);
    bool isWindowResizable() const;
    void setWindowResizable(bool resizable);

    // 缩放配置
    ScalingMode scalingMode() const;
    void setScalingMode(ScalingMode mode);
    qreal scalingFactor() const;
    void setScalingFactor(qreal factor);
    bool isHighDpiEnabled() const;
    void setHighDpiEnabled(bool enabled);

    // 自定义样式
    QString customStyleSheet() const;
    void setCustomStyleSheet(const QString& styleSheet);
    QVariantMap customProperties() const;
    void setCustomProperties(const QVariantMap& properties);
    void setCustomProperty(const QString& key, const QVariant& value);
    QVariant customProperty(const QString& key, const QVariant& defaultValue = QVariant()) const;

    // 动画配置
    bool isAnimationEnabled() const;
    void setAnimationEnabled(bool enabled);
    int animationDuration() const;
    void setAnimationDuration(int duration);
    QString animationEasing() const;
    void setAnimationEasing(const QString& easing);

    // 序列化
    QVariantMap toVariantMap() const;
    void fromVariantMap(const QVariantMap& map);
    QByteArray toJson() const;
    bool fromJson(const QByteArray& json);

    // 验证
    bool validate() const;
    QStringList validationErrors() const;
    bool isValid() const;

    // 默认配置
    void loadDefaults();
    void resetToDefaults();
    static UIConfig defaultConfig();

    // 配置比较
    bool operator==(const UIConfig& other) const;
    bool operator!=(const UIConfig& other) const;

signals:
    void themeChanged(const QString& theme);
    void languageChanged(const QString& language);
    void layoutChanged(const QString& layout);
    void fontSizeChanged(int size);
    void fontFamilyChanged(const QString& family);
    void fontChanged(const QFont& font);
    void darkModeChanged(bool enabled);
    void primaryColorChanged(const QColor& color);
    void secondaryColorChanged(const QColor& color);
    void backgroundColorChanged(const QColor& color);
    void windowStateChanged(WindowState state);
    void windowSizeChanged(const QSize& size);
    void scalingModeChanged(ScalingMode mode);
    void scalingFactorChanged(qreal factor);
    void customStyleSheetChanged(const QString& styleSheet);
    void customPropertyChanged(const QString& key, const QVariant& value);
    void animationEnabledChanged(bool enabled);
    void configurationChanged();

private:
    void setupDefaults();
    void connectSignals();
    bool validateTheme(const QString& theme) const;
    bool validateLanguage(const QString& language) const;
    bool validateLayout(const QString& layout) const;

    // 主题和外观
    QString m_theme;
    QString m_language;
    QString m_layout;
    QStringList m_availableThemes;
    QStringList m_availableLanguages;
    QStringList m_availableLayouts;

    // 字体和颜色
    QFont m_font;
    bool m_darkMode;
    QColor m_primaryColor;
    QColor m_secondaryColor;
    QColor m_backgroundColor;

    // 窗口设置
    WindowState m_windowState;
    QSize m_windowSize;
    bool m_windowResizable;

    // 缩放设置
    ScalingMode m_scalingMode;
    qreal m_scalingFactor;
    bool m_highDpiEnabled;

    // 自定义设置
    QString m_customStyleSheet;
    QVariantMap m_customProperties;

    // 动画设置
    bool m_animationEnabled;
    int m_animationDuration;
    QString m_animationEasing;
};

#endif // UICONFIG_H