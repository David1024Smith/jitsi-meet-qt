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
#include <QKeyEvent>
#include <QCloseEvent>
#include <QShowEvent>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QMessageBox>

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

PreferencesDialog::~PreferencesDialog() = default;

QString PreferencesDialog::currentProfile() const
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
        // loadProfileSettings(); // Method will be implemented
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
            // applyPreviewSettings(); // Method will be implemented
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
}

void PreferencesDialog::addPage(const PageInfo& pageInfo)
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
    
    // emit searchTextChanged(text); // Signal will be added to header
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

void PreferencesDialog::onProfileManagerButtonClicked()
{
    showProfileManager();
}

void PreferencesDialog::onSettingChanged(const QString& key, const QVariant& value)
{
    // 处理设置变化
    d->currentSettings[key] = value;
    
    // 如果启用预览模式，立即应用设置
    if (d->previewModeEnabled) {
        applyPreviewSettings();
    }
    
    // 更新按钮状态
    updateButtonStates();
    
    emit settingChanged(key, value);
}

// Public slots implementation
void PreferencesDialog::refresh()
{
    // 重新加载当前配置文件设置
    loadProfileSettings();
    
    // 更新所有页面
    for (const auto& pageInfo : d->pages) {
        if (SettingsWidget* settingsWidget = qobject_cast<SettingsWidget*>(pageInfo.widget)) {
            settingsWidget->refresh();
        }
    }
    
    // 更新界面
    updateUI();
    updatePageList();
    updateButtonStates();
    
    emit refreshCompleted();
}

void PreferencesDialog::showAbout()
{
    QMessageBox::about(this, tr("About Preferences"), 
                      tr("Jitsi Meet Qt Preferences Dialog\n\n"
                         "Version: 1.0\n"
                         "Built with Qt %1\n\n"
                         "This dialog provides comprehensive settings management "
                         "for the Jitsi Meet Qt application.").arg(QT_VERSION_STR));
}

void PreferencesDialog::togglePreviewMode()
{
    setPreviewMode(!d->previewModeEnabled);
}

void PreferencesDialog::showProfileManager()
{
    // 创建配置文件管理对话框
    QDialog profileDialog(this);
    profileDialog.setWindowTitle(tr("Profile Manager"));
    profileDialog.setModal(true);
    profileDialog.resize(400, 300);
    
    QVBoxLayout* layout = new QVBoxLayout(&profileDialog);
    
    // 配置文件列表
    QListWidget* profileList = new QListWidget();
    profileList->addItems(d->availableProfiles);
    profileList->setCurrentRow(d->availableProfiles.indexOf(d->currentProfile));
    layout->addWidget(profileList);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* newButton = new QPushButton(tr("New"));
    QPushButton* copyButton = new QPushButton(tr("Copy"));
    QPushButton* deleteButton = new QPushButton(tr("Delete"));
    QPushButton* renameButton = new QPushButton(tr("Rename"));
    
    buttonLayout->addWidget(newButton);
    buttonLayout->addWidget(copyButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(renameButton);
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
    
    // 对话框按钮
    QDialogButtonBox* dialogButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(dialogButtons);
    
    connect(dialogButtons, &QDialogButtonBox::accepted, &profileDialog, &QDialog::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected, &profileDialog, &QDialog::reject);
    
    // 按钮事件处理
    connect(newButton, &QPushButton::clicked, [this, profileList]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("New Profile"), 
                                           tr("Profile name:"), QLineEdit::Normal, 
                                           QString(), &ok);
        if (ok && !name.isEmpty()) {
            if (createProfile(name)) {
                profileList->addItem(name);
                emit profileCreated(name);
            }
        }
    });
    
    connect(deleteButton, &QPushButton::clicked, [this, profileList]() {
        QListWidgetItem* item = profileList->currentItem();
        if (item && item->text() != d->defaultProfile) {
            int ret = QMessageBox::question(this, tr("Delete Profile"), 
                                          tr("Are you sure you want to delete profile '%1'?").arg(item->text()));
            if (ret == QMessageBox::Yes) {
                if (deleteProfile(item->text())) {
                    delete item;
                    emit profileDeleted(item->text());
                }
            }
        }
    });
    
    profileDialog.exec();
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

// Event handlers
void PreferencesDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    
    // 恢复对话框状态
    restoreDialogState();
    
    // 加载当前配置文件设置
    loadProfileSettings();
    
    // 更新界面
    updateUI();
    
    emit dialogShown();
}

void PreferencesDialog::closeEvent(QCloseEvent* event)
{
    // 检查是否有未保存的更改
    if (hasUnsavedChanges() && !confirmUnsavedChanges()) {
        event->ignore();
        return;
    }
    
    // 保存对话框状态
    saveDialogState();
    
    QDialog::closeEvent(event);
    emit dialogClosed();
}

void PreferencesDialog::changeEvent(QEvent* event)
{
    QDialog::changeEvent(event);
    
    if (event->type() == QEvent::LanguageChange) {
        // 重新翻译界面
        retranslateUI();
    } else if (event->type() == QEvent::StyleChange) {
        // 应用新主题
        applyTheme(d->currentTheme);
    } else if (event->type() == QEvent::WindowStateChange) {
        // 处理窗口状态变化
        updateButtonStates();
    }
}

void PreferencesDialog::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_F1:
            // 显示帮助
            showHelp();
            event->accept();
            break;
            
        case Qt::Key_F5:
            // 刷新界面
            refresh();
            event->accept();
            break;
            
        case Qt::Key_Escape:
            // ESC键关闭对话框
            if (hasUnsavedChanges() && !confirmUnsavedChanges()) {
                event->ignore();
                return;
            }
            reject();
            event->accept();
            break;
            
        default:
            // 处理快捷键
            if (event->modifiers() & Qt::ControlModifier) {
                switch (event->key()) {
                    case Qt::Key_S:
                        // Ctrl+S 应用设置
                        applySettings();
                        event->accept();
                        break;
                        
                    case Qt::Key_R:
                        // Ctrl+R 重置设置
                        resetToDefaults();
                        event->accept();
                        break;
                        
                    case Qt::Key_I:
                        // Ctrl+I 导入设置
                        showImportDialog();
                        event->accept();
                        break;
                        
                    case Qt::Key_E:
                        // Ctrl+E 导出设置
                        showExportDialog();
                        event->accept();
                        break;
                        
                    case Qt::Key_F:
                        // Ctrl+F 聚焦搜索框
                        if (d->searchBox) {
                            d->searchBox->setFocus();
                            d->searchBox->selectAll();
                        }
                        event->accept();
                        break;
                        
                    default:
                        QDialog::keyPressEvent(event);
                        break;
                }
            } else {
                QDialog::keyPressEvent(event);
            }
            break;
    }
}

void PreferencesDialog::updateButtonStates()
{
    /**
     * @brief 更新按钮状态
     * 根据当前设置状态更新对话框按钮的启用/禁用状态
     */
    if (!d->buttonBox) return;
    
    // 检查是否有未保存的更改
    bool hasChanges = hasUnsavedChanges();
    
    // 更新Apply按钮状态
    if (QPushButton* applyButton = d->buttonBox->button(QDialogButtonBox::Apply)) {
        applyButton->setEnabled(hasChanges);
    }
    
    // 更新OK按钮状态
    if (QPushButton* okButton = d->buttonBox->button(QDialogButtonBox::Ok)) {
        okButton->setEnabled(true); // OK按钮始终可用
    }
    
    // 更新Reset按钮状态
    if (QPushButton* resetButton = d->buttonBox->button(QDialogButtonBox::Reset)) {
        resetButton->setEnabled(hasChanges);
    }
}

void PreferencesDialog::retranslateUI()
{
    /**
     * @brief 重新翻译用户界面
     * 当语言设置改变时更新所有界面文本
     */
    setWindowTitle(tr("Preferences"));
    
    // 更新顶部控件文本
    if (d->topLayout) {
        // 查找并更新标签文本
        for (int i = 0; i < d->topLayout->count(); ++i) {
            QLayoutItem* item = d->topLayout->itemAt(i);
            if (QLabel* label = qobject_cast<QLabel*>(item->widget())) {
                QString text = label->text();
                if (text.contains("Profile")) {
                    label->setText(tr("Profile:"));
                } else if (text.contains("Search")) {
                    label->setText(tr("Search:"));
                }
            }
        }
    }
    
    // 更新搜索框占位符文本
    if (d->searchBox) {
        d->searchBox->setPlaceholderText(tr("Search settings..."));
    }
    
    // 更新预览模式复选框文本
    if (d->previewModeCheckBox) {
        d->previewModeCheckBox->setText(tr("Preview changes"));
    }
    
    // 更新页面列表
    updatePageList();
    
    // 更新所有页面的翻译
    for (auto& pageInfo : d->pages) {
        if (pageInfo.id == "general") {
            pageInfo.title = tr("General");
            pageInfo.description = tr("General application settings");
        } else if (pageInfo.id == "audio") {
            pageInfo.title = tr("Audio");
            pageInfo.description = tr("Audio device and quality settings");
        } else if (pageInfo.id == "video") {
            pageInfo.title = tr("Video");
            pageInfo.description = tr("Camera and video quality settings");
        } else if (pageInfo.id == "network") {
            pageInfo.title = tr("Network");
            pageInfo.description = tr("Network and connection settings");
        } else if (pageInfo.id == "ui") {
            pageInfo.title = tr("Interface");
            pageInfo.description = tr("User interface and appearance settings");
        } else if (pageInfo.id == "advanced") {
            pageInfo.title = tr("Advanced");
            pageInfo.description = tr("Advanced configuration options");
        }
        
        // 更新设置组件的翻译
        if (SettingsWidget* settingsWidget = qobject_cast<SettingsWidget*>(pageInfo.widget)) {
            // SettingsWidget没有retranslateUI方法，使用refresh代替
            settingsWidget->refresh();
        }
    }
}

void PreferencesDialog::applyTheme(const QString& theme)
{
    /**
     * @brief 应用主题到对话框
     * @param theme 主题名称
     */
    if (d->currentTheme == theme) {
        return; // 主题未改变，无需重新应用
    }
    
    d->currentTheme = theme;
    
    // 应用主题到主对话框
    QString styleSheet;
    
    if (theme == "dark") {
        // 深色主题样式
        styleSheet = QString(
            "QDialog {"
            "    background-color: #2b2b2b;"
            "    color: #ffffff;"
            "}"
            "QLabel {"
            "    color: #ffffff;"
            "}"
            "QLineEdit {"
            "    background-color: #3c3c3c;"
            "    border: 1px solid #555555;"
            "    color: #ffffff;"
            "    padding: 4px;"
            "}"
            "QComboBox {"
            "    background-color: #3c3c3c;"
            "    border: 1px solid #555555;"
            "    color: #ffffff;"
            "    padding: 4px;"
            "}"
            "QListWidget {"
            "    background-color: #3c3c3c;"
            "    border: 1px solid #555555;"
            "    color: #ffffff;"
            "}"
            "QPushButton {"
            "    background-color: #0078d4;"
            "    border: 1px solid #005a9e;"
            "    color: #ffffff;"
            "    padding: 6px 12px;"
            "    border-radius: 3px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #106ebe;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #005a9e;"
            "}"
        );
    } else if (theme == "light") {
        // 浅色主题样式
        styleSheet = QString(
            "QDialog {"
            "    background-color: #ffffff;"
            "    color: #000000;"
            "}"
            "QLabel {"
            "    color: #000000;"
            "}"
            "QLineEdit {"
            "    background-color: #ffffff;"
            "    border: 1px solid #cccccc;"
            "    color: #000000;"
            "    padding: 4px;"
            "}"
            "QComboBox {"
            "    background-color: #ffffff;"
            "    border: 1px solid #cccccc;"
            "    color: #000000;"
            "    padding: 4px;"
            "}"
            "QListWidget {"
            "    background-color: #ffffff;"
            "    border: 1px solid #cccccc;"
            "    color: #000000;"
            "}"
            "QPushButton {"
            "    background-color: #0078d4;"
            "    border: 1px solid #005a9e;"
            "    color: #ffffff;"
            "    padding: 6px 12px;"
            "    border-radius: 3px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #106ebe;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #005a9e;"
            "}"
        );
    } else {
        // 默认主题或系统主题
        styleSheet = "";
    }
    
    // 应用样式表到对话框
    setStyleSheet(styleSheet);
    
    // 应用主题到所有页面
    for (const auto& pageInfo : d->pages) {
        if (SettingsWidget* settingsWidget = qobject_cast<SettingsWidget*>(pageInfo.widget)) {
            // 如果SettingsWidget有applyTheme方法，调用它
            // settingsWidget->applyTheme(theme);
        }
    }
    
    // 发出主题改变信号
    emit themeChanged(theme);
}

void PreferencesDialog::applyAndClose()
{
    /**
     * @brief 应用设置并关闭对话框
     */
    if (applySettings()) {
        accept();
    }
}

// Static convenience methods
void PreferencesDialog::restoreDialogState()
{
    // 恢复对话框的几何状态
    if (d->preferencesHandler) {
        QVariant geometry = d->preferencesHandler->preference("dialog", "geometry");
        if (geometry.isValid()) {
            restoreGeometry(geometry.toByteArray());
        }
        
        // 恢复分割器状态
        QVariant splitterState = d->preferencesHandler->preference("dialog", "splitterState");
        if (splitterState.isValid() && d->splitter) {
            d->splitter->restoreState(splitterState.toByteArray());
        }
        
        // 恢复当前页面
        QString lastPage = d->preferencesHandler->preference("dialog", "lastPage").toString();
        if (!lastPage.isEmpty()) {
            for (int i = 0; i < d->pageList->count(); ++i) {
                QListWidgetItem* item = d->pageList->item(i);
                if (item && item->data(Qt::UserRole).toString() == lastPage) {
                    d->pageList->setCurrentItem(item);
                    break;
                }
            }
        }
    }
}

void PreferencesDialog::saveDialogState()
{
    // 保存对话框的几何状态
    if (d->preferencesHandler) {
        d->preferencesHandler->setPreference("dialog", "geometry", saveGeometry());
        
        // 保存分割器状态
        if (d->splitter) {
            d->preferencesHandler->setPreference("dialog", "splitterState", d->splitter->saveState());
        }
        
        // 保存当前页面
        QListWidgetItem* currentItem = d->pageList->currentItem();
        if (currentItem) {
            d->preferencesHandler->setPreference("dialog", "lastPage", currentItem->data(Qt::UserRole).toString());
        }
    }
}

void PreferencesDialog::updateUI()
{
    // 更新界面元素的状态
    if (d->settingsWidget) {
        d->settingsWidget->refresh();
    }
    
    // 更新按钮状态
    updateButtonStates();
    
    // 更新配置文件选择器
    if (d->profileSelector) {
        d->profileSelector->clear();
        d->profileSelector->addItems(availableProfiles());
        d->profileSelector->setCurrentText(d->currentProfile);
    }
    
    // 更新预览模式复选框
    if (d->previewModeCheckBox) {
        d->previewModeCheckBox->setChecked(d->previewModeEnabled);
    }
    
    // 应用当前主题
    if (!d->currentTheme.isEmpty()) {
        applyTheme(d->currentTheme);
    }
}

int PreferencesDialog::showPreferences(QWidget* parent, PreferencesHandler* handler)
{
    PreferencesDialog dialog(parent);
    dialog.setPreferencesHandler(handler);
    return dialog.exec();
}