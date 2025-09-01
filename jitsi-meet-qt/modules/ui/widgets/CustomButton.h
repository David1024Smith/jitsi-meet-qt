#ifndef CUSTOMBUTTON_H
#define CUSTOMBUTTON_H

#include "BaseWidget.h"
#include <QPushButton>
#include <QIcon>
#include <QSize>
#include <QEnterEvent>

/**
 * @brief 自定义按钮组件
 * 
 * CustomButton继承自QPushButton和BaseWidget，提供增强的
 * 按钮功能，包括多种样式、图标支持和主题集成。
 */
class CustomButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(ButtonStyle buttonStyle READ buttonStyle WRITE setButtonStyle NOTIFY buttonStyleChanged)
    Q_PROPERTY(ButtonSize buttonSize READ buttonSize WRITE setButtonSize NOTIFY buttonSizeChanged)
    Q_PROPERTY(bool iconVisible READ isIconVisible WRITE setIconVisible NOTIFY iconVisibleChanged)
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)

public:
    enum ButtonStyle {
        DefaultStyle,
        PrimaryStyle,
        SecondaryStyle,
        SuccessStyle,
        WarningStyle,
        DangerStyle,
        InfoStyle,
        LinkStyle,
        OutlineStyle
    };
    Q_ENUM(ButtonStyle)

    enum ButtonSize {
        SmallSize,
        MediumSize,
        LargeSize,
        ExtraLargeSize
    };
    Q_ENUM(ButtonSize)

    explicit CustomButton(QWidget *parent = nullptr);
    explicit CustomButton(const QString& text, QWidget *parent = nullptr);
    explicit CustomButton(const QIcon& icon, const QString& text, QWidget *parent = nullptr);
    ~CustomButton() override;

    // 按钮样式
    ButtonStyle buttonStyle() const;
    void setButtonStyle(ButtonStyle style);

    // 按钮尺寸
    ButtonSize buttonSize() const;
    void setButtonSize(ButtonSize size);

    // 图标管理
    bool isIconVisible() const;
    void setIconVisible(bool visible);
    QString iconName() const;
    void setIconName(const QString& iconName);
    void setIconFromTheme(const QString& iconName);

    // 主题支持 (从 BaseWidget 接口实现)
    void applyTheme(std::shared_ptr<BaseTheme> theme);

    // 配置管理 (从 BaseWidget 接口实现)
    QVariantMap getConfiguration() const;
    void setConfiguration(const QVariantMap& config);

    // 组件信息 (从 BaseWidget 接口实现)
    QString componentName() const;

signals:
    void buttonStyleChanged(ButtonStyle style);
    void buttonSizeChanged(ButtonSize size);
    void iconVisibleChanged(bool visible);
    void iconNameChanged(const QString& iconName);

protected:
    // 主题相关 (从 BaseWidget 接口实现)
    void onThemeChanged(std::shared_ptr<BaseTheme> theme);
    QString getDefaultStyleSheet() const;
    void updateThemeColors();
    void updateThemeFonts();
    void updateThemeSizes();

    // 配置相关 (从 BaseWidget 接口实现)
    QVariantMap getDefaultConfiguration() const;
    bool validateConfiguration(const QVariantMap& config) const;

    // 事件处理
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onClicked();
    void onPressed();
    void onReleased();

private:
    void setupButton();
    void updateButtonStyle();
    void updateButtonSize();
    void updateIcon();
    QString generateStyleSheet() const;
    QString getStyleForButtonStyle(ButtonStyle style) const;
    QSize getSizeForButtonSize(ButtonSize size) const;
    QFont getFontForButtonSize(ButtonSize size) const;

    ButtonStyle m_buttonStyle;
    ButtonSize m_buttonSize;
    bool m_iconVisible;
    QString m_iconName;
    QIcon m_currentIcon;
    
    // 状态跟踪
    bool m_hovered;
    bool m_pressed;
    
    // BaseWidget 功能组合
    std::unique_ptr<BaseWidget> m_baseWidget;
};

#endif // CUSTOMBUTTON_H