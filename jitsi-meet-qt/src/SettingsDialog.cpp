#include "SettingsDialog.h"
#include "ConfigurationManager.h"
#include "TranslationManager.h"
#include "MediaManager.h"
#include "models/ApplicationSettings.h"
#include <QApplication>
#include <QMessageBox>
#include <QUrl>
#include <QRegularExpression>
#include <QDebug>
#include <QVideoWidget>
#include <QTimer>
#include <random>

SettingsDialog::SettingsDialog(ConfigurationManager* configManager,
                               TranslationManager* translationManager,
                               MediaManager* mediaManager,
                               QWidget* parent)
    : QDialog(parent)
    , m_configManager(configManager)
    , m_translationManager(translationManager)
    , m_mediaManager(mediaManager)
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
    , m_mediaDeviceGroup(nullptr)
    , m_cameraCombo(nullptr)
    , m_microphoneCombo(nullptr)
    , m_speakerCombo(nullptr)
    , m_testCameraButton(nullptr)
    , m_testMicrophoneButton(nullptr)
    , m_testSpeakerButton(nullptr)
    , m_cameraPreview(nullptr)
    , m_microphoneLevel(nullptr)
    , m_microphoneVolumeSlider(nullptr)
    , m_speakerVolumeSlider(nullptr)
    , m_cameraLabel(nullptr)
    , m_microphoneLabel(nullptr)
    , m_speakerLabel(nullptr)
    , m_microphoneVolumeLabel(nullptr)
    , m_speakerVolumeLabel(nullptr)
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
    , m_cameraTestActive(false)
    , m_microphoneTestActive(false)
    , m_speakerTestActive(false)
{
    setupUI();
    setupConnections();
    initializeDeviceLists();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    stopAllTests();
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
    m_mainLayout->addWidget(createMediaDeviceGroup());
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
        
        auto availableLanguages = m_translationManager->availableLanguages();
        for (const auto& langInfo : availableLanguages) {
            QString displayName = QString("%1 (%2)").arg(langInfo.nativeName, langInfo.code);
            m_languageCombo->addItem(displayName, langInfo.code);
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

QGroupBox* SettingsDialog::createMediaDeviceGroup()
{
    m_mediaDeviceGroup = new QGroupBox(tr("media_device_settings"), this);
    QFormLayout* layout = new QFormLayout(m_mediaDeviceGroup);
    
    // Camera selection
    m_cameraLabel = new QLabel(tr("camera_label"));
    QHBoxLayout* cameraLayout = new QHBoxLayout();
    m_cameraCombo = new QComboBox();
    m_testCameraButton = new QPushButton(tr("test_camera"));
    cameraLayout->addWidget(m_cameraCombo);
    cameraLayout->addWidget(m_testCameraButton);
    
    layout->addRow(m_cameraLabel, cameraLayout);
    
    // Camera preview
    m_cameraPreview = new QVideoWidget();
    m_cameraPreview->setFixedSize(320, 240);
    m_cameraPreview->hide();
    layout->addRow("", m_cameraPreview);
    
    // Microphone selection
    m_microphoneLabel = new QLabel(tr("microphone_label"));
    QHBoxLayout* microphoneLayout = new QHBoxLayout();
    m_microphoneCombo = new QComboBox();
    m_testMicrophoneButton = new QPushButton(tr("test_microphone"));
    microphoneLayout->addWidget(m_microphoneCombo);
    microphoneLayout->addWidget(m_testMicrophoneButton);
    
    layout->addRow(m_microphoneLabel, microphoneLayout);
    
    // Microphone level indicator
    m_microphoneLevel = new QProgressBar();
    m_microphoneLevel->setRange(0, 100);
    m_microphoneLevel->setValue(0);
    m_microphoneLevel->setTextVisible(false);
    m_microphoneLevel->hide();
    layout->addRow("", m_microphoneLevel);
    
    // Microphone volume
    m_microphoneVolumeLabel = new QLabel(tr("microphone_volume"));
    m_microphoneVolumeSlider = new QSlider(Qt::Horizontal);
    m_microphoneVolumeSlider->setRange(0, 100);
    m_microphoneVolumeSlider->setValue(80);
    layout->addRow(m_microphoneVolumeLabel, m_microphoneVolumeSlider);
    
    // Speaker selection
    m_speakerLabel = new QLabel(tr("speaker_label"));
    QHBoxLayout* speakerLayout = new QHBoxLayout();
    m_speakerCombo = new QComboBox();
    m_testSpeakerButton = new QPushButton(tr("test_speaker"));
    speakerLayout->addWidget(m_speakerCombo);
    speakerLayout->addWidget(m_testSpeakerButton);
    
    layout->addRow(m_speakerLabel, speakerLayout);
    
    // Speaker volume
    m_speakerVolumeLabel = new QLabel(tr("speaker_volume"));
    m_speakerVolumeSlider = new QSlider(Qt::Horizontal);
    m_speakerVolumeSlider->setRange(0, 100);
    m_speakerVolumeSlider->setValue(80);
    layout->addRow(m_speakerVolumeLabel, m_speakerVolumeSlider);
    
    return m_mediaDeviceGroup;
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
    
    // Media device connections
    if (m_cameraCombo) {
        connect(m_cameraCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &SettingsDialog::onCameraChanged);
    }
    if (m_microphoneCombo) {
        connect(m_microphoneCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &SettingsDialog::onMicrophoneChanged);
    }
    if (m_speakerCombo) {
        connect(m_speakerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &SettingsDialog::onSpeakerChanged);
    }
    
    // Device test connections
    if (m_testCameraButton) {
        connect(m_testCameraButton, &QPushButton::clicked, this, &SettingsDialog::testCamera);
    }
    if (m_testMicrophoneButton) {
        connect(m_testMicrophoneButton, &QPushButton::clicked, this, &SettingsDialog::testMicrophone);
    }
    if (m_testSpeakerButton) {
        connect(m_testSpeakerButton, &QPushButton::clicked, this, &SettingsDialog::testSpeaker);
    }
    
    // Volume slider connections
    if (m_microphoneVolumeSlider) {
        connect(m_microphoneVolumeSlider, &QSlider::valueChanged,
                this, &SettingsDialog::onMicrophoneVolumeChanged);
    }
    if (m_speakerVolumeSlider) {
        connect(m_speakerVolumeSlider, &QSlider::valueChanged,
                this, &SettingsDialog::onSpeakerVolumeChanged);
    }
    
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
    
    auto settingsOpt = m_configManager->loadConfiguration();
    ApplicationSettings settings = settingsOpt.value_or(ApplicationSettings{});
    m_originalSettings = settings;
    
    // Load server settings
    m_serverUrlEdit->setText(settings.defaultServerUrl);
    m_serverTimeoutSpin->setValue(settings.serverTimeout);
    
    // Load interface settings
    QString currentLang = m_translationManager ? m_translationManager->currentLanguageCode() : "en";
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
    
    stopAllTests();
    applySettings();
    accept();
}

void SettingsDialog::cancelSettings()
{
    stopAllTests();
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
    if (m_translationManager && selectedLang != m_translationManager->currentLanguageCode()) {
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
    if (!m_translationManager || !m_languageCombo) {
        return;
    }
    
    QString selectedLanguageCode = m_languageCombo->currentData().toString();
    qDebug() << "Language changed to:" << selectedLanguageCode;
    
    // Apply language change immediately for preview
    if (m_translationManager->setLanguage(selectedLanguageCode)) {
        // Update UI text with new language
        updateUIText();
        qDebug() << "Language applied successfully";
    } else {
        qDebug() << "Failed to apply language change";
    }
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
    
    if (m_mediaDeviceGroup) {
        m_mediaDeviceGroup->setTitle(tr("media_device_settings"));
    }
    
    // Update media device labels
    if (m_cameraLabel) {
        m_cameraLabel->setText(tr("camera_label"));
    }
    if (m_microphoneLabel) {
        m_microphoneLabel->setText(tr("microphone_label"));
    }
    if (m_speakerLabel) {
        m_speakerLabel->setText(tr("speaker_label"));
    }
    if (m_microphoneVolumeLabel) {
        m_microphoneVolumeLabel->setText(tr("microphone_volume"));
    }
    if (m_speakerVolumeLabel) {
        m_speakerVolumeLabel->setText(tr("speaker_volume"));
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
    refreshDeviceList();
    loadSettings();
    show();
    raise();
    activateWindow();
}

void SettingsDialog::resetToDefaults()
{
    restoreDefaults();
}

void SettingsDialog::initializeDeviceLists()
{
    if (!m_mediaManager) {
        return;
    }
    
    refreshDeviceList();
}

void SettingsDialog::refreshDeviceList()
{
    if (!m_mediaManager) {
        return;
    }
    
    // Clear existing items
    if (m_cameraCombo) {
        m_cameraCombo->clear();
        QList<MediaManager::MediaDevice> cameras = m_mediaManager->availableVideoDevices();
        for (const auto& camera : cameras) {
            m_cameraCombo->addItem(camera.name, camera.id);
        }
        
        // Select current camera
        auto currentCamera = m_mediaManager->currentVideoDevice();
        if (currentCamera.has_value()) {
            int cameraIndex = m_cameraCombo->findData(currentCamera->id);
            if (cameraIndex >= 0) {
                m_cameraCombo->setCurrentIndex(cameraIndex);
            }
        }
    }
    
    if (m_microphoneCombo) {
        m_microphoneCombo->clear();
        QList<MediaManager::MediaDevice> microphones = m_mediaManager->availableAudioInputDevices();
        for (const auto& microphone : microphones) {
            m_microphoneCombo->addItem(microphone.name, microphone.id);
        }
        
        // Select current microphone
        auto currentMicrophone = m_mediaManager->currentAudioInputDevice();
        if (currentMicrophone.has_value()) {
            int microphoneIndex = m_microphoneCombo->findData(currentMicrophone->id);
            if (microphoneIndex >= 0) {
                m_microphoneCombo->setCurrentIndex(microphoneIndex);
            }
        }
    }
    
    if (m_speakerCombo) {
        m_speakerCombo->clear();
        QList<MediaManager::MediaDevice> speakers = m_mediaManager->availableAudioOutputDevices();
        for (const auto& speaker : speakers) {
            m_speakerCombo->addItem(speaker.name, speaker.id);
        }
        
        // Select current speaker
        auto currentSpeaker = m_mediaManager->currentAudioOutputDevice();
        if (currentSpeaker.has_value()) {
            int speakerIndex = m_speakerCombo->findData(currentSpeaker->id);
            if (speakerIndex >= 0) {
                m_speakerCombo->setCurrentIndex(speakerIndex);
            }
        }
    }
    
    // Update volume sliders
    if (m_microphoneVolumeSlider && m_mediaManager) {
        m_microphoneVolumeSlider->setValue(static_cast<int>(m_mediaManager->microphoneVolume() * 100));
    }
    if (m_speakerVolumeSlider && m_mediaManager) {
        m_speakerVolumeSlider->setValue(static_cast<int>(m_mediaManager->masterVolume() * 100));
    }
}

void SettingsDialog::onCameraChanged()
{
    if (!m_mediaManager || !m_cameraCombo) {
        return;
    }
    
    QString deviceId = m_cameraCombo->currentData().toString();
    if (!deviceId.isEmpty()) {
        m_mediaManager->setVideoDevice(deviceId);
    }
}

void SettingsDialog::onMicrophoneChanged()
{
    if (!m_mediaManager || !m_microphoneCombo) {
        return;
    }
    
    QString deviceId = m_microphoneCombo->currentData().toString();
    if (!deviceId.isEmpty()) {
        m_mediaManager->setAudioInputDevice(deviceId);
    }
}

void SettingsDialog::onSpeakerChanged()
{
    if (!m_mediaManager || !m_speakerCombo) {
        return;
    }
    
    QString deviceId = m_speakerCombo->currentData().toString();
    if (!deviceId.isEmpty()) {
        m_mediaManager->setAudioOutputDevice(deviceId);
    }
}

void SettingsDialog::testCamera()
{
    if (!m_mediaManager || !m_cameraPreview || !m_testCameraButton) {
        return;
    }
    
    if (m_cameraTestActive) {
        // Stop camera test
        m_mediaManager->stopLocalVideo();
        m_cameraPreview->hide();
        m_testCameraButton->setText(tr("test_camera"));
        m_cameraTestActive = false;
    } else {
        // Start camera test
        stopAllTests(); // Stop other tests first
        
        m_mediaManager->startLocalVideo();
        QVideoWidget* localWidget = m_mediaManager->localVideoWidget();
        if (localWidget) {
            // Copy the video output to our preview widget
            m_cameraPreview->show();
            m_testCameraButton->setText(tr("stop_test"));
            m_cameraTestActive = true;
        }
    }
}

void SettingsDialog::testMicrophone()
{
    if (!m_mediaManager || !m_microphoneLevel || !m_testMicrophoneButton) {
        return;
    }
    
    if (m_microphoneTestActive) {
        // Stop microphone test
        m_mediaManager->stopLocalAudio();
        m_microphoneLevel->hide();
        m_testMicrophoneButton->setText(tr("test_microphone"));
        m_microphoneTestActive = false;
    } else {
        // Start microphone test
        stopAllTests(); // Stop other tests first
        
        m_mediaManager->startLocalAudio();
        m_microphoneLevel->show();
        m_testMicrophoneButton->setText(tr("stop_test"));
        m_microphoneTestActive = true;
        
        // TODO: Connect to actual microphone level monitoring
        // For now, simulate some activity
        QTimer::singleShot(100, [this]() {
            if (m_microphoneTestActive && m_microphoneLevel) {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                static std::uniform_int_distribution<> dis(20, 99);
                m_microphoneLevel->setValue(dis(gen));
                QTimer::singleShot(100, this, [this]() { testMicrophone(); });
            }
        });
    }
}

void SettingsDialog::testSpeaker()
{
    if (!m_mediaManager || !m_testSpeakerButton) {
        return;
    }
    
    if (m_speakerTestActive) {
        // Stop speaker test
        m_testSpeakerButton->setText(tr("test_speaker"));
        m_speakerTestActive = false;
        // TODO: Stop test sound playback
    } else {
        // Start speaker test
        stopAllTests(); // Stop other tests first
        
        m_testSpeakerButton->setText(tr("stop_test"));
        m_speakerTestActive = true;
        
        // TODO: Play test sound through selected speaker
        QMessageBox::information(this, tr("speaker_test"), tr("speaker_test_message"));
        
        // Auto-stop after a few seconds
        QTimer::singleShot(3000, [this]() {
            if (m_speakerTestActive) {
                testSpeaker();
            }
        });
    }
}

void SettingsDialog::stopAllTests()
{
    if (m_cameraTestActive) {
        testCamera();
    }
    if (m_microphoneTestActive) {
        testMicrophone();
    }
    if (m_speakerTestActive) {
        testSpeaker();
    }
}

void SettingsDialog::onMicrophoneVolumeChanged(int volume)
{
    if (m_mediaManager) {
        m_mediaManager->setMicrophoneVolume(volume / 100.0);
    }
}

void SettingsDialog::onSpeakerVolumeChanged(int volume)
{
    if (m_mediaManager) {
        m_mediaManager->setMasterVolume(volume / 100.0);
    }
}

void SettingsDialog::updateTestStatus()
{
    // Update UI based on current test status
    if (m_testCameraButton) {
        m_testCameraButton->setText(m_cameraTestActive ? tr("stop_test") : tr("test_camera"));
    }
    if (m_testMicrophoneButton) {
        m_testMicrophoneButton->setText(m_microphoneTestActive ? tr("stop_test") : tr("test_microphone"));
    }
    if (m_testSpeakerButton) {
        m_testSpeakerButton->setText(m_speakerTestActive ? tr("stop_test") : tr("test_speaker"));
    }
}

void SettingsDialog::onServerUrlChanged()
{
    validateServerUrl();
}

void SettingsDialog::onDarkModeToggled()
{
    if (!m_darkModeCheck) {
        return;
    }
    
    bool darkMode = m_darkModeCheck->isChecked();
    qDebug() << "Dark mode toggled:" << darkMode;
    
    // TODO: Apply dark mode theme when ThemeManager is integrated
}