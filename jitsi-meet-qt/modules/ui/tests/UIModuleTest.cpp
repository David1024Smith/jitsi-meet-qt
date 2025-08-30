#include "UIModuleTest.h"
#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include <QApplication>
#include <QScreen>
#include <QStyleFactory>
#include <QThread>

UIModuleTest::UIModuleTest()
    : m_uiModule(nullptr)
    , m_uiManager(nullptr)
    , m_themeManager(nullptr)
    , m_uiConfig(nullptr)
    , m_testBaseWidget(nullptr)
    , m_testButton(nullptr)
    , m_testStatusBar(nullptr)
    , m_testToolBar(nullptr)
    , m_testMainLayout(nullptr)
    , m_testConferenceLayout(nullptr)
    , m_testSettingsLayout(nullptr)
    , m_defaultTheme(nullptr)
    , m_darkTheme(nullptr)
    , m_lightTheme(nullptr)
    , m_testWindow(nullptr)
    , m_testLayout(nullptr)
{
}

UIModuleTest::~UIModuleTest()
{
}

void UIModuleTest::initTestCase()
{
    qDebug() << "Initializing UI Module Test Suite...";
    
    // Ensure we have a QApplication instance
    if (!QApplication::instance()) {
        int argc = 0;
        char* argv[] = {nullptr};
        new QApplication(argc, argv);
    }
    
    setupTestEnvironment();
    qDebug() << "UI Module Test Suite initialized successfully";
}

void UIModuleTest::cleanupTestCase()
{
    qDebug() << "Cleaning up UI Module Test Suite...";
    cleanupTestEnvironment();
    qDebug() << "UI Module Test Suite cleanup completed";
}

void UIModuleTest::init()
{
    // Create fresh instances for each test
    createTestWidgets();
}

void UIModuleTest::cleanup()
{
    // Clean up after each test
    destroyTestWidgets();
}

void UIModuleTest::setupTestEnvironment()
{
    // Create test window
    m_testWindow = new QWidget();
    m_testWindow->setWindowTitle("UI Module Test Window");
    m_testWindow->resize(800, 600);
    
    m_testLayout = new QVBoxLayout(m_testWindow);
    m_testWindow->setLayout(m_testLayout);
}

void UIModuleTest::cleanupTestEnvironment()
{
    if (m_testWindow) {
        delete m_testWindow;
        m_testWindow = nullptr;
    }
}

void UIModuleTest::createTestWidgets()
{
    // Create UI module components
    m_uiModule = new UIModule();
    m_uiManager = new UIManager();
    m_themeManager = new ThemeManager();
    m_uiConfig = new UIConfig();
    
    // Create test widgets
    m_testBaseWidget = new BaseWidget();
    m_testButton = new CustomButton("Test Button");
    m_testStatusBar = new StatusBar();
    m_testToolBar = new ToolBar("Test Toolbar");
    
    // Create test layouts
    m_testMainLayout = new MainLayout();
    m_testConferenceLayout = new ConferenceLayout();
    m_testSettingsLayout = new SettingsLayout();
    
    // Create test themes
    m_defaultTheme = new DefaultTheme();
    m_darkTheme = new DarkTheme();
    m_lightTheme = new LightTheme();
}

void UIModuleTest::destroyTestWidgets()
{
    // Clean up themes
    delete m_defaultTheme;
    delete m_darkTheme;
    delete m_lightTheme;
    m_defaultTheme = m_darkTheme = m_lightTheme = nullptr;
    
    // Clean up layouts
    delete m_testMainLayout;
    delete m_testConferenceLayout;
    delete m_testSettingsLayout;
    m_testMainLayout = m_testConferenceLayout = m_testSettingsLayout = nullptr;
    
    // Clean up widgets
    delete m_testBaseWidget;
    delete m_testButton;
    delete m_testStatusBar;
    delete m_testToolBar;
    m_testBaseWidget = nullptr;
    m_testButton = nullptr;
    m_testStatusBar = nullptr;
    m_testToolBar = nullptr;
    
    // Clean up UI components
    delete m_uiModule;
    delete m_uiManager;
    delete m_themeManager;
    delete m_uiConfig;
    m_uiModule = nullptr;
    m_uiManager = nullptr;
    m_themeManager = nullptr;
    m_uiConfig = nullptr;
}

void UIModuleTest::waitForSignal(QObject* sender, const char* signal, int timeout)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(timeout);
    
    connect(sender, signal, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    timer.start();
    loop.exec();
}

bool UIModuleTest::compareThemes(BaseTheme* theme1, BaseTheme* theme2)
{
    if (!theme1 || !theme2) return false;
    
    return theme1->name() == theme2->name() &&
           theme1->primaryColor() == theme2->primaryColor() &&
           theme1->backgroundColor() == theme2->backgroundColor() &&
           theme1->textColor() == theme2->textColor();
}

void UIModuleTest::verifyThemeApplication(BaseWidget* widget, const QString& themeName)
{
    QVERIFY(widget != nullptr);
    QCOMPARE(widget->themeName(), themeName);
    
    // Verify theme-specific properties are applied
    if (themeName == "dark") {
        QVERIFY(widget->palette().color(QPalette::Window).value() < 128);
    } else if (themeName == "light") {
        QVERIFY(widget->palette().color(QPalette::Window).value() > 128);
    }
}

void UIModuleTest::verifyLayoutApplication(QWidget* container, const QString& layoutName)
{
    QVERIFY(container != nullptr);
    QVERIFY(container->layout() != nullptr);
    
    // Verify layout type matches expected layout
    if (layoutName == "main") {
        QVERIFY(qobject_cast<QVBoxLayout*>(container->layout()) != nullptr);
    } else if (layoutName == "conference") {
        QVERIFY(qobject_cast<QGridLayout*>(container->layout()) != nullptr);
    }
}

QSize UIModuleTest::getOptimalSize(QWidget* widget)
{
    if (!widget) return QSize();
    
    widget->adjustSize();
    return widget->sizeHint();
}

void UIModuleTest::simulateWindowResize(QWidget* window, const QSize& newSize)
{
    if (!window) return;
    
    window->resize(newSize);
    QApplication::processEvents();
}

// Core module tests
void UIModuleTest::testUIModule_Initialization_Success()
{
    QVERIFY(m_uiModule != nullptr);
    QVERIFY(m_uiModule->initialize());
    QCOMPARE(m_uiModule->name(), QString("UIModule"));
    QCOMPARE(m_uiModule->version(), QString("1.0.0"));
    QVERIFY(m_uiModule->isInitialized());
}

void UIModuleTest::testUIModule_Configuration_ValidData()
{
    QVariantMap config;
    config["theme"] = "dark";
    config["language"] = "en_US";
    config["scalingFactor"] = 1.2;
    
    QVERIFY(m_uiModule->setConfiguration(config));
    
    QVariantMap retrievedConfig = m_uiModule->getConfiguration();
    QCOMPARE(retrievedConfig["theme"].toString(), QString("dark"));
    QCOMPARE(retrievedConfig["language"].toString(), QString("en_US"));
    QCOMPARE(retrievedConfig["scalingFactor"].toDouble(), 1.2);
}

void UIModuleTest::testUIModule_Status_ReportsCorrectly()
{
    QCOMPARE(m_uiModule->status(), UIModule::NotInitialized);
    
    m_uiModule->initialize();
    QCOMPARE(m_uiModule->status(), UIModule::Initialized);
    
    m_uiModule->start();
    QCOMPARE(m_uiModule->status(), UIModule::Running);
    
    m_uiModule->stop();
    QCOMPARE(m_uiModule->status(), UIModule::Stopped);
}

void UIModuleTest::testUIModule_ErrorHandling_WorksCorrectly()
{
    // Test invalid configuration
    QVariantMap invalidConfig;
    invalidConfig["scalingFactor"] = -1.0;
    
    QVERIFY(!m_uiModule->setConfiguration(invalidConfig));
    QVERIFY(!m_uiModule->lastError().isEmpty());
    
    // Test double initialization
    m_uiModule->initialize();
    QVERIFY(!m_uiModule->initialize()); // Should fail on second call
}

// UI Manager tests
void UIModuleTest::testUIManager_Initialization_Success()
{
    QVERIFY(m_uiManager != nullptr);
    QVERIFY(m_uiManager->initialize());
    QVERIFY(m_uiManager->themeManager() != nullptr);
    QVERIFY(m_uiManager->layoutManager() != nullptr);
}

void UIModuleTest::testUIManager_ThemeManagement_WorksCorrectly()
{
    m_uiManager->initialize();
    
    QSignalSpy spy(m_uiManager, &UIManager::themeChanged);
    
    QVERIFY(m_uiManager->setTheme("dark"));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_uiManager->currentTheme(), QString("dark"));
    
    QVERIFY(m_uiManager->setTheme("light"));
    QCOMPARE(spy.count(), 2);
    QCOMPARE(m_uiManager->currentTheme(), QString("light"));
}

void UIModuleTest::testUIManager_LayoutManagement_WorksCorrectly()
{
    m_uiManager->initialize();
    
    QSignalSpy spy(m_uiManager, &UIManager::layoutChanged);
    
    QVERIFY(m_uiManager->setLayout("conference"));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_uiManager->currentLayout(), QString("conference"));
}

void UIModuleTest::testUIManager_WidgetRegistration_WorksCorrectly()
{
    m_uiManager->initialize();
    
    QVERIFY(m_uiManager->registerWidget(m_testBaseWidget));
    QVERIFY(m_uiManager->isWidgetRegistered(m_testBaseWidget));
    
    QVERIFY(m_uiManager->unregisterWidget(m_testBaseWidget));
    QVERIFY(!m_uiManager->isWidgetRegistered(m_testBaseWidget));
}

// Theme switching tests
void UIModuleTest::testThemeManager_DefaultTheme_AppliesCorrectly()
{
    m_themeManager->initialize();
    
    QVERIFY(m_themeManager->setTheme("default"));
    QCOMPARE(m_themeManager->currentTheme()->name(), QString("default"));
    
    m_themeManager->applyTheme(m_testBaseWidget);
    verifyThemeApplication(m_testBaseWidget, "default");
}

void UIModuleTest::testThemeManager_DarkTheme_AppliesCorrectly()
{
    m_themeManager->initialize();
    
    QVERIFY(m_themeManager->setTheme("dark"));
    QCOMPARE(m_themeManager->currentTheme()->name(), QString("dark"));
    
    m_themeManager->applyTheme(m_testBaseWidget);
    verifyThemeApplication(m_testBaseWidget, "dark");
}

void UIModuleTest::testThemeManager_LightTheme_AppliesCorrectly()
{
    m_themeManager->initialize();
    
    QVERIFY(m_themeManager->setTheme("light"));
    QCOMPARE(m_themeManager->currentTheme()->name(), QString("light"));
    
    m_themeManager->applyTheme(m_testBaseWidget);
    verifyThemeApplication(m_testBaseWidget, "light");
}

void UIModuleTest::testThemeManager_CustomTheme_AppliesCorrectly()
{
    m_themeManager->initialize();
    
    // Register custom theme
    BaseTheme* customTheme = new BaseTheme("custom");
    customTheme->setPrimaryColor(QColor(255, 0, 0));
    customTheme->setBackgroundColor(QColor(128, 128, 128));
    
    QVERIFY(m_themeManager->registerTheme(customTheme));
    QVERIFY(m_themeManager->setTheme("custom"));
    
    m_themeManager->applyTheme(m_testBaseWidget);
    verifyThemeApplication(m_testBaseWidget, "custom");
}

void UIModuleTest::testThemeManager_ThemeSwitch_SignalsEmitted()
{
    m_themeManager->initialize();
    
    QSignalSpy spy(m_themeManager, &ThemeManager::themeChanged);
    
    m_themeManager->setTheme("dark");
    QCOMPARE(spy.count(), 1);
    
    m_themeManager->setTheme("light");
    QCOMPARE(spy.count(), 2);
    
    // Verify signal arguments
    QList<QVariant> arguments = spy.takeLast();
    QCOMPARE(arguments.at(0).toString(), QString("light"));
}

void UIModuleTest::testThemeManager_InvalidTheme_HandledGracefully()
{
    m_themeManager->initialize();
    
    QVERIFY(!m_themeManager->setTheme("nonexistent"));
    QVERIFY(!m_themeManager->lastError().isEmpty());
    
    // Should maintain current theme
    QCOMPARE(m_themeManager->currentTheme()->name(), QString("default"));
}

QTEST_MAIN(UIModuleTest)
#include "UIModuleTest.moc"// C
omponent rendering tests
void UIModuleTest::testComponentRendering_BaseWidget_RendersCorrectly()
{
    m_testLayout->addWidget(m_testBaseWidget);
    m_testWindow->show();
    QApplication::processEvents();
    
    QVERIFY(m_testBaseWidget->isVisible());
    QVERIFY(m_testBaseWidget->size().isValid());
    QVERIFY(m_testBaseWidget->size().width() > 0);
    QVERIFY(m_testBaseWidget->size().height() > 0);
}

void UIModuleTest::testComponentRendering_CustomButton_RendersCorrectly()
{
    m_testLayout->addWidget(m_testButton);
    m_testWindow->show();
    QApplication::processEvents();
    
    QVERIFY(m_testButton->isVisible());
    QCOMPARE(m_testButton->text(), QString("Test Button"));
    QVERIFY(m_testButton->sizeHint().isValid());
}

void UIModuleTest::testComponentRendering_StatusBar_RendersCorrectly()
{
    m_testLayout->addWidget(m_testStatusBar);
    m_testWindow->show();
    QApplication::processEvents();
    
    QVERIFY(m_testStatusBar->isVisible());
    QVERIFY(m_testStatusBar->height() > 0);
    
    m_testStatusBar->setStatusText("Test Status");
    QCOMPARE(m_testStatusBar->statusText(), QString("Test Status"));
}

void UIModuleTest::testComponentRendering_ToolBar_RendersCorrectly()
{
    m_testLayout->addWidget(m_testToolBar);
    m_testWindow->show();
    QApplication::processEvents();
    
    QVERIFY(m_testToolBar->isVisible());
    QVERIFY(m_testToolBar->height() > 0);
    
    QAction* action = m_testToolBar->addAction("Test Action");
    QVERIFY(action != nullptr);
    QVERIFY(m_testToolBar->actions().contains(action));
}

void UIModuleTest::testComponentRendering_ThemeApplication_WorksCorrectly()
{
    m_themeManager->initialize();
    m_themeManager->setTheme("dark");
    
    m_testLayout->addWidget(m_testBaseWidget);
    m_themeManager->applyTheme(m_testBaseWidget);
    
    m_testWindow->show();
    QApplication::processEvents();
    
    verifyThemeApplication(m_testBaseWidget, "dark");
}

void UIModuleTest::testComponentRendering_StyleSheetApplication_WorksCorrectly()
{
    QString customStyle = "background-color: red; color: white;";
    m_testBaseWidget->setCustomStyleSheet(customStyle);
    
    m_testLayout->addWidget(m_testBaseWidget);
    m_testWindow->show();
    QApplication::processEvents();
    
    QCOMPARE(m_testBaseWidget->customStyleSheet(), customStyle);
    QVERIFY(m_testBaseWidget->styleSheet().contains("background-color: red"));
}

// Layout management tests
void UIModuleTest::testLayoutManager_MainLayout_WorksCorrectly()
{
    QWidget* container = new QWidget();
    m_testMainLayout->applyTo(container);
    
    QVERIFY(container->layout() != nullptr);
    QVERIFY(qobject_cast<QVBoxLayout*>(container->layout()) != nullptr);
    
    delete container;
}

void UIModuleTest::testLayoutManager_ConferenceLayout_WorksCorrectly()
{
    QWidget* container = new QWidget();
    m_testConferenceLayout->applyTo(container);
    
    QVERIFY(container->layout() != nullptr);
    QVERIFY(qobject_cast<QGridLayout*>(container->layout()) != nullptr);
    
    delete container;
}

void UIModuleTest::testLayoutManager_SettingsLayout_WorksCorrectly()
{
    QWidget* container = new QWidget();
    m_testSettingsLayout->applyTo(container);
    
    QVERIFY(container->layout() != nullptr);
    QVERIFY(qobject_cast<QVBoxLayout*>(container->layout()) != nullptr);
    
    delete container;
}

void UIModuleTest::testLayoutManager_LayoutSwitch_WorksCorrectly()
{
    m_uiManager->initialize();
    QWidget* container = new QWidget();
    
    // Apply main layout
    QVERIFY(m_uiManager->applyLayout("main", container));
    verifyLayoutApplication(container, "main");
    
    // Switch to conference layout
    QVERIFY(m_uiManager->applyLayout("conference", container));
    verifyLayoutApplication(container, "conference");
    
    delete container;
}

void UIModuleTest::testLayoutManager_LayoutPersistence_WorksCorrectly()
{
    m_uiManager->initialize();
    
    // Set layout and verify persistence
    QVERIFY(m_uiManager->setLayout("conference"));
    QCOMPARE(m_uiManager->currentLayout(), QString("conference"));
    
    // Simulate restart
    m_uiManager->shutdown();
    m_uiManager->initialize();
    
    // Layout should be restored
    QCOMPARE(m_uiManager->currentLayout(), QString("conference"));
}

// Responsive design tests
void UIModuleTest::testResponsiveDesign_WindowResize_LayoutAdjusts()
{
    m_testLayout->addWidget(m_testBaseWidget);
    m_testWindow->show();
    
    QSize originalSize = m_testWindow->size();
    QSize widgetOriginalSize = m_testBaseWidget->size();
    
    // Resize window
    simulateWindowResize(m_testWindow, QSize(1200, 800));
    
    // Widget should adjust
    QVERIFY(m_testBaseWidget->size().width() > widgetOriginalSize.width());
}

void UIModuleTest::testResponsiveDesign_ScreenDPI_ScalingWorks()
{
    QScreen* screen = QApplication::primaryScreen();
    qreal dpi = screen->logicalDotsPerInch();
    
    m_uiConfig->setScalingFactor(dpi / 96.0); // Standard DPI scaling
    
    m_testLayout->addWidget(m_testButton);
    m_testWindow->show();
    QApplication::processEvents();
    
    // Button should scale with DPI
    QSize expectedSize = m_testButton->sizeHint();
    QVERIFY(expectedSize.width() > 0);
    QVERIFY(expectedSize.height() > 0);
}

void UIModuleTest::testResponsiveDesign_FontScaling_WorksCorrectly()
{
    int originalFontSize = m_testButton->font().pointSize();
    
    m_uiConfig->setFontSize(16);
    m_uiConfig->applyTo(m_testButton);
    
    QCOMPARE(m_testButton->font().pointSize(), 16);
    QVERIFY(m_testButton->font().pointSize() != originalFontSize);
}

void UIModuleTest::testResponsiveDesign_ComponentScaling_WorksCorrectly()
{
    m_uiConfig->setScalingFactor(1.5);
    
    QSize originalSize = m_testButton->sizeHint();
    m_uiConfig->applyTo(m_testButton);
    QSize scaledSize = m_testButton->sizeHint();
    
    // Size should be scaled
    QVERIFY(scaledSize.width() >= originalSize.width());
    QVERIFY(scaledSize.height() >= originalSize.height());
}

// UI configuration tests
void UIModuleTest::testUIConfig_Creation_Success()
{
    QVERIFY(m_uiConfig != nullptr);
    QCOMPARE(m_uiConfig->theme(), QString("default"));
    QCOMPARE(m_uiConfig->language(), QString("en_US"));
    QCOMPARE(m_uiConfig->scalingFactor(), 1.0);
    QVERIFY(m_uiConfig->validate());
}

void UIModuleTest::testUIConfig_ThemeConfiguration_WorksCorrectly()
{
    QSignalSpy spy(m_uiConfig, &UIConfig::themeChanged);
    
    m_uiConfig->setTheme("dark");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_uiConfig->theme(), QString("dark"));
    
    m_uiConfig->setDarkMode(true);
    QVERIFY(m_uiConfig->isDarkMode());
}

void UIModuleTest::testUIConfig_LayoutConfiguration_WorksCorrectly()
{
    QSignalSpy spy(m_uiConfig, &UIConfig::layoutChanged);
    
    m_uiConfig->setLayout("conference");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_uiConfig->layout(), QString("conference"));
}

void UIModuleTest::testUIConfig_Serialization_WorksCorrectly()
{
    // Set configuration values
    m_uiConfig->setTheme("dark");
    m_uiConfig->setLanguage("zh_CN");
    m_uiConfig->setScalingFactor(1.2);
    m_uiConfig->setFontSize(14);
    
    // Serialize
    QByteArray json = m_uiConfig->toJson();
    QVERIFY(!json.isEmpty());
    
    // Deserialize to new config
    UIConfig newConfig;
    QVERIFY(newConfig.fromJson(json));
    
    // Verify values
    QCOMPARE(newConfig.theme(), QString("dark"));
    QCOMPARE(newConfig.language(), QString("zh_CN"));
    QCOMPARE(newConfig.scalingFactor(), 1.2);
    QCOMPARE(newConfig.fontSize(), 14);
}

void UIModuleTest::testUIConfig_Validation_WorksCorrectly()
{
    // Valid configuration
    QVERIFY(m_uiConfig->validate());
    QVERIFY(m_uiConfig->validationErrors().isEmpty());
    
    // Invalid scaling factor
    m_uiConfig->setScalingFactor(-1.0);
    QVERIFY(!m_uiConfig->validate());
    QVERIFY(!m_uiConfig->validationErrors().isEmpty());
    
    // Invalid font size
    m_uiConfig->setScalingFactor(1.0); // Fix previous error
    m_uiConfig->setFontSize(0);
    QVERIFY(!m_uiConfig->validate());
    
    // Fix all errors
    m_uiConfig->setFontSize(12);
    QVERIFY(m_uiConfig->validate());
}

void UIModuleTest::testUIConfig_Persistence_WorksCorrectly()
{
    QString configFile = "test_ui_config.json";
    
    // Set and save configuration
    m_uiConfig->setTheme("dark");
    m_uiConfig->setLanguage("fr_FR");
    QVERIFY(m_uiConfig->saveToFile(configFile));
    
    // Load into new config
    UIConfig loadedConfig;
    QVERIFY(loadedConfig.loadFromFile(configFile));
    
    // Verify values
    QCOMPARE(loadedConfig.theme(), QString("dark"));
    QCOMPARE(loadedConfig.language(), QString("fr_FR"));
    
    // Cleanup
    QFile::remove(configFile);
}// Com
patibility tests
void UIModuleTest::testCompatibility_ExistingWidgets_IntegrateCorrectly()
{
    // Create standard Qt widgets
    QPushButton* qtButton = new QPushButton("Qt Button");
    QLabel* qtLabel = new QLabel("Qt Label");
    
    // Add to UI manager
    QVERIFY(m_uiManager->registerWidget(qtButton));
    QVERIFY(m_uiManager->registerWidget(qtLabel));
    
    // Apply theme to Qt widgets
    m_themeManager->setTheme("dark");
    m_themeManager->applyTheme(qtButton);
    m_themeManager->applyTheme(qtLabel);
    
    // Verify integration
    QVERIFY(m_uiManager->isWidgetRegistered(qtButton));
    QVERIFY(m_uiManager->isWidgetRegistered(qtLabel));
    
    delete qtButton;
    delete qtLabel;
}

void UIModuleTest::testCompatibility_ExistingLayouts_IntegrateCorrectly()
{
    QWidget* container = new QWidget();
    QHBoxLayout* qtLayout = new QHBoxLayout();
    
    // Add custom widgets to Qt layout
    qtLayout->addWidget(m_testButton);
    qtLayout->addWidget(m_testStatusBar);
    container->setLayout(qtLayout);
    
    // Apply theme to container
    m_themeManager->setTheme("light");
    m_themeManager->applyTheme(container);
    
    // Verify layout works with themed widgets
    QVERIFY(container->layout() != nullptr);
    QCOMPARE(container->layout()->count(), 2);
    
    delete container;
}

void UIModuleTest::testCompatibility_ExistingThemes_IntegrateCorrectly()
{
    // Test with system theme
    QString systemStyle = QApplication::style()->objectName();
    
    // Apply custom theme over system theme
    m_themeManager->setTheme("dark");
    m_themeManager->applyTheme(m_testBaseWidget);
    
    // Verify custom theme doesn't break system functionality
    QVERIFY(m_testBaseWidget->style() != nullptr);
    QCOMPARE(m_testBaseWidget->themeName(), QString("dark"));
}

void UIModuleTest::testCompatibility_LegacyAPI_WorksCorrectly()
{
    // Test legacy configuration methods
    QVariantMap legacyConfig;
    legacyConfig["style"] = "dark"; // Old key name
    legacyConfig["lang"] = "en"; // Old key name
    
    // Should handle legacy keys gracefully
    QVERIFY(m_uiConfig->setLegacyConfiguration(legacyConfig));
    QCOMPARE(m_uiConfig->theme(), QString("dark"));
    QCOMPARE(m_uiConfig->language(), QString("en"));
}

// Performance tests
void UIModuleTest::testPerformance_ThemeSwitch_CompletesQuickly()
{
    m_themeManager->initialize();
    
    // Add multiple widgets
    QList<BaseWidget*> widgets;
    for (int i = 0; i < 100; ++i) {
        widgets.append(new BaseWidget());
        m_uiManager->registerWidget(widgets.last());
    }
    
    // Measure theme switch time
    QElapsedTimer timer;
    timer.start();
    
    m_themeManager->setTheme("dark");
    
    qint64 elapsed = timer.elapsed();
    QVERIFY(elapsed < PERFORMANCE_TIMEOUT);
    
    qDebug() << "Theme switch took" << elapsed << "ms for 100 widgets";
    
    // Cleanup
    qDeleteAll(widgets);
}

void UIModuleTest::testPerformance_LayoutSwitch_CompletesQuickly()
{
    m_uiManager->initialize();
    QWidget* container = new QWidget();
    
    // Measure layout switch time
    QElapsedTimer timer;
    timer.start();
    
    m_uiManager->applyLayout("conference", container);
    
    qint64 elapsed = timer.elapsed();
    QVERIFY(elapsed < PERFORMANCE_TIMEOUT);
    
    qDebug() << "Layout switch took" << elapsed << "ms";
    
    delete container;
}

void UIModuleTest::testPerformance_ComponentCreation_CompletesQuickly()
{
    QElapsedTimer timer;
    timer.start();
    
    // Create multiple components
    QList<CustomButton*> buttons;
    for (int i = 0; i < 1000; ++i) {
        buttons.append(new CustomButton(QString("Button %1").arg(i)));
    }
    
    qint64 elapsed = timer.elapsed();
    QVERIFY(elapsed < PERFORMANCE_TIMEOUT * 2); // Allow more time for 1000 widgets
    
    qDebug() << "Created 1000 buttons in" << elapsed << "ms";
    
    // Cleanup
    qDeleteAll(buttons);
}

void UIModuleTest::testPerformance_MemoryUsage_WithinLimits()
{
    // This is a simplified memory test
    // In a real scenario, you'd use more sophisticated memory profiling
    
    size_t initialMemory = QThread::currentThread()->stackSize();
    
    // Create many widgets
    QList<BaseWidget*> widgets;
    for (int i = 0; i < 1000; ++i) {
        widgets.append(new BaseWidget());
    }
    
    // Memory usage should be reasonable
    size_t currentMemory = QThread::currentThread()->stackSize();
    QVERIFY(currentMemory - initialMemory < MAX_MEMORY_USAGE);
    
    // Cleanup
    qDeleteAll(widgets);
}

// Error handling tests
void UIModuleTest::testErrorHandling_InvalidTheme_HandledGracefully()
{
    m_themeManager->initialize();
    
    QString originalTheme = m_themeManager->currentTheme()->name();
    
    // Try to set invalid theme
    QVERIFY(!m_themeManager->setTheme("nonexistent_theme"));
    
    // Should maintain current theme
    QCOMPARE(m_themeManager->currentTheme()->name(), originalTheme);
    
    // Error should be reported
    QVERIFY(!m_themeManager->lastError().isEmpty());
}

void UIModuleTest::testErrorHandling_InvalidLayout_HandledGracefully()
{
    m_uiManager->initialize();
    QWidget* container = new QWidget();
    
    // Try to apply invalid layout
    QVERIFY(!m_uiManager->applyLayout("nonexistent_layout", container));
    
    // Container should remain unchanged
    QVERIFY(container->layout() == nullptr);
    
    // Error should be reported
    QVERIFY(!m_uiManager->lastError().isEmpty());
    
    delete container;
}

void UIModuleTest::testErrorHandling_InvalidConfiguration_HandledGracefully()
{
    QVariantMap invalidConfig;
    invalidConfig["scalingFactor"] = "invalid_string"; // Should be double
    invalidConfig["fontSize"] = -10; // Invalid value
    
    QVERIFY(!m_uiConfig->setConfiguration(invalidConfig));
    QVERIFY(!m_uiConfig->validationErrors().isEmpty());
    
    // Configuration should remain valid
    QVERIFY(m_uiConfig->validate());
}

void UIModuleTest::testErrorHandling_ResourceNotFound_HandledGracefully()
{
    // Try to load non-existent configuration file
    QVERIFY(!m_uiConfig->loadFromFile("nonexistent_file.json"));
    QVERIFY(!m_uiConfig->lastError().isEmpty());
    
    // Try to load non-existent theme resource
    BaseTheme* theme = new BaseTheme("test");
    QVERIFY(!theme->loadFromResource(":/nonexistent/theme.qss"));
    
    delete theme;
}

// Integration tests
void UIModuleTest::testIntegration_ThemeAndLayout_WorkTogether()
{
    m_uiManager->initialize();
    QWidget* container = new QWidget();
    
    // Apply layout and theme together
    QVERIFY(m_uiManager->applyLayout("main", container));
    QVERIFY(m_uiManager->setTheme("dark"));
    
    // Add widgets to container
    container->layout()->addWidget(m_testButton);
    container->layout()->addWidget(m_testStatusBar);
    
    // Theme should be applied to all widgets in layout
    verifyThemeApplication(m_testButton, "dark");
    verifyThemeApplication(m_testStatusBar, "dark");
    
    delete container;
}

void UIModuleTest::testIntegration_ConfigAndTheme_WorkTogether()
{
    // Set configuration
    m_uiConfig->setTheme("light");
    m_uiConfig->setFontSize(16);
    m_uiConfig->setScalingFactor(1.2);
    
    // Apply configuration to UI manager
    m_uiManager->setConfiguration(m_uiConfig->toVariantMap());
    
    // Verify theme and configuration are both applied
    QCOMPARE(m_uiManager->currentTheme(), QString("light"));
    
    m_uiManager->applyTheme(m_testButton);
    QCOMPARE(m_testButton->font().pointSize(), 16);
}

void UIModuleTest::testIntegration_WidgetsAndLayout_WorkTogether()
{
    QWidget* container = new QWidget();
    m_testMainLayout->applyTo(container);
    
    // Add various widgets to layout
    container->layout()->addWidget(m_testButton);
    container->layout()->addWidget(m_testStatusBar);
    container->layout()->addWidget(m_testToolBar);
    
    // Show container and verify all widgets are visible
    container->show();
    QApplication::processEvents();
    
    QVERIFY(m_testButton->isVisible());
    QVERIFY(m_testStatusBar->isVisible());
    QVERIFY(m_testToolBar->isVisible());
    
    delete container;
}

void UIModuleTest::testIntegration_FullUIWorkflow_WorksCorrectly()
{
    // Initialize complete UI system
    QVERIFY(m_uiModule->initialize());
    QVERIFY(m_uiManager->initialize());
    
    // Set up configuration
    m_uiConfig->setTheme("dark");
    m_uiConfig->setLayout("conference");
    m_uiConfig->setLanguage("en_US");
    
    // Apply configuration
    QVERIFY(m_uiManager->setConfiguration(m_uiConfig->toVariantMap()));
    
    // Create UI
    QWidget* mainWindow = new QWidget();
    QVERIFY(m_uiManager->applyLayout("conference", mainWindow));
    
    // Add widgets
    mainWindow->layout()->addWidget(m_testButton);
    mainWindow->layout()->addWidget(m_testStatusBar);
    mainWindow->layout()->addWidget(m_testToolBar);
    
    // Apply theme
    QVERIFY(m_uiManager->setTheme("dark"));
    
    // Show and verify
    mainWindow->show();
    QApplication::processEvents();
    
    // Verify complete workflow
    QVERIFY(mainWindow->isVisible());
    QCOMPARE(m_uiManager->currentTheme(), QString("dark"));
    QCOMPARE(m_uiManager->currentLayout(), QString("conference"));
    
    // Verify all widgets are properly themed and laid out
    verifyThemeApplication(m_testButton, "dark");
    verifyThemeApplication(m_testStatusBar, "dark");
    verifyLayoutApplication(mainWindow, "conference");
    
    delete mainWindow;
}