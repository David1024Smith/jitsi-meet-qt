#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QDebug>

#include "../widgets/CustomButton.h"
#include "../widgets/StatusBar.h"
#include "../widgets/ToolBar.h"
#include "../config/UIConfig.h"

/**
 * @brief UI组件示例应用程序
 * 
 * 这个示例展示了如何使用UI模块中的各种组件：
 * - CustomButton: 自定义按钮组件
 * - StatusBar: 状态栏组件
 * - ToolBar: 工具栏组件
 * - UIConfig: UI配置管理
 */
class UIComponentsExample : public QMainWindow
{
    Q_OBJECT

public:
    UIComponentsExample(QWidget *parent = nullptr);
    ~UIComponentsExample();

private slots:
    void onPrimaryButtonClicked();
    void onSecondaryButtonClicked();
    void onSuccessButtonClicked();
    void onWarningButtonClicked();
    void onDangerButtonClicked();
    void onToolBarActionTriggered();
    void onProgressUpdate();
    void onThemeToggle();

private:
    void setupUI();
    void setupToolBar();
    void setupStatusBar();
    void setupButtons();
    void setupConfiguration();
    void updateProgress();

    // UI组件
    ToolBar* m_toolBar;
    StatusBar* m_statusBar;
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;

    // 按钮组件
    CustomButton* m_primaryButton;
    CustomButton* m_secondaryButton;
    CustomButton* m_successButton;
    CustomButton* m_warningButton;
    CustomButton* m_dangerButton;
    CustomButton* m_themeToggleButton;

    // 配置和状态
    UIConfig* m_uiConfig;
    QTimer* m_progressTimer;
    int m_progressValue;
    bool m_isDarkTheme;
};

UIComponentsExample::UIComponentsExample(QWidget *parent)
    : QMainWindow(parent)
    , m_toolBar(nullptr)
    , m_statusBar(nullptr)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_primaryButton(nullptr)
    , m_secondaryButton(nullptr)
    , m_successButton(nullptr)
    , m_warningButton(nullptr)
    , m_dangerButton(nullptr)
    , m_themeToggleButton(nullptr)
    , m_uiConfig(new UIConfig(this))
    , m_progressTimer(new QTimer(this))
    , m_progressValue(0)
    , m_isDarkTheme(false)
{
    setupConfiguration();
    setupUI();
    
    // 连接进度定时器
    connect(m_progressTimer, &QTimer::timeout, this, &UIComponentsExample::onProgressUpdate);
}

UIComponentsExample::~UIComponentsExample() = default;

void UIComponentsExample::setupUI()
{
    setWindowTitle("UI Components Example - Jitsi Meet Qt");
    setMinimumSize(800, 600);
    
    // 设置中央组件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 添加标题
    QLabel* titleLabel = new QLabel("UI Components Demonstration", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
    titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(titleLabel);
    
    // 设置各个组件
    setupToolBar();
    setupButtons();
    setupStatusBar();
    
    // 添加弹性空间
    m_mainLayout->addStretch();
}

void UIComponentsExample::setupToolBar()
{
    m_toolBar = new ToolBar("Main Toolbar", this);
    addToolBar(m_toolBar);
    
    // 添加工具栏动作
    QAction* newAction = m_toolBar->addAction(QIcon(":/icons/new.png"), "New");
    QAction* openAction = m_toolBar->addAction(QIcon(":/icons/open.png"), "Open");
    QAction* saveAction = m_toolBar->addAction(QIcon(":/icons/save.png"), "Save");
    
    m_toolBar->addSeparator();
    
    QAction* settingsAction = m_toolBar->addAction(QIcon(":/icons/settings.png"), "Settings");
    
    // 连接信号
    connect(newAction, &QAction::triggered, this, &UIComponentsExample::onToolBarActionTriggered);
    connect(openAction, &QAction::triggered, this, &UIComponentsExample::onToolBarActionTriggered);
    connect(saveAction, &QAction::triggered, this, &UIComponentsExample::onToolBarActionTriggered);
    connect(settingsAction, &QAction::triggered, this, &UIComponentsExample::onToolBarActionTriggered);
    
    // 添加自定义按钮到工具栏
    CustomButton* customToolButton = m_toolBar->addCustomButton("Custom");
    customToolButton->setButtonStyle(CustomButton::InfoStyle);
    customToolButton->setButtonSize(CustomButton::SmallSize);
}

void UIComponentsExample::setupStatusBar()
{
    m_statusBar = new StatusBar(this);
    setStatusBar(m_statusBar);
    
    // 设置初始状态
    m_statusBar->setStatusText("Ready");
    m_statusBar->setStatusType(StatusBar::InfoStatus);
    
    // 显示各种状态指示器
    m_statusBar->showConnectionStatus(true);
    m_statusBar->showNetworkQuality(85);
    m_statusBar->showRecordingStatus(false);
    m_statusBar->showMuteStatus(false);
}

void UIComponentsExample::setupButtons()
{
    // 创建按钮布局
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setSpacing(15);
    
    // 创建不同样式的按钮
    m_primaryButton = new CustomButton("Primary", this);
    m_primaryButton->setButtonStyle(CustomButton::PrimaryStyle);
    m_primaryButton->setButtonSize(CustomButton::MediumSize);
    connect(m_primaryButton, &CustomButton::clicked, this, &UIComponentsExample::onPrimaryButtonClicked);
    
    m_secondaryButton = new CustomButton("Secondary", this);
    m_secondaryButton->setButtonStyle(CustomButton::SecondaryStyle);
    m_secondaryButton->setButtonSize(CustomButton::MediumSize);
    connect(m_secondaryButton, &CustomButton::clicked, this, &UIComponentsExample::onSecondaryButtonClicked);
    
    m_successButton = new CustomButton("Success", this);
    m_successButton->setButtonStyle(CustomButton::SuccessStyle);
    m_successButton->setButtonSize(CustomButton::MediumSize);
    connect(m_successButton, &CustomButton::clicked, this, &UIComponentsExample::onSuccessButtonClicked);
    
    m_warningButton = new CustomButton("Warning", this);
    m_warningButton->setButtonStyle(CustomButton::WarningStyle);
    m_warningButton->setButtonSize(CustomButton::MediumSize);
    connect(m_warningButton, &CustomButton::clicked, this, &UIComponentsExample::onWarningButtonClicked);
    
    m_dangerButton = new CustomButton("Danger", this);
    m_dangerButton->setButtonStyle(CustomButton::DangerStyle);
    m_dangerButton->setButtonSize(CustomButton::MediumSize);
    connect(m_dangerButton, &CustomButton::clicked, this, &UIComponentsExample::onDangerButtonClicked);
    
    // 主题切换按钮
    m_themeToggleButton = new CustomButton("Toggle Theme", this);
    m_themeToggleButton->setButtonStyle(CustomButton::LinkStyle);
    m_themeToggleButton->setButtonSize(CustomButton::SmallSize);
    connect(m_themeToggleButton, &CustomButton::clicked, this, &UIComponentsExample::onThemeToggle);
    
    // 添加按钮到布局
    m_buttonLayout->addWidget(m_primaryButton);
    m_buttonLayout->addWidget(m_secondaryButton);
    m_buttonLayout->addWidget(m_successButton);
    m_buttonLayout->addWidget(m_warningButton);
    m_buttonLayout->addWidget(m_dangerButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_themeToggleButton);
    
    // 添加按钮组说明
    QLabel* buttonLabel = new QLabel("Button Styles:", this);
    buttonLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    
    m_mainLayout->addWidget(buttonLabel);
    m_mainLayout->addLayout(m_buttonLayout);
    
    // 添加按钮尺寸示例
    QHBoxLayout* sizeLayout = new QHBoxLayout();
    
    CustomButton* smallButton = new CustomButton("Small", this);
    smallButton->setButtonStyle(CustomButton::OutlineStyle);
    smallButton->setButtonSize(CustomButton::SmallSize);
    
    CustomButton* mediumButton = new CustomButton("Medium", this);
    mediumButton->setButtonStyle(CustomButton::OutlineStyle);
    mediumButton->setButtonSize(CustomButton::MediumSize);
    
    CustomButton* largeButton = new CustomButton("Large", this);
    largeButton->setButtonStyle(CustomButton::OutlineStyle);
    largeButton->setButtonSize(CustomButton::LargeSize);
    
    CustomButton* extraLargeButton = new CustomButton("Extra Large", this);
    extraLargeButton->setButtonStyle(CustomButton::OutlineStyle);
    extraLargeButton->setButtonSize(CustomButton::ExtraLargeSize);
    
    sizeLayout->addWidget(smallButton);
    sizeLayout->addWidget(mediumButton);
    sizeLayout->addWidget(largeButton);
    sizeLayout->addWidget(extraLargeButton);
    sizeLayout->addStretch();
    
    QLabel* sizeLabel = new QLabel("Button Sizes:", this);
    sizeLabel->setStyleSheet("font-weight: bold; margin-top: 20px;");
    
    m_mainLayout->addWidget(sizeLabel);
    m_mainLayout->addLayout(sizeLayout);
}

void UIComponentsExample::setupConfiguration()
{
    // 设置默认配置
    m_uiConfig->setTheme("default");
    m_uiConfig->setLanguage("en_US");
    m_uiConfig->setDarkMode(false);
    m_uiConfig->setAnimationEnabled(true);
    m_uiConfig->setAnimationDuration(250);
    
    // 连接配置变化信号
    connect(m_uiConfig, &UIConfig::themeChanged, [this](const QString& theme) {
        qDebug() << "Theme changed to:" << theme;
    });
    
    connect(m_uiConfig, &UIConfig::darkModeChanged, [this](bool enabled) {
        qDebug() << "Dark mode:" << (enabled ? "enabled" : "disabled");
    });
}

void UIComponentsExample::onPrimaryButtonClicked()
{
    m_statusBar->showMessage("Primary button clicked!", StatusBar::InfoStatus, 3000);
    qDebug() << "Primary button clicked";
}

void UIComponentsExample::onSecondaryButtonClicked()
{
    m_statusBar->showMessage("Secondary button clicked!", StatusBar::InfoStatus, 3000);
    qDebug() << "Secondary button clicked";
}

void UIComponentsExample::onSuccessButtonClicked()
{
    m_statusBar->showMessage("Operation successful!", StatusBar::SuccessStatus, 3000);
    updateProgress();
}

void UIComponentsExample::onWarningButtonClicked()
{
    m_statusBar->showMessage("Warning: Check your settings!", StatusBar::WarningStatus, 5000);
    qDebug() << "Warning button clicked";
}

void UIComponentsExample::onDangerButtonClicked()
{
    m_statusBar->showMessage("Error: Operation failed!", StatusBar::ErrorStatus, 5000);
    qDebug() << "Danger button clicked";
}

void UIComponentsExample::onToolBarActionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString message = QString("Toolbar action '%1' triggered").arg(action->text());
        m_statusBar->showMessage(message, StatusBar::InfoStatus, 2000);
        qDebug() << message;
    }
}

void UIComponentsExample::onProgressUpdate()
{
    m_progressValue += 10;
    m_statusBar->setProgressValue(m_progressValue);
    
    if (m_progressValue >= 100) {
        m_progressTimer->stop();
        m_statusBar->hideProgress();
        m_statusBar->showMessage("Progress completed!", StatusBar::SuccessStatus, 3000);
        m_progressValue = 0;
    }
}

void UIComponentsExample::onThemeToggle()
{
    m_isDarkTheme = !m_isDarkTheme;
    
    if (m_isDarkTheme) {
        m_uiConfig->setTheme("dark");
        m_uiConfig->setDarkMode(true);
        m_themeToggleButton->setText("Light Theme");
    } else {
        m_uiConfig->setTheme("light");
        m_uiConfig->setDarkMode(false);
        m_themeToggleButton->setText("Dark Theme");
    }
    
    QString message = QString("Switched to %1 theme").arg(m_isDarkTheme ? "dark" : "light");
    m_statusBar->showMessage(message, StatusBar::InfoStatus, 2000);
}

void UIComponentsExample::updateProgress()
{
    m_statusBar->showProgress("Processing...");
    m_statusBar->setProgressRange(0, 100);
    m_statusBar->setProgressValue(0);
    m_progressValue = 0;
    m_progressTimer->start(200); // Update every 200ms
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("UI Components Example");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Jitsi Meet Qt");
    
    // 创建并显示示例窗口
    UIComponentsExample window;
    window.show();
    
    return app.exec();
}

#include "UIComponentsExample.moc"