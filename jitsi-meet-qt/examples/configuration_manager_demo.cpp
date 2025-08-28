#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QListWidget>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QTextEdit>
#include <QMessageBox>
#include <QSplitter>
#include <QDebug>

#include "ConfigurationManager.h"
#include "models/ApplicationSettings.h"

/**
 * @brief 配置管理器演示窗口
 */
class ConfigurationManagerDemo : public QMainWindow
{
    Q_OBJECT

public:
    ConfigurationManagerDemo(QWidget* parent = nullptr)
        : QMainWindow(parent)
        , m_configManager(new ConfigurationManager(this))
    {
        setupUI();
        connectSignals();
        loadCurrentConfiguration();
        
        setWindowTitle("Configuration Manager Demo");
        resize(800, 600);
    }

private slots:
    void onLoadConfiguration()
    {
        ApplicationSettings config = m_configManager->loadConfiguration();
        displayConfiguration(config);
        m_logOutput->append("Configuration loaded successfully");
    }
    
    void onSaveConfiguration()
    {
        ApplicationSettings config = getConfigurationFromUI();
        m_configManager->saveConfiguration(config);
        m_logOutput->append("Configuration saved successfully");
    }
    
    void onResetToDefaults()
    {
        m_configManager->resetToDefaults();
        loadCurrentConfiguration();
        m_logOutput->append("Configuration reset to defaults");
    }
    
    void onValidateConfiguration()
    {
        bool isValid = m_configManager->validateConfiguration();
        QString message = isValid ? "Configuration is valid" : "Configuration is invalid";
        m_logOutput->append(message);
        
        ApplicationSettings config = m_configManager->currentConfiguration();
        m_logOutput->append(QString("Configuration details: %1").arg(config.toString()));
    }
    
    void onAddRecentUrl()
    {
        QString url = m_newUrlEdit->text().trimmed();
        if (!url.isEmpty()) {
            m_configManager->addRecentUrl(url);
            updateRecentUrlsList();
            m_newUrlEdit->clear();
            m_logOutput->append(QString("Added recent URL: %1").arg(url));
        }
    }
    
    void onClearRecentUrls()
    {
        m_configManager->clearRecentUrls();
        updateRecentUrlsList();
        m_logOutput->append("Recent URLs cleared");
    }
    
    void onRemoveSelectedUrl()
    {
        QListWidgetItem* item = m_recentUrlsList->currentItem();
        if (item) {
            QString url = item->text();
            // Note: ConfigurationManager doesn't have removeRecentUrl method in the current implementation
            // This would need to be added to the interface
            m_logOutput->append(QString("Would remove URL: %1 (method not implemented)").arg(url));
        }
    }
    
    void onTestServerUrl()
    {
        QString url = m_serverUrlEdit->text().trimmed();
        if (url.isEmpty()) {
            QMessageBox::warning(this, "Warning", "Please enter a server URL");
            return;
        }
        
        // Test URL validation
        m_configManager->setServerUrl(url);
        QString actualUrl = m_configManager->serverUrl();
        
        if (actualUrl == url) {
            m_logOutput->append(QString("Server URL is valid: %1").arg(url));
            QMessageBox::information(this, "Success", "Server URL is valid!");
        } else {
            m_logOutput->append(QString("Server URL is invalid: %1").arg(url));
            QMessageBox::warning(this, "Invalid URL", "The server URL format is invalid");
        }
    }
    
    void onConfigurationChanged()
    {
        m_logOutput->append("Configuration changed signal received");
        updateStatusLabel();
    }
    
    void onServerUrlChanged(const QString& url)
    {
        m_logOutput->append(QString("Server URL changed to: %1").arg(url));
    }
    
    void onLanguageChanged(const QString& language)
    {
        m_logOutput->append(QString("Language changed to: %1").arg(language));
    }
    
    void onDarkModeChanged(bool darkMode)
    {
        m_logOutput->append(QString("Dark mode changed to: %1").arg(darkMode ? "enabled" : "disabled"));
    }

private:
    void setupUI()
    {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
        
        // 左侧配置面板
        QWidget* configPanel = createConfigurationPanel();
        splitter->addWidget(configPanel);
        
        // 右侧日志面板
        QWidget* logPanel = createLogPanel();
        splitter->addWidget(logPanel);
        
        splitter->setStretchFactor(0, 2);
        splitter->setStretchFactor(1, 1);
        
        QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
        mainLayout->addWidget(splitter);
        
        // 状态栏
        m_statusLabel = new QLabel("Ready");
        statusBar()->addWidget(m_statusLabel);
    }
    
    QWidget* createConfigurationPanel()
    {
        QWidget* panel = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(panel);
        
        // 服务器配置组
        QGroupBox* serverGroup = new QGroupBox("Server Configuration");
        QFormLayout* serverLayout = new QFormLayout(serverGroup);
        
        m_serverUrlEdit = new QLineEdit();
        m_serverTimeoutSpin = new QSpinBox();
        m_serverTimeoutSpin->setRange(1, 300);
        m_serverTimeoutSpin->setSuffix(" seconds");
        
        QPushButton* testUrlBtn = new QPushButton("Test URL");
        connect(testUrlBtn, &QPushButton::clicked, this, &ConfigurationManagerDemo::onTestServerUrl);
        
        QHBoxLayout* urlLayout = new QHBoxLayout();
        urlLayout->addWidget(m_serverUrlEdit);
        urlLayout->addWidget(testUrlBtn);
        
        serverLayout->addRow("Server URL:", urlLayout);
        serverLayout->addRow("Timeout:", m_serverTimeoutSpin);
        
        // 界面配置组
        QGroupBox* uiGroup = new QGroupBox("UI Configuration");
        QFormLayout* uiLayout = new QFormLayout(uiGroup);
        
        m_languageCombo = new QComboBox();
        m_languageCombo->addItems({"auto", "en", "zh-CN", "ja", "es", "fr", "de"});
        
        m_darkModeCheck = new QCheckBox();
        
        uiLayout->addRow("Language:", m_languageCombo);
        uiLayout->addRow("Dark Mode:", m_darkModeCheck);
        
        // 功能配置组
        QGroupBox* featureGroup = new QGroupBox("Feature Configuration");
        QFormLayout* featureLayout = new QFormLayout(featureGroup);
        
        m_autoJoinAudioCheck = new QCheckBox();
        m_autoJoinVideoCheck = new QCheckBox();
        m_rememberWindowStateCheck = new QCheckBox();
        m_maxRecentItemsSpin = new QSpinBox();
        m_maxRecentItemsSpin->setRange(1, 100);
        
        featureLayout->addRow("Auto Join Audio:", m_autoJoinAudioCheck);
        featureLayout->addRow("Auto Join Video:", m_autoJoinVideoCheck);
        featureLayout->addRow("Remember Window State:", m_rememberWindowStateCheck);
        featureLayout->addRow("Max Recent Items:", m_maxRecentItemsSpin);
        
        // 最近URL管理组
        QGroupBox* recentGroup = new QGroupBox("Recent URLs Management");
        QVBoxLayout* recentLayout = new QVBoxLayout(recentGroup);
        
        m_recentUrlsList = new QListWidget();
        m_recentUrlsList->setMaximumHeight(150);
        
        QHBoxLayout* urlManagementLayout = new QHBoxLayout();
        m_newUrlEdit = new QLineEdit();
        m_newUrlEdit->setPlaceholderText("Enter URL to add...");
        QPushButton* addUrlBtn = new QPushButton("Add");
        QPushButton* removeUrlBtn = new QPushButton("Remove Selected");
        QPushButton* clearUrlsBtn = new QPushButton("Clear All");
        
        connect(addUrlBtn, &QPushButton::clicked, this, &ConfigurationManagerDemo::onAddRecentUrl);
        connect(removeUrlBtn, &QPushButton::clicked, this, &ConfigurationManagerDemo::onRemoveSelectedUrl);
        connect(clearUrlsBtn, &QPushButton::clicked, this, &ConfigurationManagerDemo::onClearRecentUrls);
        
        urlManagementLayout->addWidget(m_newUrlEdit);
        urlManagementLayout->addWidget(addUrlBtn);
        urlManagementLayout->addWidget(removeUrlBtn);
        urlManagementLayout->addWidget(clearUrlsBtn);
        
        recentLayout->addWidget(m_recentUrlsList);
        recentLayout->addLayout(urlManagementLayout);
        
        // 操作按钮
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* loadBtn = new QPushButton("Load Configuration");
        QPushButton* saveBtn = new QPushButton("Save Configuration");
        QPushButton* resetBtn = new QPushButton("Reset to Defaults");
        QPushButton* validateBtn = new QPushButton("Validate Configuration");
        
        connect(loadBtn, &QPushButton::clicked, this, &ConfigurationManagerDemo::onLoadConfiguration);
        connect(saveBtn, &QPushButton::clicked, this, &ConfigurationManagerDemo::onSaveConfiguration);
        connect(resetBtn, &QPushButton::clicked, this, &ConfigurationManagerDemo::onResetToDefaults);
        connect(validateBtn, &QPushButton::clicked, this, &ConfigurationManagerDemo::onValidateConfiguration);
        
        buttonLayout->addWidget(loadBtn);
        buttonLayout->addWidget(saveBtn);
        buttonLayout->addWidget(resetBtn);
        buttonLayout->addWidget(validateBtn);
        
        // 添加所有组到主布局
        layout->addWidget(serverGroup);
        layout->addWidget(uiGroup);
        layout->addWidget(featureGroup);
        layout->addWidget(recentGroup);
        layout->addLayout(buttonLayout);
        layout->addStretch();
        
        return panel;
    }
    
    QWidget* createLogPanel()
    {
        QWidget* panel = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(panel);
        
        QLabel* logLabel = new QLabel("Activity Log:");
        m_logOutput = new QTextEdit();
        m_logOutput->setReadOnly(true);
        m_logOutput->setMaximumHeight(200);
        
        QPushButton* clearLogBtn = new QPushButton("Clear Log");
        connect(clearLogBtn, &QPushButton::clicked, m_logOutput, &QTextEdit::clear);
        
        layout->addWidget(logLabel);
        layout->addWidget(m_logOutput);
        layout->addWidget(clearLogBtn);
        
        return panel;
    }
    
    void connectSignals()
    {
        connect(m_configManager, &ConfigurationManager::configurationChanged,
                this, &ConfigurationManagerDemo::onConfigurationChanged);
        connect(m_configManager, &ConfigurationManager::serverUrlChanged,
                this, &ConfigurationManagerDemo::onServerUrlChanged);
        connect(m_configManager, &ConfigurationManager::languageChanged,
                this, &ConfigurationManagerDemo::onLanguageChanged);
        connect(m_configManager, &ConfigurationManager::darkModeChanged,
                this, &ConfigurationManagerDemo::onDarkModeChanged);
    }
    
    void loadCurrentConfiguration()
    {
        ApplicationSettings config = m_configManager->currentConfiguration();
        displayConfiguration(config);
        updateRecentUrlsList();
        updateStatusLabel();
    }
    
    void displayConfiguration(const ApplicationSettings& config)
    {
        m_serverUrlEdit->setText(config.defaultServerUrl);
        m_serverTimeoutSpin->setValue(config.serverTimeout);
        
        int languageIndex = m_languageCombo->findText(config.language);
        if (languageIndex >= 0) {
            m_languageCombo->setCurrentIndex(languageIndex);
        }
        
        m_darkModeCheck->setChecked(config.darkMode);
        m_autoJoinAudioCheck->setChecked(config.autoJoinAudio);
        m_autoJoinVideoCheck->setChecked(config.autoJoinVideo);
        m_rememberWindowStateCheck->setChecked(config.rememberWindowState);
        m_maxRecentItemsSpin->setValue(config.maxRecentItems);
    }
    
    ApplicationSettings getConfigurationFromUI()
    {
        ApplicationSettings config;
        
        config.defaultServerUrl = m_serverUrlEdit->text().trimmed();
        config.serverTimeout = m_serverTimeoutSpin->value();
        config.language = m_languageCombo->currentText();
        config.darkMode = m_darkModeCheck->isChecked();
        config.autoJoinAudio = m_autoJoinAudioCheck->isChecked();
        config.autoJoinVideo = m_autoJoinVideoCheck->isChecked();
        config.rememberWindowState = m_rememberWindowStateCheck->isChecked();
        config.maxRecentItems = m_maxRecentItemsSpin->value();
        config.recentUrls = m_configManager->recentUrls(); // 保持现有的最近URL列表
        
        // 使用当前窗口几何信息
        config.windowGeometry = geometry();
        config.maximized = isMaximized();
        
        return config;
    }
    
    void updateRecentUrlsList()
    {
        m_recentUrlsList->clear();
        QStringList recentUrls = m_configManager->recentUrls();
        for (const QString& url : recentUrls) {
            m_recentUrlsList->addItem(url);
        }
    }
    
    void updateStatusLabel()
    {
        ApplicationSettings config = m_configManager->currentConfiguration();
        QString status = QString("Server: %1 | Language: %2 | Recent URLs: %3")
                        .arg(config.defaultServerUrl)
                        .arg(config.language)
                        .arg(config.recentUrls.size());
        m_statusLabel->setText(status);
    }

private:
    ConfigurationManager* m_configManager;
    
    // UI组件
    QLineEdit* m_serverUrlEdit;
    QSpinBox* m_serverTimeoutSpin;
    QComboBox* m_languageCombo;
    QCheckBox* m_darkModeCheck;
    QCheckBox* m_autoJoinAudioCheck;
    QCheckBox* m_autoJoinVideoCheck;
    QCheckBox* m_rememberWindowStateCheck;
    QSpinBox* m_maxRecentItemsSpin;
    
    QListWidget* m_recentUrlsList;
    QLineEdit* m_newUrlEdit;
    
    QTextEdit* m_logOutput;
    QLabel* m_statusLabel;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Configuration Manager Demo");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Jitsi Meet Qt");
    
    ConfigurationManagerDemo demo;
    demo.show();
    
    return app.exec();
}

#include "configuration_manager_demo.moc"