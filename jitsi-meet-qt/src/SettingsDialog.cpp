#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QListWidget>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QApplication>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_categoryList(nullptr)
    , m_pageStack(nullptr)
    , m_buttonBox(nullptr)
    , m_headerLabel(nullptr)
    , m_currentPage(GeneralPage)
    , m_settingsModified(false)
    , m_applyingSettings(false)
{
    setWindowTitle(tr("Settings"));
    setModal(true);
    resize(800, 600);
    
    setupUI();
    createPages();
    setupConnections();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Header
    m_headerLabel = new QLabel(tr("Settings"), this);
    m_headerLabel->setStyleSheet("font-size: 18px; font-weight: bold; padding: 10px;");
    m_mainLayout->addWidget(m_headerLabel);
    
    // Splitter for category list and pages
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_splitter);
    
    // Category list
    m_categoryList = new QListWidget(this);
    m_categoryList->setMaximumWidth(200);
    m_categoryList->addItem(tr("General"));
    m_categoryList->addItem(tr("Audio"));
    m_categoryList->addItem(tr("Video"));
    m_categoryList->addItem(tr("Network"));
    m_categoryList->addItem(tr("Security"));
    m_categoryList->addItem(tr("Appearance"));
    m_categoryList->addItem(tr("Advanced"));
    m_categoryList->addItem(tr("About"));
    m_categoryList->setCurrentRow(0);
    m_splitter->addWidget(m_categoryList);
    
    // Page stack
    m_pageStack = new QStackedWidget(this);
    m_splitter->addWidget(m_pageStack);
    
    // Button box
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | 
                                      QDialogButtonBox::Apply | QDialogButtonBox::RestoreDefaults, this);
    m_mainLayout->addWidget(m_buttonBox);
}

void SettingsDialog::createPages()
{
    createGeneralPage();
    createAudioPage();
    createVideoPage();
    createNetworkPage();
    createSecurityPage();
    createAppearancePage();
    createAdvancedPage();
    createAboutPage();
}

void SettingsDialog::createGeneralPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    QGroupBox* generalGroup = new QGroupBox(tr("General Settings"));
    QFormLayout* formLayout = new QFormLayout(generalGroup);
    
    QLineEdit* serverEdit = new QLineEdit();
    formLayout->addRow(tr("Server URL:"), serverEdit);
    
    QComboBox* languageCombo = new QComboBox();
    languageCombo->addItems({"English", "中文", "Français", "Deutsch"});
    formLayout->addRow(tr("Language:"), languageCombo);
    
    QCheckBox* autoStartCheck = new QCheckBox(tr("Start automatically"));
    formLayout->addRow(autoStartCheck);
    
    layout->addWidget(generalGroup);
    layout->addStretch();
    
    m_pageWidgets[GeneralPage] = page;
    m_pageStack->addWidget(page);
}

void SettingsDialog::createAudioPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    QGroupBox* audioGroup = new QGroupBox(tr("Audio Settings"));
    QFormLayout* formLayout = new QFormLayout(audioGroup);
    
    QComboBox* micCombo = new QComboBox();
    micCombo->addItem(tr("Default Microphone"));
    formLayout->addRow(tr("Microphone:"), micCombo);
    
    QComboBox* speakerCombo = new QComboBox();
    speakerCombo->addItem(tr("Default Speaker"));
    formLayout->addRow(tr("Speaker:"), speakerCombo);
    
    QSlider* volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    formLayout->addRow(tr("Volume:"), volumeSlider);
    
    layout->addWidget(audioGroup);
    layout->addStretch();
    
    m_pageWidgets[AudioPage] = page;
    m_pageStack->addWidget(page);
}

void SettingsDialog::createVideoPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    QGroupBox* videoGroup = new QGroupBox(tr("Video Settings"));
    QFormLayout* formLayout = new QFormLayout(videoGroup);
    
    QComboBox* cameraCombo = new QComboBox();
    cameraCombo->addItem(tr("Default Camera"));
    formLayout->addRow(tr("Camera:"), cameraCombo);
    
    QComboBox* resolutionCombo = new QComboBox();
    resolutionCombo->addItems({"720p", "1080p", "4K"});
    formLayout->addRow(tr("Resolution:"), resolutionCombo);
    
    layout->addWidget(videoGroup);
    layout->addStretch();
    
    m_pageWidgets[VideoPage] = page;
    m_pageStack->addWidget(page);
}

void SettingsDialog::createNetworkPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    QGroupBox* networkGroup = new QGroupBox(tr("Network Settings"));
    QFormLayout* formLayout = new QFormLayout(networkGroup);
    
    QLineEdit* proxyEdit = new QLineEdit();
    formLayout->addRow(tr("Proxy Server:"), proxyEdit);
    
    QSpinBox* portSpin = new QSpinBox();
    portSpin->setRange(1, 65535);
    portSpin->setValue(8080);
    formLayout->addRow(tr("Port:"), portSpin);
    
    layout->addWidget(networkGroup);
    layout->addStretch();
    
    m_pageWidgets[NetworkPage] = page;
    m_pageStack->addWidget(page);
}

void SettingsDialog::createSecurityPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    QGroupBox* securityGroup = new QGroupBox(tr("Security Settings"));
    QFormLayout* formLayout = new QFormLayout(securityGroup);
    
    QCheckBox* encryptionCheck = new QCheckBox(tr("Enable encryption"));
    formLayout->addRow(encryptionCheck);
    
    QCheckBox* authCheck = new QCheckBox(tr("Require authentication"));
    formLayout->addRow(authCheck);
    
    layout->addWidget(securityGroup);
    layout->addStretch();
    
    m_pageWidgets[SecurityPage] = page;
    m_pageStack->addWidget(page);
}

void SettingsDialog::createAppearancePage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    QGroupBox* appearanceGroup = new QGroupBox(tr("Appearance Settings"));
    QFormLayout* formLayout = new QFormLayout(appearanceGroup);
    
    QComboBox* themeCombo = new QComboBox();
    themeCombo->addItems({tr("Light"), tr("Dark"), tr("Auto")});
    formLayout->addRow(tr("Theme:"), themeCombo);
    
    QCheckBox* animationsCheck = new QCheckBox(tr("Enable animations"));
    formLayout->addRow(animationsCheck);
    
    layout->addWidget(appearanceGroup);
    layout->addStretch();
    
    m_pageWidgets[AppearancePage] = page;
    m_pageStack->addWidget(page);
}

void SettingsDialog::createAdvancedPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    QGroupBox* advancedGroup = new QGroupBox(tr("Advanced Settings"));
    QFormLayout* formLayout = new QFormLayout(advancedGroup);
    
    QCheckBox* debugCheck = new QCheckBox(tr("Enable debug logging"));
    formLayout->addRow(debugCheck);
    
    QCheckBox* telemetryCheck = new QCheckBox(tr("Send telemetry data"));
    formLayout->addRow(telemetryCheck);
    
    layout->addWidget(advancedGroup);
    layout->addStretch();
    
    m_pageWidgets[AdvancedPage] = page;
    m_pageStack->addWidget(page);
}

void SettingsDialog::createAboutPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    QLabel* aboutLabel = new QLabel(tr("Jitsi Meet Qt\nVersion 1.0.0\n\nA Qt-based Jitsi Meet client"));
    aboutLabel->setAlignment(Qt::AlignCenter);
    aboutLabel->setStyleSheet("padding: 20px; font-size: 14px;");
    
    layout->addWidget(aboutLabel);
    layout->addStretch();
    
    m_pageWidgets[AboutPage] = page;
    m_pageStack->addWidget(page);
}

void SettingsDialog::setupConnections()
{
    connect(m_categoryList, &QListWidget::currentRowChanged, this, &SettingsDialog::onPageChanged);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    
    QPushButton* applyButton = m_buttonBox->button(QDialogButtonBox::Apply);
    if (applyButton) {
        connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
    }
    
    QPushButton* resetButton = m_buttonBox->button(QDialogButtonBox::RestoreDefaults);
    if (resetButton) {
        connect(resetButton, &QPushButton::clicked, this, &SettingsDialog::onResetClicked);
    }
}

void SettingsDialog::showPage(SettingsPage page)
{
    m_categoryList->setCurrentRow(static_cast<int>(page));
    m_pageStack->setCurrentIndex(static_cast<int>(page));
    m_currentPage = page;
    emit pageChanged(page);
}

SettingsDialog::SettingsPage SettingsDialog::currentPage() const
{
    return m_currentPage;
}

void SettingsDialog::setSetting(const QString& key, const QVariant& value)
{
    m_settings[key] = value;
    m_settingsModified = true;
}

QVariant SettingsDialog::getSetting(const QString& key, const QVariant& defaultValue) const
{
    return m_settings.value(key, defaultValue);
}

void SettingsDialog::resetAllSettings()
{
    m_settings.clear();
    m_settingsModified = true;
    emit settingsReset();
}

void SettingsDialog::resetPageSettings(SettingsPage page)
{
    Q_UNUSED(page)
    // Implementation would clear settings for specific page
    m_settingsModified = true;
}

bool SettingsDialog::applySettings()
{
    if (!validateSettings()) {
        return false;
    }
    
    collectSettings();
    emit settingsChanged(m_settings);
    emit settingsApplied();
    m_settingsModified = false;
    return true;
}

bool SettingsDialog::loadSettings()
{
    // Load settings from configuration
    // This is a simplified implementation
    m_originalSettings = m_settings;
    return true;
}

bool SettingsDialog::saveSettings()
{
    // Save settings to configuration
    // This is a simplified implementation
    return applySettings();
}

bool SettingsDialog::validateSettings()
{
    m_errors.clear();
    // Add validation logic here
    return m_errors.isEmpty();
}

QStringList SettingsDialog::validationErrors() const
{
    return m_errors;
}

void SettingsDialog::accept()
{
    if (applySettings()) {
        QDialog::accept();
    }
}

void SettingsDialog::reject()
{
    m_settings = m_originalSettings;
    QDialog::reject();
}

void SettingsDialog::onPageChanged(int index)
{
    if (index >= 0 && index < static_cast<int>(AboutPage) + 1) {
        m_currentPage = static_cast<SettingsPage>(index);
        m_pageStack->setCurrentIndex(index);
        emit pageChanged(m_currentPage);
    }
}

void SettingsDialog::onApplyClicked()
{
    applySettings();
}

void SettingsDialog::onResetClicked()
{
    resetAllSettings();
}

void SettingsDialog::onCategorySelectionChanged()
{
    // Handle category selection changes
}

void SettingsDialog::onSettingValueChanged()
{
    m_settingsModified = true;
    updateButtonStates();
}

void SettingsDialog::onRestoreDefaultsClicked()
{
    resetAllSettings();
}

void SettingsDialog::updateButtonStates()
{
    QPushButton* applyButton = m_buttonBox->button(QDialogButtonBox::Apply);
    if (applyButton) {
        applyButton->setEnabled(m_settingsModified);
    }
}

void SettingsDialog::savePageState(SettingsPage page)
{
    Q_UNUSED(page)
    // Save current page state
}

void SettingsDialog::restorePageState(SettingsPage page)
{
    Q_UNUSED(page)
    // Restore page state
}

QString SettingsDialog::pageToString(SettingsPage page) const
{
    switch (page) {
    case GeneralPage: return "General";
    case AudioPage: return "Audio";
    case VideoPage: return "Video";
    case NetworkPage: return "Network";
    case SecurityPage: return "Security";
    case AppearancePage: return "Appearance";
    case AdvancedPage: return "Advanced";
    case AboutPage: return "About";
    }
    return "General";
}

SettingsDialog::SettingsPage SettingsDialog::stringToPage(const QString& pageName) const
{
    if (pageName == "Audio") return AudioPage;
    if (pageName == "Video") return VideoPage;
    if (pageName == "Network") return NetworkPage;
    if (pageName == "Security") return SecurityPage;
    if (pageName == "Appearance") return AppearancePage;
    if (pageName == "Advanced") return AdvancedPage;
    if (pageName == "About") return AboutPage;
    return GeneralPage;
}

QWidget* SettingsDialog::createPageWidget(SettingsPage page)
{
    return m_pageWidgets.value(page, nullptr);
}

void SettingsDialog::collectSettings()
{
    // Collect settings from all pages
    // This is a simplified implementation
}

void SettingsDialog::applySettingsToPage(SettingsPage page)
{
    Q_UNUSED(page)
    // Apply settings to specific page
}

void SettingsDialog::validatePage(SettingsPage page)
{
    Q_UNUSED(page)
    // Validate specific page
}