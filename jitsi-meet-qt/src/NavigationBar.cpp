#include "../include/NavigationBar.h"
#include <QApplication>
#include <QStyle>

NavigationBar::NavigationBar(QWidget *parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_backButton(nullptr)
    , m_titleLabel(nullptr)
    , m_spacer(nullptr)
    , m_settingsButton(nullptr)
    , m_aboutButton(nullptr)
    , m_buttonConfiguration(SettingsButton | AboutButton)
{
    setupUI();
    setupConnections();
    applyStyles();
}

NavigationBar::~NavigationBar()
{
    // Qt handles cleanup automatically for child widgets
}

void NavigationBar::setupUI()
{
    // 创建主布局
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(10, 5, 10, 5);
    m_layout->setSpacing(10);

    // 创建返回按钮
    m_backButton = new QPushButton(this);
    m_backButton->setText(tr("back_button"));
    m_backButton->setObjectName("backButton");
    m_backButton->setVisible(false); // 默认隐藏
    
    // 创建标题标签
    m_titleLabel = new QLabel(this);
    m_titleLabel->setText("Jitsi Meet");
    m_titleLabel->setObjectName("titleLabel");
    
    // 创建弹性空间
    m_spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    // 创建设置按钮
    m_settingsButton = new QPushButton(this);
    m_settingsButton->setText(tr("settings_button"));
    m_settingsButton->setObjectName("settingsButton");
    
    // 创建关于按钮
    m_aboutButton = new QPushButton(this);
    m_aboutButton->setText(tr("about_button"));
    m_aboutButton->setObjectName("aboutButton");
    
    // 添加到布局
    m_layout->addWidget(m_backButton);
    m_layout->addWidget(m_titleLabel);
    m_layout->addItem(m_spacer);
    m_layout->addWidget(m_settingsButton);
    m_layout->addWidget(m_aboutButton);
    
    // 设置固定高度
    setFixedHeight(50);
    
    // 更新按钮可见性
    updateButtonVisibility();
}

void NavigationBar::setupConnections()
{
    connect(m_backButton, &QPushButton::clicked, this, &NavigationBar::onBackButtonClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &NavigationBar::onSettingsButtonClicked);
    connect(m_aboutButton, &QPushButton::clicked, this, &NavigationBar::onAboutButtonClicked);
}

void NavigationBar::applyStyles()
{
    // 设置导航栏样式
    setStyleSheet(
        "NavigationBar {"
        "    background-color: #f5f5f5;"
        "    border-bottom: 1px solid #ddd;"
        "}"
        
        "QPushButton {"
        "    background-color: #ffffff;"
        "    border: 1px solid #ccc;"
        "    border-radius: 4px;"
        "    padding: 6px 12px;"
        "    font-size: 12px;"
        "    min-width: 60px;"
        "}"
        
        "QPushButton:hover {"
        "    background-color: #e6e6e6;"
        "    border-color: #adadad;"
        "}"
        
        "QPushButton:pressed {"
        "    background-color: #d4d4d4;"
        "}"
        
        "QPushButton#backButton {"
        "    background-color: #007bff;"
        "    color: white;"
        "    border-color: #007bff;"
        "}"
        
        "QPushButton#backButton:hover {"
        "    background-color: #0056b3;"
        "    border-color: #0056b3;"
        "}"
        
        "QLabel#titleLabel {"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: #333;"
        "    margin-left: 10px;"
        "}"
    );
}

void NavigationBar::setButtonConfiguration(ButtonTypes buttons)
{
    m_buttonConfiguration = buttons;
    updateButtonVisibility();
}

void NavigationBar::setTitle(const QString& title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

bool NavigationBar::isButtonVisible(ButtonType button) const
{
    return m_buttonConfiguration.testFlag(button);
}

void NavigationBar::updateButtonVisibility()
{
    if (m_backButton) {
        m_backButton->setVisible(m_buttonConfiguration.testFlag(BackButton));
    }
    
    if (m_settingsButton) {
        m_settingsButton->setVisible(m_buttonConfiguration.testFlag(SettingsButton));
    }
    
    if (m_aboutButton) {
        m_aboutButton->setVisible(m_buttonConfiguration.testFlag(AboutButton));
    }
}

void NavigationBar::onBackButtonClicked()
{
    emit backClicked();
}

void NavigationBar::onSettingsButtonClicked()
{
    emit settingsClicked();
}

void NavigationBar::onAboutButtonClicked()
{
    emit aboutClicked();
}

void NavigationBar::retranslateUi()
{
    if (m_backButton) {
        m_backButton->setText(tr("back_button"));
    }
    
    if (m_settingsButton) {
        m_settingsButton->setText(tr("settings_button"));
    }
    
    if (m_aboutButton) {
        m_aboutButton->setText(tr("about_button"));
    }
}