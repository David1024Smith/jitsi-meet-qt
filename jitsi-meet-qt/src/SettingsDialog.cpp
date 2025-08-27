#include "SettingsDialog.h"
#include "ConfigurationManager.h"
#include "TranslationManager.h"
#include "models/ApplicationSettings.h"
#include <QApplication>
#include <QMessageBox>
#include <QUrl>
#include <QRegularExpression>
#include <QDebug>

SettingsDialog::SettingsDialog(ConfigurationManager* configManager,
                               TranslationManager* translationManager,
                               QWidget* parent)
    : QDialog(parent)
    , m_configManager(configManager)
    , m_translationManager(translationManager)
    , m_serverGroup(nullptr)
    , m_serverUrlEdit(nullptr)
    , m_serverTimeoutSpin(nullptr)
    , m_serverUrlLabel(nullptr)
    , m_serverTimeoutLabel(nullptr)
    , m_serverUrlStatusLabel(nullptr)
    , m_interfaceGroup(nullptr)
    , m_languageCombo(nullptr)
    , m_darkModeCheck(nullptr)
    , m_rememberWindowStateCheck(nullptr)
    , m_languageLabel(nullptr)
    , m_conferenceGroup(nullptr)
    , m_autoJoinAudioCheck(nullptr)
    , m_autoJoinVideoCheck(nullptr)
    , m_advancedGroup(nullptr)
    , m_maxRecentItemsSpin(nullptr)
    , m_clearRecentButton(nullptr)
    , m_maxRecentItemsLabel(nullptr)
    , m_buttonBox(nullptr)
    , m_applyButton(nullptr)
    , m_resetButton(nullptr)
    , m_mainLayout(nullptr)
{
    setupUI();
    setupConnections();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    // Qt handles cleanup automatically
}

void SettingsDialog::setupUI()
{
    setWindowTitle(tr("settings_title"));
    setModal(true);
    resize(500, 600);
    
    // Create main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    
    // Create groups
    m_mainLayout->addWidget(createServerGroup());
    m_mainLayout->addWidget(createInterfaceGroup());
    m_mainLayout->addWidget(createConferenceGroup());
    m_mainLayout->addWidget(createAdvancedGroup());
    
    // Create button box
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_applyButton = m_buttonBox->addButton(tr("apply_button"), QDialogButtonBox::ApplyRole);
    m_resetButton = m_buttonBox->addButton(tr("reset_button"), QDialogButtonBox::ResetRole);
    
    m_mainLayout->addWidget(m_buttonBox);
    
    // Update UI text
    updateUIText();
}

QGroupBox* SettingsDialog::createServerGroup()
{
    m_serverGroup = new QGroupBox(tr("server_settings"), this);
    QFormLayout* layout = new QFormLayout(m_serverGroup);
    
    // Server URL
    m_serverUrlLabel = new QLabel(tr("server_url_label"));
    m_serverUrlEdit = new QLineEdit();
    m_serverUrlStatusLabel = new QLabel();
    m_serverUrlStatusLabel->setStyleSheet("color: red; font-size: 11px;");
    m_serverUrlStatusLabel->hide();
    
    layout->addRow(m_serverUrlLabel, m_serverUrlEdit);
    layout->addRow("", m_serverUrlStatusLabel);
    
    // Server timeout
    m_serverTimeoutLabel = new QLabel(tr("server_timeout_label"));
    m_serverTimeoutSpin = new QSpinBox();
    m_serverTimeoutSpin->setRange(5, 120);
    m_serverTimeoutSpin->setSuffix(" " + tr("seconds"));
    
    layout->addRow(m_serverTimeoutLabel, m_serverTimeoutSpin);
    
    return m_serverGroup;
}

QGroupBox* SettingsDialog::createInterfaceGroup()
{
    m_interfaceGroup = new QGroupBox(tr("interface_settings"), this);
    QFormLayout* layout = new QFormLayout(m_interfaceGroup);
    
    // Language selection
    m_languageLabel = new QLabel(tr("language_label"));
    m_languageCombo = new QComboBox();
    
    // Populate language combo
    if (m_translationManager) {
        m_languageCombo->addItem(tr("auto_language"), "auto");
        
        QStringList languages = m_translationManager->availableLanguages();
        for (const QString& lang : languages) {
            QString displayName = m_translationManager->languageDisplayName(lang);
            m_languageCombo->addItem(displayName, lang);
        }
    }
    
    layout->addRow(m_languageLabel, m_languageCombo);
    
    // Dark mode
    m_darkModeCheck = new QCheckBox(tr("dark_mode"));
    layout->addRow("", m_darkModeCheck);
    
    // Remember window state
    m_rememberWindowStateCheck = new QCheckBox(tr("remember_window_state"));
    layout->addRow("", m_rememberWindowStateCheck);
    
    return m_interfaceGroup;
}

QGroupBox* SettingsDialog::createConferenceGroup()
{
    m_conferenceGroup = new QGroupBox(tr("conference_settings"), this);
    QVBoxLayout* layout = new QVBoxLayout(m_conferenceGroup);
    
    // Auto join audio
    m_autoJoinAudioCheck = new QCheckBox(tr("auto_join_audio"));
    layout->addWidget(m_autoJoinAudioCheck);
    
    // Auto join video
    m_autoJoinVideoCheck = new QCheckBox(tr("auto_join_video"));
    layout->addWidget(m_autoJoinVideoCheck);
    
    return m_conferenceGroup;
}

QGroupBox* SettingsDialog::createAdvancedGroup()
{
    m_advancedGroup = new QGroupBox(tr("advanced_settings"), this);
    QFormLayout* layout = new QFormLayout(m_advancedGroup);
    
    // Max recent items
    m_maxRecentItemsLabel = new QLabel(tr("max_recent_items"));
    m_maxRecentItemsSpin = new QSpinBox();
    m_maxRecentItemsSpin->setRange(0, 50);
    
    layout->addRow(m_maxRecentItemsLabel, m_maxRecentItemsSpin);
    
    // Clear recent button
    m_clearRecentButton = new QPushButton(tr("clear_recent_history"));
    layout->addRow("", m_clearRecentButton);
    
    return m_advancedGroup;
}

void SettingsDialog::setupConnections()
{
    // Button box connections
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::saveSettings);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::cancelSettings);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::applySettings);
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsDialog::restoreDefaults);
    
    // Input validation connections
    connect(m_serverUrlEdit, &QLineEdit::textChanged, this, &SettingsDialog::validateServerUrl);
    connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onLanguageChanged);
    
    // Clear recent button
    connect(m_clearRecentButton, &QPushButton::clicked, [this]() {
        if (m_configManager) {
            m_configManager->clearRecentItems();
            QMessageBox::information(this, tr("settings_title"), tr("recent_history_cleared"));
        }
    });
    
    // Translation manager connection
    if (m_translationManager) {
        connect(m_translationManager, &TranslationManager::languageChanged,
                this, &SettingsDialog::onTranslationChanged);
    }
}

void SettingsDialog::loadSettings()
{
    if (!m_configManager) {
        return;
    }
    
    ApplicationSettings settings = m_configManager->loadConfiguration();
    m_originalSettings = settings;
    
    // Load server settings
    m_serverUrlEdit->setText(settings.defaultServerUrl);
    m_serverTimeoutSpin->setValue(settings.serverTimeout);
    
    // Load interface settings
    QString currentLang = m_translationManager ? m_translationManager->currentLanguage() : "en";
    int langIndex = m_languageCombo->findData(currentLang);
    if (langIndex >= 0) {
        m_languageCombo->setCurrentIndex(langIndex);
    }
    
    m_darkModeCheck->setChecked(settings.darkMode);
    m_rememberWindowStateCheck->setChecked(settings.rememberWindowState);
    
    // Load conference settings
    m_autoJoinAudioCheck->setChecked(settings.autoJoinAudio);
    m_autoJoinVideoCheck->setChecked(settings.autoJoinVideo);
    
    // Load advanced settings
    m_maxRecentItemsSpin->setValue(settings.maxRecentItems);
}

void SettingsDialog::saveSettings()
{
    if (!validateInput()) {
        return;
    }
    
    applySettings();
    accept();
}

void SettingsDialog::cancelSettings()
{
    reject();
}

void SettingsDialog::applySettings()
{
    if (!m_configManager || !validateInput()) {
        return;
    }
    
    ApplicationSettings settings = getSettingsFromUI();
    m_configManager->saveConfiguration(settings);
    
    // Apply language change
    QString selectedLang = m_languageCombo->currentData().toString();
    if (m_translationManager && selectedLang != m_translationManager->currentLanguage()) {
        m_translationManager->setLanguage(selectedLang);
        emit languageChanged(selectedLang);
    }
    
    emit settingsSaved();
    
    // Update original settings
    m_originalSettings = settings;
}

void SettingsDialog::restoreDefaults()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("settings_title"),
        tr("restore_defaults_confirm"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (m_configManager) {
            m_configManager->resetToDefaults();
            loadSettings();
        }
    }
}

void SettingsDialog::validateServerUrl()
{
    QString url = m_serverUrlEdit->text().trimmed();
    
    if (url.isEmpty()) {
        m_serverUrlStatusLabel->setText(tr("invalid_server_url"));
        m_serverUrlStatusLabel->show();
        return;
    }
    
    QUrl qurl(url);
    if (!qurl.isValid() || (qurl.scheme() != "http" && qurl.scheme() != "https")) {
        m_serverUrlStatusLabel->setText(tr("invalid_server_url"));
        m_serverUrlStatusLabel->show();
    } else {
        m_serverUrlStatusLabel->hide();
    }
}

void SettingsDialog::onLanguageChanged()
{
    // Language change will be applied when settings are saved
}

void SettingsDialog::onTranslationChanged()
{
    updateUIText();
}

void SettingsDialog::updateUIText()
{
    setWindowTitle(tr("settings_title"));
    
    if (m_serverGroup) {
        m_serverGroup->setTitle(tr("server_settings"));
    }
    
    if (m_interfaceGroup) {
        m_interfaceGroup->setTitle(tr("interface_settings"));
    }
    
    if (m_conferenceGroup) {
        m_conferenceGroup->setTitle(tr("conference_settings"));
    }
    
    if (m_advancedGroup) {
        m_advancedGroup->setTitle(tr("advanced_settings"));
    }
    
    // Update button texts
    if (m_buttonBox) {
        m_buttonBox->button(QDialogButtonBox::Ok)->setText(tr("save_button"));
        m_buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("cancel_button"));
    }
    
    if (m_applyButton) {
        m_applyButton->setText(tr("apply_button"));
    }
    
    if (m_resetButton) {
        m_resetButton->setText(tr("reset_button"));
    }
}

ApplicationSettings SettingsDialog::getSettingsFromUI() const
{
    ApplicationSettings settings;
    
    // Server settings
    settings.defaultServerUrl = m_serverUrlEdit->text().trimmed();
    settings.serverTimeout = m_serverTimeoutSpin->value();
    
    // Interface settings
    settings.language = m_languageCombo->currentData().toString();
    settings.darkMode = m_darkModeCheck->isChecked();
    settings.rememberWindowState = m_rememberWindowStateCheck->isChecked();
    
    // Conference settings
    settings.autoJoinAudio = m_autoJoinAudioCheck->isChecked();
    settings.autoJoinVideo = m_autoJoinVideoCheck->isChecked();
    
    // Advanced settings
    settings.maxRecentItems = m_maxRecentItemsSpin->value();
    
    return settings;
}

bool SettingsDialog::validateInput()
{
    // Validate server URL
    QString url = m_serverUrlEdit->text().trimmed();
    if (url.isEmpty()) {
        showValidationError(tr("invalid_server_url"));
        return false;
    }
    
    QUrl qurl(url);
    if (!qurl.isValid() || (qurl.scheme() != "http" && qurl.scheme() != "https")) {
        showValidationError(tr("invalid_server_url"));
        return false;
    }
    
    return true;
}

void SettingsDialog::showValidationError(const QString& message)
{
    QMessageBox::warning(this, tr("settings_title"), message);
}

void SettingsDialog::showSettings()
{
    loadSettings();
    show();
    raise();
    activateWindow();
}

void SettingsDialog::resetToDefaults()
{
    restoreDefaults();
}