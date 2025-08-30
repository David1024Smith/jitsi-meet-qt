#include "UIComponentTest.h"
#include "SettingsWidget.h"
#include "PreferencesDialog.h"
#include "ConfigEditor.h"

#include <QApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTableWidget>

void UIComponentTest::initTestCase()
{
    // Ensure we have a QApplication instance
    if (!QApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        m_app = new QApplication(argc, argv);
    }
}

void UIComponentTest::cleanupTestCase()
{
    if (m_app) {
        delete m_app;
        m_app = nullptr;
    }
}

void UIComponentTest::init()
{
    // Setup for each test
}

void UIComponentTest::cleanup()
{
    // Cleanup after each test
}

void UIComponentTest::testSettingsWidgetInitialization()
{
    SettingsWidget widget;
    
    // Test initialization
    QVERIFY(widget.initialize());
    QVERIFY(widget.isInitialized());
    
    // Test widget properties
    QVERIFY(!widget.windowTitle().isEmpty());
    QVERIFY(widget.layout() != nullptr);
    
    // Test default state
    QVERIFY(widget.validate());
    QVERIFY(!widget.hasUnsavedChanges());
}

void UIComponentTest::testSettingsWidgetValueManagement()
{
    SettingsWidget widget;
    QVERIFY(widget.initialize());
    
    // Test setting and getting values
    widget.setValue("audio/volume", 75);
    QCOMPARE(widget.value("audio/volume").toInt(), 75);
    
    widget.setValue("video/enabled", true);
    QCOMPARE(widget.value("video/enabled").toBool(), true);
    
    widget.setValue("network/server", "https://meet.jit.si");
    QCOMPARE(widget.value("network/server").toString(), QString("https://meet.jit.si"));
    
    // Test value change signals
    QSignalSpy spy(&widget, &SettingsWidget::valueChanged);
    widget.setValue("test/signal", "signal_test");
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QString("test/signal"));
    QCOMPARE(arguments.at(1).toString(), QString("signal_test"));
    
    // Test unsaved changes tracking
    QVERIFY(widget.hasUnsavedChanges());
}

void UIComponentTest::testSettingsWidgetCategories()
{
    SettingsWidget widget;
    QVERIFY(widget.initialize());
    
    // Test adding categories
    widget.addCategory("Audio", "Audio Settings", ":/icons/audio.png");
    widget.addCategory("Video", "Video Settings", ":/icons/video.png");
    widget.addCategory("Network", "Network Settings", ":/icons/network.png");
    
    // Test category retrieval
    QStringList categories = widget.categories();
    QVERIFY(categories.contains("Audio"));
    QVERIFY(categories.contains("Video"));
    QVERIFY(categories.contains("Network"));
    
    // Test category selection
    widget.selectCategory("Audio");
    QCOMPARE(widget.currentCategory(), QString("Audio"));
    
    // Test category change signal
    QSignalSpy spy(&widget, &SettingsWidget::categoryChanged);
    widget.selectCategory("Video");
    
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toString(), QString("Video"));
}

void UIComponentTest::testSettingsWidgetValidation()
{
    SettingsWidget widget;
    QVERIFY(widget.initialize());
    
    // Add validation rules
    widget.addValidationRule("audio/volume", [](const QVariant& value) {
        int vol = value.toInt();
        return vol >= 0 && vol <= 100;
    });
    
    widget.addValidationRule("network/port", [](const QVariant& value) {
        int port = value.toInt();
        return port >= 1024 && port <= 65535;
    });
    
    // Test valid values
    widget.setValue("audio/volume", 50);
    widget.setValue("network/port", 8080);
    QVERIFY(widget.validate());
    
    // Test invalid values
    widget.setValue("audio/volume", 150); // Out of range
    QVERIFY(!widget.validate());
    
    QStringList errors = widget.validationErrors();
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.join(" ").contains("audio/volume"));
    
    // Fix the error and test again
    widget.setValue("audio/volume", 75);
    QVERIFY(widget.validate());
}

void UIComponentTest::testPreferencesDialogInitialization()
{
    PreferencesDialog dialog;
    
    // Test initialization
    QVERIFY(dialog.initialize());
    QVERIFY(dialog.isInitialized());
    
    // Test dialog properties
    QVERIFY(!dialog.windowTitle().isEmpty());
    QVERIFY(dialog.layout() != nullptr);
    
    // Test modal behavior
    QVERIFY(dialog.isModal());
}

void UIComponentTest::testPreferencesDialogCategoryManagement()
{
    PreferencesDialog dialog;
    QVERIFY(dialog.initialize());
    
    // Test adding categories
    dialog.addCategory("Audio", "Audio Preferences");
    dialog.addCategory("Video", "Video Preferences");
    dialog.addCategory("UI", "User Interface");
    dialog.addCategory("Advanced", "Advanced Settings");
    
    // Test category retrieval
    QStringList categories = dialog.categories();
    QCOMPARE(categories.size(), 4);
    QVERIFY(categories.contains("Audio"));
    QVERIFY(categories.contains("Video"));
    QVERIFY(categories.contains("UI"));
    QVERIFY(categories.contains("Advanced"));
    
    // Test category descriptions
    QCOMPARE(dialog.categoryDescription("Audio"), QString("Audio Preferences"));
    QCOMPARE(dialog.categoryDescription("Video"), QString("Video Preferences"));
    
    // Test category removal
    dialog.removeCategory("Advanced");
    categories = dialog.categories();
    QCOMPARE(categories.size(), 3);
    QVERIFY(!categories.contains("Advanced"));
}

void UIComponentTest::testPreferencesDialogPreferenceManagement()
{
    PreferencesDialog dialog;
    QVERIFY(dialog.initialize());
    
    dialog.addCategory("Audio", "Audio Preferences");
    dialog.addCategory("Video", "Video Preferences");
    
    // Test setting preferences
    dialog.setPreference("Audio", "volume", 75);
    dialog.setPreference("Audio", "muted", false);
    dialog.setPreference("Video", "resolution", "1920x1080");
    dialog.setPreference("Video", "fps", 30);
    
    // Test getting preferences
    QCOMPARE(dialog.preference("Audio", "volume").toInt(), 75);
    QCOMPARE(dialog.preference("Audio", "muted").toBool(), false);
    QCOMPARE(dialog.preference("Video", "resolution").toString(), QString("1920x1080"));
    QCOMPARE(dialog.preference("Video", "fps").toInt(), 30);
    
    // Test preference change signals
    QSignalSpy spy(&dialog, &PreferencesDialog::preferenceChanged);
    dialog.setPreference("Audio", "volume", 80);
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QString("Audio"));
    QCOMPARE(arguments.at(1).toString(), QString("volume"));
    QCOMPARE(arguments.at(2).toInt(), 80);
}

void UIComponentTest::testPreferencesDialogUserInteraction()
{
    PreferencesDialog dialog;
    QVERIFY(dialog.initialize());
    
    dialog.addCategory("Test", "Test Category");
    dialog.setPreference("Test", "value", 50);
    
    // Test dialog buttons
    QPushButton* okButton = dialog.findChild<QPushButton*>("okButton");
    QPushButton* cancelButton = dialog.findChild<QPushButton*>("cancelButton");
    QPushButton* applyButton = dialog.findChild<QPushButton*>("applyButton");
    QPushButton* resetButton = dialog.findChild<QPushButton*>("resetButton");
    
    QVERIFY(okButton != nullptr);
    QVERIFY(cancelButton != nullptr);
    QVERIFY(applyButton != nullptr);
    QVERIFY(resetButton != nullptr);
    
    // Test button states
    QVERIFY(okButton->isEnabled());
    QVERIFY(cancelButton->isEnabled());
    
    // Test apply button (should be disabled initially)
    QVERIFY(!applyButton->isEnabled());
    
    // Make a change and test apply button
    dialog.setPreference("Test", "value", 75);
    QVERIFY(applyButton->isEnabled());
    
    // Test reset functionality
    QSignalSpy resetSpy(&dialog, &PreferencesDialog::preferencesReset);
    resetButton->click();
    
    QCOMPARE(resetSpy.count(), 1);
}

void UIComponentTest::testConfigEditorInitialization()
{
    ConfigEditor editor;
    
    // Test initialization
    QVERIFY(editor.initialize());
    QVERIFY(editor.isInitialized());
    
    // Test editor properties
    QVERIFY(editor.layout() != nullptr);
    QVERIFY(!editor.isReadOnly());
}

void UIComponentTest::testConfigEditorConfigurationManagement()
{
    ConfigEditor editor;
    QVERIFY(editor.initialize());
    
    // Test loading configuration
    QVariantMap config;
    config["audio/volume"] = 75;
    config["audio/enabled"] = true;
    config["video/resolution"] = "1920x1080";
    config["video/fps"] = 30;
    config["network/server"] = "https://meet.jit.si";
    config["network/port"] = 443;
    
    editor.loadConfiguration(config);
    
    // Test configuration retrieval
    QVariantMap loadedConfig = editor.configuration();
    QCOMPARE(loadedConfig["audio/volume"].toInt(), 75);
    QCOMPARE(loadedConfig["audio/enabled"].toBool(), true);
    QCOMPARE(loadedConfig["video/resolution"].toString(), QString("1920x1080"));
    QCOMPARE(loadedConfig["video/fps"].toInt(), 30);
    QCOMPARE(loadedConfig["network/server"].toString(), QString("https://meet.jit.si"));
    QCOMPARE(loadedConfig["network/port"].toInt(), 443);
    
    // Test individual value access
    QCOMPARE(editor.value("audio/volume").toInt(), 75);
    QCOMPARE(editor.value("video/resolution").toString(), QString("1920x1080"));
}

void UIComponentTest::testConfigEditorEditing()
{
    ConfigEditor editor;
    QVERIFY(editor.initialize());
    
    // Load initial configuration
    QVariantMap config;
    config["test/string"] = "initial_value";
    config["test/integer"] = 100;
    editor.loadConfiguration(config);
    
    // Test value modification
    editor.setValue("test/string", "modified_value");
    editor.setValue("test/integer", 200);
    editor.setValue("test/new_key", "new_value");
    
    // Test change tracking
    QVERIFY(editor.hasUnsavedChanges());
    
    // Test change signals
    QSignalSpy spy(&editor, &ConfigEditor::valueChanged);
    editor.setValue("test/signal", "signal_value");
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QString("test/signal"));
    QCOMPARE(arguments.at(1).toString(), QString("signal_value"));
    
    // Test modified configuration
    QVariantMap modifiedConfig = editor.configuration();
    QCOMPARE(modifiedConfig["test/string"].toString(), QString("modified_value"));
    QCOMPARE(modifiedConfig["test/integer"].toInt(), 200);
    QCOMPARE(modifiedConfig["test/new_key"].toString(), QString("new_value"));
}

void UIComponentTest::testConfigEditorValidation()
{
    ConfigEditor editor;
    QVERIFY(editor.initialize());
    
    // Add validation rules
    editor.addValidationRule("test/range", [](const QVariant& value) {
        int val = value.toInt();
        return val >= 1 && val <= 10;
    });
    
    editor.addValidationRule("test/pattern", [](const QVariant& value) {
        QString str = value.toString();
        return str.startsWith("prefix_");
    });
    
    // Test valid configuration
    editor.setValue("test/range", 5);
    editor.setValue("test/pattern", "prefix_test");
    QVERIFY(editor.validate());
    
    // Test invalid configuration
    editor.setValue("test/range", 15); // Out of range
    QVERIFY(!editor.validate());
    
    QStringList errors = editor.validationErrors();
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.join(" ").contains("test/range"));
    
    // Fix error and test pattern validation
    editor.setValue("test/range", 3);
    editor.setValue("test/pattern", "invalid_pattern");
    QVERIFY(!editor.validate());
    
    errors = editor.validationErrors();
    QVERIFY(errors.join(" ").contains("test/pattern"));
}

void UIComponentTest::testUIComponentIntegration()
{
    // Test integration between UI components
    SettingsWidget widget;
    PreferencesDialog dialog;
    ConfigEditor editor;
    
    QVERIFY(widget.initialize());
    QVERIFY(dialog.initialize());
    QVERIFY(editor.initialize());
    
    // Test data sharing
    QVariantMap testData;
    testData["integration/test1"] = "value1";
    testData["integration/test2"] = 42;
    
    // Set data in widget
    widget.setValue("integration/test1", "value1");
    widget.setValue("integration/test2", 42);
    
    // Set data in dialog
    dialog.addCategory("Integration", "Integration Test");
    dialog.setPreference("Integration", "test1", "value1");
    dialog.setPreference("Integration", "test2", 42);
    
    // Load data in editor
    editor.loadConfiguration(testData);
    
    // Verify consistency
    QCOMPARE(widget.value("integration/test1").toString(), QString("value1"));
    QCOMPARE(dialog.preference("Integration", "test1").toString(), QString("value1"));
    QCOMPARE(editor.value("integration/test1").toString(), QString("value1"));
    
    QCOMPARE(widget.value("integration/test2").toInt(), 42);
    QCOMPARE(dialog.preference("Integration", "test2").toInt(), 42);
    QCOMPARE(editor.value("integration/test2").toInt(), 42);
}

void UIComponentTest::testUIComponentPerformance()
{
    SettingsWidget widget;
    QVERIFY(widget.initialize());
    
    QElapsedTimer timer;
    
    // Test widget performance with many values
    timer.start();
    for (int i = 0; i < 1000; ++i) {
        widget.setValue(QString("perf/key_%1").arg(i), QString("value_%1").arg(i));
    }
    qint64 setTime = timer.elapsed();
    
    timer.restart();
    for (int i = 0; i < 1000; ++i) {
        QVariant value = widget.value(QString("perf/key_%1").arg(i));
        Q_UNUSED(value);
    }
    qint64 getTime = timer.elapsed();
    
    // Performance assertions
    QVERIFY(setTime < 3000); // 3 seconds for 1000 sets
    QVERIFY(getTime < 1000); // 1 second for 1000 gets
    
    qDebug() << "UI Performance results:";
    qDebug() << "Set time:" << setTime << "ms";
    qDebug() << "Get time:" << getTime << "ms";
}

QTEST_MAIN(UIComponentTest)