#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

#include "../widgets/SettingsWidget.h"
#include "../widgets/PreferencesDialog.h"
#include "../widgets/ConfigEditor.h"
#include "../config/SettingsConfig.h"

class SettingsExampleWindow : public QMainWindow
{
    Q_OBJECT

public:
    SettingsExampleWindow(QWidget* parent = nullptr)
        : QMainWindow(parent)
    {
        setWindowTitle("Settings Module Example");
        setMinimumSize(800, 600);
        
        setupUI();
        setupExampleSettings();
    }

private slots:
    void showSettingsWidget()
    {
        SettingsWidget* widget = new SettingsWidget();
        widget->setWindowTitle("Settings Widget Example");
        widget->resize(600, 400);
        
        // Add some example settings
        addExampleSettings(widget);
        
        widget->show();
    }
    
    void showPreferencesDialog()
    {
        PreferencesDialog dialog(this);
        dialog.setWindowTitle("Preferences Dialog Example");
        dialog.exec();
    }
    
    void showConfigEditor()
    {
        ConfigEditor* editor = new ConfigEditor();
        editor->setWindowTitle("Config Editor Example");
        editor->resize(700, 500);
        
        // Set example JSON content
        QString exampleJson = R"({
    "application": {
        "name": "Jitsi Meet Qt",
        "version": "1.0.0",
        "debug": false
    },
    "audio": {
        "inputDevice": "default",
        "outputDevice": "default",
        "quality": "high"
    },
    "video": {
        "resolution": "720p",
        "frameRate": 30,
        "enabled": true
    }
})";
        
        editor->setConfigText(exampleJson);
        editor->show();
    }
    
    void testSettingsConfig()
    {
        SettingsConfig* config = SettingsConfig::instance();
        
        // Test basic functionality
        config->setConfigVersion("2.0.0");
        config->setStorageBackend("local");
        config->setEncryptionEnabled(true);
        config->setValidationEnabled(true);
        
        // Validate configuration
        auto result = config->validateConfiguration();
        bool isValid = result.first;
        QStringList errors = result.second;
        
        QString message;
        if (isValid) {
            message = "Settings configuration is valid!";
        } else {
            message = "Settings configuration has errors:\n" + errors.join("\n");
        }
        
        QMessageBox::information(this, "Settings Config Test", message);
    }

private:
    void setupUI()
    {
        QWidget* centralWidget = new QWidget();
        setCentralWidget(centralWidget);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
        
        // Title
        QLabel* titleLabel = new QLabel("Settings Module Examples");
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Description
        QLabel* descLabel = new QLabel(
            "This example demonstrates the Settings Module components:\n"
            "• SettingsWidget: Configurable settings interface\n"
            "• PreferencesDialog: User-friendly preferences dialog\n"
            "• ConfigEditor: Advanced configuration editor\n"
            "• SettingsConfig: Configuration management"
        );
        descLabel->setWordWrap(true);
        descLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(descLabel);
        
        mainLayout->addSpacing(20);
        
        // Buttons
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        
        QPushButton* settingsWidgetBtn = new QPushButton("Show Settings Widget");
        connect(settingsWidgetBtn, &QPushButton::clicked, this, &SettingsExampleWindow::showSettingsWidget);
        buttonLayout->addWidget(settingsWidgetBtn);
        
        QPushButton* preferencesBtn = new QPushButton("Show Preferences Dialog");
        connect(preferencesBtn, &QPushButton::clicked, this, &SettingsExampleWindow::showPreferencesDialog);
        buttonLayout->addWidget(preferencesBtn);
        
        QPushButton* configEditorBtn = new QPushButton("Show Config Editor");
        connect(configEditorBtn, &QPushButton::clicked, this, &SettingsExampleWindow::showConfigEditor);
        buttonLayout->addWidget(configEditorBtn);
        
        QPushButton* testConfigBtn = new QPushButton("Test Settings Config");
        connect(testConfigBtn, &QPushButton::clicked, this, &SettingsExampleWindow::testSettingsConfig);
        buttonLayout->addWidget(testConfigBtn);
        
        mainLayout->addLayout(buttonLayout);
        mainLayout->addStretch();
    }
    
    void setupExampleSettings()
    {
        // Initialize settings config with example values
        SettingsConfig* config = SettingsConfig::instance();
        config->setConfigVersion("1.0.0");
        config->setStorageBackend(SettingsConfig::LocalFileBackend);
        config->setValidationEnabled(true);
        config->setAutoSyncEnabled(true);
    }
    
    void addExampleSettings(SettingsWidget* widget)
    {
        // Add categories
        widget->addCategory("general", "General", ":/icons/general.png");
        widget->addCategory("audio", "Audio", ":/icons/audio.png");
        widget->addCategory("video", "Video", ":/icons/video.png");
        widget->addCategory("network", "Network", ":/icons/network.png");
        
        // General settings
        SettingsWidget::SettingDescriptor appNameDesc;
        appNameDesc.key = "general.appName";
        appNameDesc.displayName = "Application Name";
        appNameDesc.description = "Name of the application";
        appNameDesc.type = SettingsWidget::StringSetting;
        appNameDesc.category = "general";
        appNameDesc.defaultValue = "Jitsi Meet Qt";
        appNameDesc.placeholder = "Enter application name";
        widget->addSetting(appNameDesc);
        
        SettingsWidget::SettingDescriptor debugModeDesc;
        debugModeDesc.key = "general.debugMode";
        debugModeDesc.displayName = "Debug Mode";
        debugModeDesc.description = "Enable debug logging";
        debugModeDesc.type = SettingsWidget::BooleanSetting;
        debugModeDesc.category = "general";
        debugModeDesc.defaultValue = false;
        debugModeDesc.isAdvanced = true;
        widget->addSetting(debugModeDesc);
        
        // Audio settings
        SettingsWidget::SettingDescriptor audioQualityDesc;
        audioQualityDesc.key = "audio.quality";
        audioQualityDesc.displayName = "Audio Quality";
        audioQualityDesc.description = "Audio encoding quality";
        audioQualityDesc.type = SettingsWidget::EnumSetting;
        audioQualityDesc.category = "audio";
        audioQualityDesc.defaultValue = "high";
        audioQualityDesc.enumValues = QStringList() << "low" << "medium" << "high" << "ultra";
        widget->addSetting(audioQualityDesc);
        
        SettingsWidget::SettingDescriptor volumeDesc;
        volumeDesc.key = "audio.volume";
        volumeDesc.displayName = "Master Volume";
        volumeDesc.description = "Master audio volume (0-100)";
        volumeDesc.type = SettingsWidget::IntegerSetting;
        volumeDesc.category = "audio";
        volumeDesc.defaultValue = 75;
        volumeDesc.minValue = 0;
        volumeDesc.maxValue = 100;
        widget->addSetting(volumeDesc);
        
        // Video settings
        SettingsWidget::SettingDescriptor resolutionDesc;
        resolutionDesc.key = "video.resolution";
        resolutionDesc.displayName = "Video Resolution";
        resolutionDesc.description = "Video capture resolution";
        resolutionDesc.type = SettingsWidget::EnumSetting;
        resolutionDesc.category = "video";
        resolutionDesc.defaultValue = "720p";
        resolutionDesc.enumValues = QStringList() << "480p" << "720p" << "1080p" << "4K";
        widget->addSetting(resolutionDesc);
        
        SettingsWidget::SettingDescriptor frameRateDesc;
        frameRateDesc.key = "video.frameRate";
        frameRateDesc.displayName = "Frame Rate";
        frameRateDesc.description = "Video frame rate (FPS)";
        frameRateDesc.type = SettingsWidget::IntegerSetting;
        frameRateDesc.category = "video";
        frameRateDesc.defaultValue = 30;
        frameRateDesc.minValue = 15;
        frameRateDesc.maxValue = 60;
        widget->addSetting(frameRateDesc);
        
        // Network settings
        SettingsWidget::SettingDescriptor serverUrlDesc;
        serverUrlDesc.key = "network.serverUrl";
        serverUrlDesc.displayName = "Server URL";
        serverUrlDesc.description = "Jitsi Meet server URL";
        serverUrlDesc.type = SettingsWidget::StringSetting;
        serverUrlDesc.category = "network";
        serverUrlDesc.defaultValue = "https://meet.jit.si";
        serverUrlDesc.placeholder = "https://your-server.com";
        widget->addSetting(serverUrlDesc);
        
        SettingsWidget::SettingDescriptor timeoutDesc;
        timeoutDesc.key = "network.timeout";
        timeoutDesc.displayName = "Connection Timeout";
        timeoutDesc.description = "Network connection timeout in seconds";
        timeoutDesc.type = SettingsWidget::IntegerSetting;
        timeoutDesc.category = "network";
        timeoutDesc.defaultValue = 30;
        timeoutDesc.minValue = 5;
        timeoutDesc.maxValue = 120;
        timeoutDesc.isAdvanced = true;
        widget->addSetting(timeoutDesc);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("Settings Module Example");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Jitsi Meet Qt");
    
    SettingsExampleWindow window;
    window.show();
    
    return app.exec();
}

#include "SettingsWidgetExample.moc"