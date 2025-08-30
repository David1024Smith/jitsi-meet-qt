#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include "../../widgets/BaseWidget.h"
#include "../../widgets/CustomButton.h"
#include "../../widgets/StatusBar.h"
#include "../../widgets/ToolBar.h"
#include "../../config/UIConfig.h"

class UIComponentsTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // BaseWidget tests
    void testBaseWidget_Creation_Success();
    void testBaseWidget_ThemeChange_SignalEmitted();
    void testBaseWidget_Configuration_ValidData();

    // CustomButton tests
    void testCustomButton_Creation_Success();
    void testCustomButton_StyleChange_UpdatesAppearance();
    void testCustomButton_SizeChange_UpdatesDimensions();
    void testCustomButton_Configuration_ValidData();

    // StatusBar tests
    void testStatusBar_Creation_Success();
    void testStatusBar_StatusChange_UpdatesDisplay();
    void testStatusBar_ProgressDisplay_WorksCorrectly();
    void testStatusBar_Configuration_ValidData();

    // ToolBar tests
    void testToolBar_Creation_Success();
    void testToolBar_AddAction_Success();
    void testToolBar_AddCustomButton_Success();
    void testToolBar_Configuration_ValidData();

    // UIConfig tests
    void testUIConfig_Creation_Success();
    void testUIConfig_ThemeChange_SignalEmitted();
    void testUIConfig_Serialization_WorksCorrectly();
    void testUIConfig_Validation_WorksCorrectly();

private:
    BaseWidget* m_baseWidget;
    CustomButton* m_customButton;
    StatusBar* m_statusBar;
    ToolBar* m_toolBar;
    UIConfig* m_uiConfig;
};

void UIComponentsTest::initTestCase()
{
    // Initialize test environment
}

void UIComponentsTest::cleanupTestCase()
{
    // Cleanup test environment
}

void UIComponentsTest::init()
{
    // Create fresh instances for each test
    m_baseWidget = new BaseWidget();
    m_customButton = new CustomButton("Test Button");
    m_statusBar = new StatusBar();
    m_toolBar = new ToolBar("Test Toolbar");
    m_uiConfig = new UIConfig();
}

void UIComponentsTest::cleanup()
{
    // Clean up after each test
    delete m_baseWidget;
    delete m_customButton;
    delete m_statusBar;
    delete m_toolBar;
    delete m_uiConfig;
}

void UIComponentsTest::testBaseWidget_Creation_Success()
{
    QVERIFY(m_baseWidget != nullptr);
    QCOMPARE(m_baseWidget->componentName(), QString("BaseWidget"));
    QCOMPARE(m_baseWidget->themeName(), QString("default"));
    QVERIFY(m_baseWidget->isThemeEnabled());
}

void UIComponentsTest::testBaseWidget_ThemeChange_SignalEmitted()
{
    QSignalSpy spy(m_baseWidget, &BaseWidget::themeNameChanged);
    
    m_baseWidget->setThemeName("dark");
    
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_baseWidget->themeName(), QString("dark"));
}

void UIComponentsTest::testBaseWidget_Configuration_ValidData()
{
    QVariantMap config;
    config["themeName"] = "light";
    config["themeEnabled"] = false;
    config["customStyleSheet"] = "background-color: red;";
    
    m_baseWidget->setConfiguration(config);
    
    QCOMPARE(m_baseWidget->themeName(), QString("light"));
    QVERIFY(!m_baseWidget->isThemeEnabled());
    QCOMPARE(m_baseWidget->customStyleSheet(), QString("background-color: red;"));
    
    QVariantMap retrievedConfig = m_baseWidget->getConfiguration();
    QCOMPARE(retrievedConfig["themeName"].toString(), QString("light"));
    QCOMPARE(retrievedConfig["themeEnabled"].toBool(), false);
}

void UIComponentsTest::testCustomButton_Creation_Success()
{
    QVERIFY(m_customButton != nullptr);
    QCOMPARE(m_customButton->componentName(), QString("CustomButton"));
    QCOMPARE(m_customButton->text(), QString("Test Button"));
    QCOMPARE(m_customButton->buttonStyle(), CustomButton::DefaultStyle);
    QCOMPARE(m_customButton->buttonSize(), CustomButton::MediumSize);
}

void UIComponentsTest::testCustomButton_StyleChange_UpdatesAppearance()
{
    QSignalSpy spy(m_customButton, &CustomButton::buttonStyleChanged);
    
    m_customButton->setButtonStyle(CustomButton::PrimaryStyle);
    
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_customButton->buttonStyle(), CustomButton::PrimaryStyle);
}

void UIComponentsTest::testCustomButton_SizeChange_UpdatesDimensions()
{
    QSignalSpy spy(m_customButton, &CustomButton::buttonSizeChanged);
    
    m_customButton->setButtonSize(CustomButton::LargeSize);
    
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_customButton->buttonSize(), CustomButton::LargeSize);
}

void UIComponentsTest::testCustomButton_Configuration_ValidData()
{
    QVariantMap config;
    config["buttonStyle"] = static_cast<int>(CustomButton::SuccessStyle);
    config["buttonSize"] = static_cast<int>(CustomButton::SmallSize);
    config["iconVisible"] = false;
    config["text"] = "New Text";
    
    m_customButton->setConfiguration(config);
    
    QCOMPARE(m_customButton->buttonStyle(), CustomButton::SuccessStyle);
    QCOMPARE(m_customButton->buttonSize(), CustomButton::SmallSize);
    QVERIFY(!m_customButton->isIconVisible());
    QCOMPARE(m_customButton->text(), QString("New Text"));
}

void UIComponentsTest::testStatusBar_Creation_Success()
{
    QVERIFY(m_statusBar != nullptr);
    QCOMPARE(m_statusBar->componentName(), QString("StatusBar"));
    QCOMPARE(m_statusBar->statusType(), StatusBar::InfoStatus);
    QVERIFY(!m_statusBar->isProgressVisible());
}

void UIComponentsTest::testStatusBar_StatusChange_UpdatesDisplay()
{
    QSignalSpy spy(m_statusBar, &StatusBar::statusTextChanged);
    
    m_statusBar->setStatusText("Test Status");
    
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_statusBar->statusText(), QString("Test Status"));
}

void UIComponentsTest::testStatusBar_ProgressDisplay_WorksCorrectly()
{
    QSignalSpy spy(m_statusBar, &StatusBar::progressVisibleChanged);
    
    m_statusBar->showProgress("Loading...");
    
    QCOMPARE(spy.count(), 1);
    QVERIFY(m_statusBar->isProgressVisible());
    QCOMPARE(m_statusBar->statusText(), QString("Loading..."));
    
    m_statusBar->hideProgress();
    QVERIFY(!m_statusBar->isProgressVisible());
}

void UIComponentsTest::testStatusBar_Configuration_ValidData()
{
    QVariantMap config;
    config["statusText"] = "Test Message";
    config["statusType"] = static_cast<int>(StatusBar::WarningStatus);
    config["progressVisible"] = true;
    config["progressValue"] = 50;
    
    m_statusBar->setConfiguration(config);
    
    QCOMPARE(m_statusBar->statusText(), QString("Test Message"));
    QCOMPARE(m_statusBar->statusType(), StatusBar::WarningStatus);
    QVERIFY(m_statusBar->isProgressVisible());
    QCOMPARE(m_statusBar->progressValue(), 50);
}

void UIComponentsTest::testToolBar_Creation_Success()
{
    QVERIFY(m_toolBar != nullptr);
    QCOMPARE(m_toolBar->componentName(), QString("ToolBar"));
    QCOMPARE(m_toolBar->toolBarStyle(), ToolBar::IconAndTextStyle);
    QVERIFY(m_toolBar->areIconsVisible());
    QVERIFY(m_toolBar->isTextVisible());
}

void UIComponentsTest::testToolBar_AddAction_Success()
{
    QAction* action = m_toolBar->addAction("Test Action");
    
    QVERIFY(action != nullptr);
    QCOMPARE(action->text(), QString("Test Action"));
    QVERIFY(m_toolBar->actions().contains(action));
}

void UIComponentsTest::testToolBar_AddCustomButton_Success()
{
    QSignalSpy spy(m_toolBar, &ToolBar::customButtonAdded);
    
    CustomButton* button = m_toolBar->addCustomButton("Custom Button");
    
    QVERIFY(button != nullptr);
    QCOMPARE(button->text(), QString("Custom Button"));
    QCOMPARE(spy.count(), 1);
}

void UIComponentsTest::testToolBar_Configuration_ValidData()
{
    QVariantMap config;
    config["toolBarStyle"] = static_cast<int>(ToolBar::IconOnlyStyle);
    config["iconsVisible"] = true;
    config["textVisible"] = false;
    config["buttonSize"] = 24;
    
    m_toolBar->setConfiguration(config);
    
    QCOMPARE(m_toolBar->toolBarStyle(), ToolBar::IconOnlyStyle);
    QVERIFY(m_toolBar->areIconsVisible());
    QVERIFY(!m_toolBar->isTextVisible());
    QCOMPARE(m_toolBar->buttonSize(), 24);
}

void UIComponentsTest::testUIConfig_Creation_Success()
{
    QVERIFY(m_uiConfig != nullptr);
    QCOMPARE(m_uiConfig->theme(), QString("default"));
    QCOMPARE(m_uiConfig->language(), QString("en_US"));
    QVERIFY(!m_uiConfig->isDarkMode());
}

void UIComponentsTest::testUIConfig_ThemeChange_SignalEmitted()
{
    QSignalSpy spy(m_uiConfig, &UIConfig::themeChanged);
    
    m_uiConfig->setTheme("dark");
    
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_uiConfig->theme(), QString("dark"));
}

void UIComponentsTest::testUIConfig_Serialization_WorksCorrectly()
{
    // Set some configuration values
    m_uiConfig->setTheme("dark");
    m_uiConfig->setLanguage("zh_CN");
    m_uiConfig->setDarkMode(true);
    m_uiConfig->setFontSize(14);
    
    // Serialize to JSON
    QByteArray json = m_uiConfig->toJson();
    QVERIFY(!json.isEmpty());
    
    // Create new config and deserialize
    UIConfig newConfig;
    QVERIFY(newConfig.fromJson(json));
    
    // Verify values
    QCOMPARE(newConfig.theme(), QString("dark"));
    QCOMPARE(newConfig.language(), QString("zh_CN"));
    QVERIFY(newConfig.isDarkMode());
    QCOMPARE(newConfig.fontSize(), 14);
}

void UIComponentsTest::testUIConfig_Validation_WorksCorrectly()
{
    // Valid configuration
    QVERIFY(m_uiConfig->validate());
    QVERIFY(m_uiConfig->validationErrors().isEmpty());
    
    // Invalid scaling factor
    m_uiConfig->setScalingFactor(-1.0);
    QVERIFY(!m_uiConfig->validate());
    QVERIFY(!m_uiConfig->validationErrors().isEmpty());
    
    // Fix the issue
    m_uiConfig->setScalingFactor(1.0);
    QVERIFY(m_uiConfig->validate());
}

QTEST_MAIN(UIComponentsTest)
#include "UIComponentsTest.moc"