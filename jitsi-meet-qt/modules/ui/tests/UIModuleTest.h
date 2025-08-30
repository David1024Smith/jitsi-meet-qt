#ifndef UIMODULETEST_H
#define UIMODULETEST_H

#include <QtTest/QtTest>
#include <QObject>
#include <QApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

// UI Module includes
#include "../include/UIModule.h"
#include "../include/UIManager.h"
#include "../include/ThemeManager.h"
#include "../include/ThemeFactory.h"
#include "../config/UIConfig.h"

// Theme includes
#include "../themes/BaseTheme.h"
#include "../themes/DefaultTheme.h"
#include "../themes/DarkTheme.h"
#include "../themes/LightTheme.h"

// Widget includes
#include "../widgets/BaseWidget.h"
#include "../widgets/CustomButton.h"
#include "../widgets/StatusBar.h"
#include "../widgets/ToolBar.h"

// Layout includes
#include "../layouts/MainLayout.h"
#include "../layouts/ConferenceLayout.h"
#include "../layouts/SettingsLayout.h"

// Interface includes
#include "../interfaces/IUIManager.h"
#include "../interfaces/IThemeManager.h"
#include "../interfaces/ILayoutManager.h"

/**
 * @brief Comprehensive test suite for the UI Module
 * 
 * This test class covers all aspects of the UI module including:
 * - Theme switching and management
 * - Component rendering and styling
 * - Layout management and responsive design
 * - UI configuration management
 * - Compatibility with existing interface components
 */
class UIModuleTest : public QObject
{
    Q_OBJECT

public:
    UIModuleTest();
    ~UIModuleTest();

private slots:
    // Test lifecycle
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Core module tests
    void testUIModule_Initialization_Success();
    void testUIModule_Configuration_ValidData();
    void testUIModule_Status_ReportsCorrectly();
    void testUIModule_ErrorHandling_WorksCorrectly();

    // UI Manager tests
    void testUIManager_Initialization_Success();
    void testUIManager_ThemeManagement_WorksCorrectly();
    void testUIManager_LayoutManagement_WorksCorrectly();
    void testUIManager_WidgetRegistration_WorksCorrectly();

    // Theme switching tests
    void testThemeManager_DefaultTheme_AppliesCorrectly();
    void testThemeManager_DarkTheme_AppliesCorrectly();
    void testThemeManager_LightTheme_AppliesCorrectly();
    void testThemeManager_CustomTheme_AppliesCorrectly();
    void testThemeManager_ThemeSwitch_SignalsEmitted();
    void testThemeManager_InvalidTheme_HandledGracefully();

    // Component rendering tests
    void testComponentRendering_BaseWidget_RendersCorrectly();
    void testComponentRendering_CustomButton_RendersCorrectly();
    void testComponentRendering_StatusBar_RendersCorrectly();
    void testComponentRendering_ToolBar_RendersCorrectly();
    void testComponentRendering_ThemeApplication_WorksCorrectly();
    void testComponentRendering_StyleSheetApplication_WorksCorrectly();

    // Layout management tests
    void testLayoutManager_MainLayout_WorksCorrectly();
    void testLayoutManager_ConferenceLayout_WorksCorrectly();
    void testLayoutManager_SettingsLayout_WorksCorrectly();
    void testLayoutManager_LayoutSwitch_WorksCorrectly();
    void testLayoutManager_LayoutPersistence_WorksCorrectly();

    // Responsive design tests
    void testResponsiveDesign_WindowResize_LayoutAdjusts();
    void testResponsiveDesign_ScreenDPI_ScalingWorks();
    void testResponsiveDesign_FontScaling_WorksCorrectly();
    void testResponsiveDesign_ComponentScaling_WorksCorrectly();

    // UI configuration tests
    void testUIConfig_Creation_Success();
    void testUIConfig_ThemeConfiguration_WorksCorrectly();
    void testUIConfig_LayoutConfiguration_WorksCorrectly();
    void testUIConfig_Serialization_WorksCorrectly();
    void testUIConfig_Validation_WorksCorrectly();
    void testUIConfig_Persistence_WorksCorrectly();

    // Compatibility tests
    void testCompatibility_ExistingWidgets_IntegrateCorrectly();
    void testCompatibility_ExistingLayouts_IntegrateCorrectly();
    void testCompatibility_ExistingThemes_IntegrateCorrectly();
    void testCompatibility_LegacyAPI_WorksCorrectly();

    // Performance tests
    void testPerformance_ThemeSwitch_CompletesQuickly();
    void testPerformance_LayoutSwitch_CompletesQuickly();
    void testPerformance_ComponentCreation_CompletesQuickly();
    void testPerformance_MemoryUsage_WithinLimits();

    // Error handling tests
    void testErrorHandling_InvalidTheme_HandledGracefully();
    void testErrorHandling_InvalidLayout_HandledGracefully();
    void testErrorHandling_InvalidConfiguration_HandledGracefully();
    void testErrorHandling_ResourceNotFound_HandledGracefully();

    // Integration tests
    void testIntegration_ThemeAndLayout_WorkTogether();
    void testIntegration_ConfigAndTheme_WorkTogether();
    void testIntegration_WidgetsAndLayout_WorkTogether();
    void testIntegration_FullUIWorkflow_WorksCorrectly();

private:
    // Helper methods
    void setupTestEnvironment();
    void cleanupTestEnvironment();
    void createTestWidgets();
    void destroyTestWidgets();
    void waitForSignal(QObject* sender, const char* signal, int timeout = 5000);
    bool compareThemes(BaseTheme* theme1, BaseTheme* theme2);
    void verifyThemeApplication(BaseWidget* widget, const QString& themeName);
    void verifyLayoutApplication(QWidget* container, const QString& layoutName);
    QSize getOptimalSize(QWidget* widget);
    void simulateWindowResize(QWidget* window, const QSize& newSize);

    // Test data
    UIModule* m_uiModule;
    UIManager* m_uiManager;
    ThemeManager* m_themeManager;
    UIConfig* m_uiConfig;
    
    // Test widgets
    BaseWidget* m_testBaseWidget;
    CustomButton* m_testButton;
    StatusBar* m_testStatusBar;
    ToolBar* m_testToolBar;
    
    // Test layouts
    MainLayout* m_testMainLayout;
    ConferenceLayout* m_testConferenceLayout;
    SettingsLayout* m_testSettingsLayout;
    
    // Test themes
    DefaultTheme* m_defaultTheme;
    DarkTheme* m_darkTheme;
    LightTheme* m_lightTheme;
    
    // Test containers
    QWidget* m_testWindow;
    QVBoxLayout* m_testLayout;
    
    // Test configuration
    static const int PERFORMANCE_TIMEOUT = 1000; // ms
    static const int SIGNAL_TIMEOUT = 5000; // ms
    static const size_t MAX_MEMORY_USAGE = 50 * 1024 * 1024; // 50MB
};

#endif // UIMODULETEST_H