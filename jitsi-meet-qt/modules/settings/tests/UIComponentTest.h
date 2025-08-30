#ifndef UICOMPONENTTEST_H
#define UICOMPONENTTEST_H

#include <QObject>
#include <QTest>
#include <QElapsedTimer>

class QApplication;

/**
 * @brief UI组件测试类
 * 
 * 专门测试设置模块的UI组件，包括SettingsWidget、PreferencesDialog和ConfigEditor。
 * 测试组件初始化、用户交互、数据绑定、验证和性能等方面。
 */
class UIComponentTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // SettingsWidget tests
    void testSettingsWidgetInitialization();
    void testSettingsWidgetValueManagement();
    void testSettingsWidgetCategories();
    void testSettingsWidgetValidation();
    
    // PreferencesDialog tests
    void testPreferencesDialogInitialization();
    void testPreferencesDialogCategoryManagement();
    void testPreferencesDialogPreferenceManagement();
    void testPreferencesDialogUserInteraction();
    
    // ConfigEditor tests
    void testConfigEditorInitialization();
    void testConfigEditorConfigurationManagement();
    void testConfigEditorEditing();
    void testConfigEditorValidation();
    
    // Integration tests
    void testUIComponentIntegration();
    void testUIComponentPerformance();

private:
    QApplication* m_app = nullptr;
};

#endif // UICOMPONENTTEST_H