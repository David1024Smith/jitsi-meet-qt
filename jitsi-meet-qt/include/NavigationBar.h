#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>

class NavigationBar : public QWidget
{
    Q_OBJECT

public:
    enum ButtonType {
        SettingsButton = 0x01,
        AboutButton = 0x02,
        BackButton = 0x04
    };
    Q_DECLARE_FLAGS(ButtonTypes, ButtonType)

    explicit NavigationBar(QWidget *parent = nullptr);
    ~NavigationBar();

    // 配置导航栏显示的按钮
    void setButtonConfiguration(ButtonTypes buttons);
    
    // 设置标题文本
    void setTitle(const QString& title);
    
    // 获取按钮状态
    bool isButtonVisible(ButtonType button) const;

public slots:
    // 更新翻译文本
    void retranslateUi();

signals:
    void settingsClicked();
    void aboutClicked();
    void backClicked();

private slots:
    void onSettingsButtonClicked();
    void onAboutButtonClicked();
    void onBackButtonClicked();

private:
    void setupUI();
    void setupConnections();
    void updateButtonVisibility();
    void applyStyles();

    QHBoxLayout* m_layout;
    QPushButton* m_backButton;
    QLabel* m_titleLabel;
    QSpacerItem* m_spacer;
    QPushButton* m_settingsButton;
    QPushButton* m_aboutButton;
    
    ButtonTypes m_buttonConfiguration;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(NavigationBar::ButtonTypes)

#endif // NAVIGATIONBAR_H