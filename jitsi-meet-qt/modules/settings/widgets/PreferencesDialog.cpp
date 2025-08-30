#include "PreferencesDialog.h"
#include "SettingsWidget.h"
#include "../include/PreferencesHandler.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QSlider>
#include <QProgressBar>
#include <QSplitter>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QStyle>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

class PreferencesDialog::Private
{
public:
    Private()
        : preferencesHandler(nullptr)
        , currentDialogMode(StandardMode)
        , previewModeEnabled(false)
        , progressDialog(nullptr)
    {
    }

    // Core components
    PreferencesHandler* preferencesHandler;
    
    // UI components
    QVBoxLayout* mainLayout;
    QHBoxLayout* topLayout;
    QComboBox* profileSelector;
    QLineEdit* searchBox;
    QCheckBox* previewModeCheckBox;
    
    QSplitter* splitter;
    QListWidget* pageList;
    QStackedWidget* pageStack;
    QDialogButtonBox* buttonBox;
    
    // Settings
    QString currentProfile;
    DialogMode currentDialogMode;
    bool previewModeEnabled;
    QString currentTheme;
    QString currentLanguage;
    QString helpUrl;
    
    // Pages
    QList<PageInfo> pages;
    QString currentPageId;
    
    // Widgets
    SettingsWidget* settingsWidget;
    QProgressBar* progressDialog;
    
    // Data
    QStringList availableProfiles;
    QString defaultProfile;
    QVariantMap currentSettings;
    QVariantMap originalSettings;
};

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
    , d(std::make_unique<Private>())
{
    setWindowTitle(tr("Preferences"));
    setModal(true);
    resize(800, 600);
    
    setupUI();
    createDefaultPages();
    connectSignals();
    
    // Load default profile
    d->currentProfile = "default";
    d->defaultProfile = "default";
}

PreferencesDialog::~PreferencesDialog() = default;Q
String PreferencesDialog::currentProfile() const
{
    return d->currentProfile;
}

void PreferencesDialog::setCurrentProfile(const QString& profile)
{
    if (d->currentProfile != profile) {
        QString oldProfile = d->currentProfile;
        d->currentProfile = profile;
        
        // Update profile selector
        if (d->profileSelector) {
            d->profileSelector->setCurrentText(profile);
        }
        
        emit currentProfileChanged(profile);
        emit profileChanged(oldProfile, profile);
        
        // Load profile settings
        loadProfileSettings();
    }
}

bool PreferencesDialog::isPreviewMode() const
{
    return d->previewModeEnabled;
}

void PreferencesDialog::setPreviewMode(bool enabled)
{
    if (d->previewModeEnabled != enabled) {
        d->previewModeEnabled = enabled;
        
        if (d->previewModeCheckBox) {
            d->previewModeCheckBox->setChecked(enabled);
        }
        
        emit previewModeChanged(enabled);
        
        // Apply preview changes immediately if enabled
        if (enabled && d->settingsWidget) {
            // Apply current settings as preview
            applyPreviewSettings();
        }
    }
}

PreferencesDialog::DialogMode PreferencesDialog::dialogMode() const
{
    return d->currentDialogMode;
}

void PreferencesDialog::setDialogMode(DialogMode mode)
{
    if (d->currentDialogMode != mode) {
        d->currentDialogMode = mode;
        emit dialogModeChanged(mode);
        setupUI(); // Rebuild UI for new mode
    }
}

void PreferencesDialog::setPreferencesHandler(PreferencesHandler* handler)
{
    d->preferencesHandler = handler;
    
    if (d->settingsWidget) {
        d->settingsWidget->setPreferencesHandler(handler);
    }
}

PreferencesHandler* PreferencesDialog::preferencesHandler() const
{
    return d->preferencesHandler;
}

void PreferencesDialog::setupUI()
{
    // Clear existing layout
    if (layout()) {
        QLayoutItem* item;
        while ((item = layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete layout();
    }
    
    d->mainLayout = new QVBoxLayout(this);
    d->mainLayout->setContentsMargins(6, 6, 6, 6);
    d->mainLayout->setSpacing(6);
    
    // Setup based on dialog mode
    switch (d->currentDialogMode) {
        case StandardMode:
            setupStandardMode();
            break;
        case CompactMode:
            setupCompactMode();
            break;
        case WizardMode:
            setupWizardMode();
            break;
        case ExpertMode:
            setupExpertMode();
            break;
    }
    
    setupButtonBox();
}

void PreferencesDialog::setupStandardMode()
{
    // Top layout with profile selector and search
    d->topLayout = new QHBoxLayout();
    
    // Profile selector
    QLabel* profileLabel = new QLabel(tr("Profile:"));
    d->profileSelector = new QComboBox();
    d->profileSelector->setMinimumWidth(150);
    d->profileSelector->addItems(d->availableProfiles);
    d->profileSelector->setCurrentText(d->currentProfile);
    
    // Search box
    QLabel* searchLabel = new QLabel(tr("Search:"));
    d->searchBox = new QLineEdit();
    d->searchBox->setPlaceholderText(tr("Search settings..."));
    d->searchBox->setClearButtonEnabled(true);
    
    // Preview mode checkbox
    d->previewModeCheckBox = new QCheckBox(tr("Preview changes"));
    d->previewModeCheckBox->setChecked(d->previewModeEnabled);
    
    d->topLayout->addWidget(profileLabel);
    d->topLayout->addWidget(d->profileSelector);
    d->topLayout->addStretch();
    d->topLayout->addWidget(searchLabel);
    d->topLayout->addWidget(d->searchBox);
    d->topLayout->addWidget(d->previewModeCheckBox);
    
    d->mainLayout->addLayout(d->topLayout);
    
    // Main content area
    d->splitter = new QSplitter(Qt::Horizontal);
    
    // Page list
    d->pageList = new QListWidget();
    d->pageList->setMaximumWidth(200);
    d->pageList->setAlternatingRowColors(true);
    
    // Page stack
    d->pageStack = new QStackedWidget();
    
    d->splitter->addWidget(d->pageList);
    d->splitter->addWidget(d->pageStack);
    d->splitter->setStretchFactor(0, 0);
    d->splitter->setStretchFactor(1, 1);
    
    d->mainLayout->addWidget(d->splitter);
    
    // Populate pages
    updatePageList();
}

void PreferencesDialog::setupCompactMode()
{
    // Simplified layout for compact mode
    d->settingsWidget = new SettingsWidget();
    d->settingsWidget->setViewMode(SettingsWidget::ListView);
    d->settingsWidget->setPreferencesHandler(d->preferencesHandler);
    
    d->mainLayout->addWidget(d->settingsWidget);
}

void PreferencesDialog::setupWizardMode()
{
    // Wizard-style layout
    d->settingsWidget = new SettingsWidget();
    d->settingsWidget->setViewMode(SettingsWidget::WizardView);
    d->settingsWidget->setPreferencesHandler(d->preferencesHandler);
    
    d->mainLayout->addWidget(d->settingsWidget);
}

void PreferencesDialog::setupExpertMode()
{
    // Expert mode with advanced options
    setupStandardMode(); // Use standard as base
    
    // Add expert-specific controls
    if (d->settingsWidget) {
        d->settingsWidget->setShowAdvancedEnabled(true);
    }
}

void PreferencesDialog::createDefaultPages()
{
    // Clear existing pages
    d->pages.clear();
    
    createGeneralPage();
    createAudioPage();
    createVideoPage();
    createNetworkPage();
    createUIPage();
    createAdvancedPage();
}

void PreferencesDialog::createGeneralPage()
{
    PageInfo pageInfo;
    pageInfo.id = "general";
    pageInfo.title = tr("General");
    pageInfo.description = tr("General application settings");
    pageInfo.icon = ":/icons/general.png";
    pageInfo.order = 0;
    
    // Create settings widget for general settings
    SettingsWidget* generalWidget = new SettingsWidget();
    generalWidget->setPreferencesHandler(d->preferencesHandler);
    
    // Add some general settings
    SettingsWidget::SettingDescriptor languageDesc;
    languageDesc.key = "general.language";
    languageDesc.displayName = tr("Language");
    languageDesc.description = tr("Application language");
    languageDesc.type = SettingsWidget::EnumSetting;
    languageDesc.category = "general";
    languageDesc.defaultValue = "en";
    languageDesc.enumValues = QStringList() << "en" << "zh" << "es" << "fr" << "de";
    generalWidget->addSetting(languageDesc);
    
    SettingsWidget::SettingDescriptor startupDesc;
    startupDesc.key = "general.startWithSystem";
    startupDesc.displayName = tr("Start with system");
    startupDesc.description = tr("Start application when system boots");
    startupDesc.type = SettingsWidget::BooleanSetting;
    startupDesc.category = "general";
    startupDesc.defaultValue = false;
    generalWidget->addSetting(startupDesc);
    
    pageInfo.widget = generalWidget;
    addPage(pageInfo);
}

void PreferencesDialog::createAudioPage()
{
    PageInfo pageInfo;
    pageInfo.id = "audio";
    pageInfo.title = tr("Audio");
    pageInfo.description = tr("Audio device and quality settings");
    pageInfo.icon = ":/icons/audio.png";
    pageInfo.order = 1;
    
    SettingsWidget* audioWidget = new SettingsWidget();
    audioWidget->setPreferencesHandler(d->preferencesHandler);
    
    // Audio device settings
    SettingsWidget::SettingDescriptor inputDeviceDesc;
    inputDeviceDesc.key = "audio.inputDevice";
    inputDeviceDesc.displayName = tr("Input Device");
    inputDeviceDesc.description = tr("Microphone or audio input device");
    inputDeviceDesc.type = SettingsWidget::EnumSetting;
    inputDeviceDesc.category = "audio";
    inputDeviceDesc.defaultValue = "default";
    inputDeviceDesc.enumValues = QStringList() << "default" << "device1" << "device2";
    audioWidget->addSetting(inputDeviceDesc);
    
    SettingsWidget::SettingDescriptor outputDeviceDesc;
    outputDeviceDesc.key = "audio.outputDevice";
    outputDeviceDesc.displayName = tr("Output Device");
    outputDeviceDesc.description = tr("Speaker or audio output device");
    outputDeviceDesc.type = SettingsWidget::EnumSetting;
    outputDeviceDesc.category = "audio";
    outputDeviceDesc.defaultValue = "default";
    outputDeviceDesc.enumValues = QStringList() << "default" << "speakers" << "headphones";
    audioWidget->addSetting(outputDeviceDesc);
    
    pageInfo.widget = audioWidget;
    addPage(pageInfo);
}

void PreferencesDialog::createVideoPage()
{
    PageInfo pageInfo;
    pageInfo.id = "video";
    pageInfo.title = tr("Video");
    pageInfo.description = tr("Camera and video quality settings");
    pageInfo.icon = ":/icons/video.png";
    pageInfo.order = 2;
    
    SettingsWidget* videoWidget = new SettingsWidget();
    videoWidget->setPreferencesHandler(d->preferencesHandler);
    
    // Video quality settings
    SettingsWidget::SettingDescriptor qualityDesc;
    qualityDesc.key = "video.quality";
    qualityDesc.displayName = tr("Video Quality");
    qualityDesc.description = tr("Video resolution and quality");
    qualityDesc.type = SettingsWidget::EnumSetting;
    qualityDesc.category = "video";
    qualityDesc.defaultValue = "720p";
    qualityDesc.enumValues = QStringList() << "480p" << "720p" << "1080p";
    videoWidget->addSetting(qualityDesc);
    
    pageInfo.widget = videoWidget;
    addPage(pageInfo);
}

void PreferencesDialog::createNetworkPage()
{
    PageInfo pageInfo;
    pageInfo.id = "network";
    pageInfo.title = tr("Network");
    pageInfo.description = tr("Network and connection settings");
    pageInfo.icon = ":/icons/network.png";
    pageInfo.order = 3;
    
    SettingsWidget* networkWidget = new SettingsWidget();
    networkWidget->setPreferencesHandler(d->preferencesHandler);
    
    pageInfo.widget = networkWidget;
    addPage(pageInfo);
}

void PreferencesDialog::createUIPage()
{
    PageInfo pageInfo;
    pageInfo.id = "ui";
    pageInfo.title = tr("Interface");
    pageInfo.description = tr("User interface and theme settings");
    pageInfo.icon = ":/icons/ui.png";
    pageInfo.order = 4;
    
    SettingsWidget* uiWidget = new SettingsWidget();
    uiWidget->setPreferencesHandler(d->preferencesHandler);
    
    pageInfo.widget = uiWidget;
    addPage(pageInfo);
}

void PreferencesDialog::createAdvancedPage()
{
    PageInfo pageInfo;
    pageInfo.id = "advanced";
    pageInfo.title = tr("Advanced");
    pageInfo.description = tr("Advanced settings and debugging");
    pageInfo.icon = ":/icons/advanced.png";
    pageInfo.order = 5;
    
    SettingsWidget* advancedWidget = new SettingsWidget();
    advancedWidget->setShowAdvancedEnabled(true);
    advancedWidget->setPreferencesHandler(d->preferencesHandler);
    
    pageInfo.widget = advancedWidget;
    addPage(pageInfo);
}void Prefe
rencesDialog::addPage(const PageInfo& pageInfo)
{
    d->pages.append(pageInfo);
    
    // Sort pages by order
    std::sort(d->pages.begin(), d->pages.end(), 
              [](const PageInfo& a, const PageInfo& b) { return a.order < b.order; });
    
    updatePageList();
}

void PreferencesDialog::updatePageList()
{
    if (!d->pageList || !d->pageStack) return;
    
    d->pageList->clear();
    
    for (const auto& pageInfo : d->pages) {
        if (!pageInfo.isVisible) continue;
        
        QListWidgetItem* item = new QListWidgetItem(pageInfo.title);
        item->setData(Qt::UserRole, pageInfo.id);
        item->setToolTip(pageInfo.description);
        
        if (!pageInfo.icon.isEmpty()) {
            item->setIcon(QIcon(pageInfo.icon));
        }
        
        d->pageList->addItem(item);
        
        if (pageInfo.widget && d->pageStack->indexOf(pageInfo.widget) == -1) {
            d->pageStack->addWidget(pageInfo.widget);
        }
    }
    
    // Select first page if none selected
    if (d->pageList->count() > 0 && d->currentPageId.isEmpty()) {
        d->pageList->setCurrentRow(0);
        onPageSelectionChanged();
    }
}

void PreferencesDialog::setupButtonBox()
{
    d->buttonBox = new QDialogButtonBox(this);
    
    // Standard buttons
    QPushButton* okButton = d->buttonBox->addButton(QDialogButtonBox::Ok);
    QPushButton* cancelButton = d->buttonBox->addButton(QDialogButtonBox::Cancel);
    QPushButton* applyButton = d->buttonBox->addButton(QDialogButtonBox::Apply);
    
    // Custom buttons
    QPushButton* resetButton = d->buttonBox->addButton(tr("Reset"), QDialogButtonBox::ResetRole);
    QPushButton* importButton = d->buttonBox->addButton(tr("Import..."), QDialogButtonBox::ActionRole);
    QPushButton* exportButton = d->buttonBox->addButton(tr("Export..."), QDialogButtonBox::ActionRole);
    QPushButton* helpButton = d->buttonBox->addButton(QDialogButtonBox::Help);
    
    // Set button properties
    okButton->setDefault(true);
    applyButton->setEnabled(false); // Enable when changes are made
    
    d->mainLayout->addWidget(d->buttonBox);
}

void PreferencesDialog::connectSignals()
{
    // Profile selector
    if (d->profileSelector) {
        connect(d->profileSelector, &QComboBox::currentTextChanged, 
                this, &PreferencesDialog::onProfileChanged);
    }
    
    // Search box
    if (d->searchBox) {
        connect(d->searchBox, &QLineEdit::textChanged, 
                this, &PreferencesDialog::onSearchTextChanged);
    }
    
    // Preview mode checkbox
    if (d->previewModeCheckBox) {
        connect(d->previewModeCheckBox, &QCheckBox::toggled, 
                this, &PreferencesDialog::onPreviewModeToggled);
    }
    
    // Page list
    if (d->pageList) {
        connect(d->pageList, &QListWidget::currentRowChanged, 
                this, &PreferencesDialog::onPageSelectionChanged);
    }
    
    // Button box
    if (d->buttonBox) {
        connect(d->buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::onOkButtonClicked);
        connect(d->buttonBox, &QDialogButtonBox::rejected, this, &PreferencesDialog::onCancelButtonClicked);
        
        if (QPushButton* applyButton = d->buttonBox->button(QDialogButtonBox::Apply)) {
            connect(applyButton, &QPushButton::clicked, this, &PreferencesDialog::onApplyButtonClicked);
        }
        
        if (QPushButton* resetButton = d->buttonBox->button(QDialogButtonBox::Reset)) {
            connect(resetButton, &QPushButton::clicked, this, &PreferencesDialog::onResetButtonClicked);
        }
        
        if (QPushButton* helpButton = d->buttonBox->button(QDialogButtonBox::Help)) {
            connect(helpButton, &QPushButton::clicked, this, &PreferencesDialog::onHelpButtonClicked);
        }
        
        // Find custom buttons
        const QList<QAbstractButton*> buttons = d->buttonBox->buttons();
        for (QAbstractButton* button : buttons) {
            if (button->text() == tr("Import...")) {
                connect(button, &QAbstractButton::clicked, this, &PreferencesDialog::onImportButtonClicked);
            } else if (button->text() == tr("Export...")) {
                connect(button, &QAbstractButton::clicked, this, &PreferencesDialog::onExportButtonClicked);
            }
        }
    }
}

// Profile management
bool PreferencesDialog::createProfile(const QString& profileName, const QString& copyFrom)
{
    if (d->availableProfiles.contains(profileName)) {
        return false; // Profile already exists
    }
    
    d->availableProfiles.append(profileName);
    
    if (d->profileSelector) {
        d->profileSelector->addItem(profileName);
    }
    
    emit profileCreated(profileName);
    return true;
}

bool PreferencesDialog::deleteProfile(const QString& profileName)
{
    if (!d->availableProfiles.contains(profileName) || profileName == d->defaultProfile) {
        return false; // Cannot delete default profile
    }
    
    d->availableProfiles.removeAll(profileName);
    
    if (d->profileSelector) {
        int index = d->profileSelector->findText(profileName);
        if (index >= 0) {
            d->profileSelector->removeItem(index);
        }
    }
    
    // Switch to default profile if current profile is deleted
    if (d->currentProfile == profileName) {
        setCurrentProfile(d->defaultProfile);
    }
    
    emit profileDeleted(profileName);
    return true;
}

QStringList PreferencesDialog::availableProfiles() const
{
    return d->availableProfiles;
}

void PreferencesDialog::setDefaultProfile(const QString& profileName)
{
    d->defaultProfile = profileName;
}

QString PreferencesDialog::defaultProfile() const
{
    return d->defaultProfile;
}

// Validation and application
bool PreferencesDialog::validateSettings()
{
    QStringList errors;
    
    // Validate all page widgets
    for (const auto& pageInfo : d->pages) {
        if (SettingsWidget* settingsWidget = qobject_cast<SettingsWidget*>(pageInfo.widget)) {
            if (!settingsWidget->validateSettings()) {
                errors.append(settingsWidget->validationErrors());
            }
        }
    }
    
    bool isValid = errors.isEmpty();
    emit validationCompleted(isValid, errors);
    
    if (!isValid) {
        showValidationErrors(errors);
    }
    
    return isValid;
}

QStringList PreferencesDialog::validationErrors() const
{
    QStringList errors;
    
    for (const auto& pageInfo : d->pages) {
        if (SettingsWidget* settingsWidget = qobject_cast<SettingsWidget*>(pageInfo.widget)) {
            errors.append(settingsWidget->validationErrors());
        }
    }
    
    return errors;
}

bool PreferencesDialog::applySettings()
{
    if (!validateSettings()) {
        return false;
    }
    
    bool success = true;
    
    // Apply settings from all pages
    for (const auto& pageInfo : d->pages) {
        if (SettingsWidget* settingsWidget = qobject_cast<SettingsWidget*>(pageInfo.widget)) {
            success &= settingsWidget->saveSettings();
        }
    }
    
    if (success) {
        // Save current settings as original
        d->originalSettings = d->currentSettings;
    }
    
    emit settingsApplied(success);
    return success;
}

bool PreferencesDialog::hasUnsavedChanges() const
{
    for (const auto& pageInfo : d->pages) {
        if (SettingsWidget* settingsWidget = qobject_cast<SettingsWidget*>(pageInfo.widget)) {
            if (settingsWidget->hasUnsavedChanges()) {
                return true;
            }
        }
    }
    
    return false;
}

// Slot implementations
void PreferencesDialog::onPageSelectionChanged()
{
    if (!d->pageList || !d->pageStack) return;
    
    int currentRow = d->pageList->currentRow();
    if (currentRow >= 0 && currentRow < d->pages.size()) {
        const PageInfo& pageInfo = d->pages[currentRow];
        d->currentPageId = pageInfo.id;
        
        if (pageInfo.widget) {
            d->pageStack->setCurrentWidget(pageInfo.widget);
        }
        
        emit currentPageChanged(pageInfo.id);
    }
}

void PreferencesDialog::onProfileChanged(const QString& profile)
{
    setCurrentProfile(profile);
}

void PreferencesDialog::onSearchTextChanged(const QString& text)
{
    // Implement search functionality
    for (const auto& pageInfo : d->pages) {
        if (SettingsWidget* settingsWidget = qobject_cast<SettingsWidget*>(pageInfo.widget)) {
            settingsWidget->setSearchText(text);
        }
    }
    
    emit searchTextChanged(text);
}

void PreferencesDialog::onPreviewModeToggled(bool enabled)
{
    setPreviewMode(enabled);
}

void PreferencesDialog::onOkButtonClicked()
{
    if (applySettings()) {
        accept();
    }
}

void PreferencesDialog::onCancelButtonClicked()
{
    if (hasUnsavedChanges() && !confirmUnsavedChanges()) {
        return;
    }
    
    reject();
}

void PreferencesDialog::onApplyButtonClicked()
{
    applySettings();
}

void PreferencesDialog::onResetButtonClicked()
{
    int ret = QMessageBox::question(this, tr("Reset Settings"), 
                                   tr("Are you sure you want to reset all settings to their default values?"),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        resetToDefaults();
    }
}

void PreferencesDialog::onHelpButtonClicked()
{
    showHelp();
}

void PreferencesDialog::onImportButtonClicked()
{
    showImportDialog();
}

void PreferencesDialog::onExportButtonClicked()
{
    showExportDialog();
}

// Helper methods
void PreferencesDialog::loadProfileSettings()
{
    if (d->preferencesHandler) {
        // Load settings for current profile
        // This would typically load from the preferences handler
    }
}

void PreferencesDialog::applyPreviewSettings()
{
    if (d->previewModeEnabled) {
        // Apply settings immediately for preview
        for (const auto& pageInfo : d->pages) {
            if (SettingsWidget* settingsWidget = qobject_cast<SettingsWidget*>(pageInfo.widget)) {
                // Apply settings without saving
            }
        }
    }
}

bool PreferencesDialog::confirmUnsavedChanges()
{
    int ret = QMessageBox::question(this, tr("Unsaved Changes"), 
                                   tr("You have unsaved changes. Do you want to discard them?"),
                                   QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);
    
    return ret == QMessageBox::Discard;
}

void PreferencesDialog::showValidationErrors(const QStringList& errors)
{
    QString errorText = tr("The following validation errors occurred:\n\n");
    errorText += errors.join("\n");
    
    QMessageBox::warning(this, tr("Validation Errors"), errorText);
}

void PreferencesDialog::resetToDefaults()
{
    for (const auto& pageInfo : d->pages) {
        if (SettingsWidget* settingsWidget = qobject_cast<SettingsWidget*>(pageInfo.widget)) {
            settingsWidget->resetAll();
        }
    }
    
    emit settingsReset(QString());
}

void PreferencesDialog::showImportDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Settings"), 
                                                   QString(), tr("JSON Files (*.json)"));
    
    if (!fileName.isEmpty()) {
        bool success = importSettings(fileName);
        emit importCompleted(success, fileName);
    }
}

void PreferencesDialog::showExportDialog()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Settings"), 
                                                   QString(), tr("JSON Files (*.json)"));
    
    if (!fileName.isEmpty()) {
        bool success = exportCurrentSettings(fileName);
        emit exportCompleted(success, fileName);
    }
}

bool PreferencesDialog::importSettings(const QString& filePath, bool merge)
{
    // Implementation would read settings from file and apply them
    Q_UNUSED(filePath)
    Q_UNUSED(merge)
    return true; // Placeholder
}

bool PreferencesDialog::exportCurrentSettings(const QString& filePath, const QString& format)
{
    // Implementation would save current settings to file
    Q_UNUSED(filePath)
    Q_UNUSED(format)
    return true; // Placeholder
}

void PreferencesDialog::showHelp(const QString& topic)
{
    Q_UNUSED(topic)
    // Implementation would show help for the given topic
    emit helpRequested(topic);
}

// Static convenience methods
int PreferencesDialog::showPreferences(QWidget* parent, PreferencesHandler* handler)
{
    PreferencesDialog dialog(parent);
    dialog.setPreferencesHandler(handler);
    return dialog.exec();
}