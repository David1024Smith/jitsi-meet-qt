#include "SettingsWidget.h"
#include "../include/SettingsManager.h"
#include "../include/PreferencesHandler.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QScrollArea>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QPushButton>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QApplication>
#include <QStyle>
#include <QHeaderView>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

class SettingsWidget::Private
{
public:
    Private()
        : settingsManager(nullptr)
        , preferencesHandler(nullptr)
        , currentViewMode(TabView)
        , autoSaveEnabled(true)
        , showAdvancedEnabled(false)
        , hasUnsavedChanges(false)
        , autoSaveTimer(new QTimer())
    {
        autoSaveTimer->setSingleShot(true);
        autoSaveTimer->setInterval(1000); // 1 second delay
    }

    // Core components
    SettingsManager* settingsManager;
    PreferencesHandler* preferencesHandler;
    
    // UI components
    QVBoxLayout* mainLayout;
    QTabWidget* tabWidget;
    QTreeWidget* treeWidget;
    QSplitter* splitter;
    QHBoxLayout* buttonLayout;
    QPushButton* applyButton;
    QPushButton* cancelButton;
    QPushButton* resetButton;
    
    // Settings
    QString currentCategory;
    ViewMode currentViewMode;
    bool autoSaveEnabled;
    bool showAdvancedEnabled;
    bool hasUnsavedChanges;
    QString currentTheme;
    QString searchText;
    
    // Data
    QList<SettingDescriptor> settingDescriptors;
    QMap<QString, QString> categoryDisplayNames;
    QMap<QString, QString> categoryIcons;
    QMap<QString, QWidget*> settingWidgets;
    QMap<QString, QVariant> currentValues;
    QMap<QString, QVariant> originalValues;
    QStringList validationErrors;
    
    // Timers and filters
    QTimer* autoSaveTimer;
    std::function<bool(const SettingDescriptor&)> currentFilter;
};

SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent)
    , d(std::make_unique<Private>())
{
    setupUI();
    
    // Connect auto-save timer
    connect(d->autoSaveTimer, &QTimer::timeout, this, [this]() {
        if (d->autoSaveEnabled && d->hasUnsavedChanges) {
            saveSettings();
        }
    });
}

SettingsWidget::~SettingsWidget() = default;Q
String SettingsWidget::currentCategory() const
{
    return d->currentCategory;
}

void SettingsWidget::setCurrentCategory(const QString& category)
{
    if (d->currentCategory != category) {
        d->currentCategory = category;
        emit currentCategoryChanged(category);
        
        // Update UI to show the selected category
        if (d->tabWidget) {
            for (int i = 0; i < d->tabWidget->count(); ++i) {
                if (d->tabWidget->tabText(i) == getCategoryDisplayName(category)) {
                    d->tabWidget->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
}

bool SettingsWidget::isAutoSaveEnabled() const
{
    return d->autoSaveEnabled;
}

void SettingsWidget::setAutoSaveEnabled(bool enabled)
{
    if (d->autoSaveEnabled != enabled) {
        d->autoSaveEnabled = enabled;
        emit autoSaveChanged(enabled);
    }
}

bool SettingsWidget::isShowAdvancedEnabled() const
{
    return d->showAdvancedEnabled;
}

void SettingsWidget::setShowAdvancedEnabled(bool enabled)
{
    if (d->showAdvancedEnabled != enabled) {
        d->showAdvancedEnabled = enabled;
        emit showAdvancedChanged(enabled);
        refresh(); // Refresh to show/hide advanced settings
    }
}

SettingsWidget::ViewMode SettingsWidget::viewMode() const
{
    return d->currentViewMode;
}

void SettingsWidget::setViewMode(ViewMode mode)
{
    if (d->currentViewMode != mode) {
        d->currentViewMode = mode;
        emit viewModeChanged(mode);
        setupUI(); // Rebuild UI for new mode
    }
}

void SettingsWidget::setSettingsManager(SettingsManager* manager)
{
    d->settingsManager = manager;
}

SettingsManager* SettingsWidget::settingsManager() const
{
    return d->settingsManager;
}

void SettingsWidget::setPreferencesHandler(PreferencesHandler* handler)
{
    d->preferencesHandler = handler;
}

PreferencesHandler* SettingsWidget::preferencesHandler() const
{
    return d->preferencesHandler;
}vo
id SettingsWidget::addSetting(const SettingDescriptor& descriptor)
{
    d->settingDescriptors.append(descriptor);
    
    // Add category if it doesn't exist
    if (!d->categoryDisplayNames.contains(descriptor.category)) {
        addCategory(descriptor.category);
    }
    
    // Store original value
    d->originalValues[descriptor.key] = descriptor.defaultValue;
    d->currentValues[descriptor.key] = descriptor.defaultValue;
    
    refresh();
}

void SettingsWidget::removeSetting(const QString& key)
{
    d->settingDescriptors.removeAll(
        std::find_if(d->settingDescriptors.begin(), d->settingDescriptors.end(),
                     [key](const SettingDescriptor& desc) { return desc.key == key; }));
    
    d->settingWidgets.remove(key);
    d->currentValues.remove(key);
    d->originalValues.remove(key);
    
    refresh();
}

SettingsWidget::SettingDescriptor SettingsWidget::getSettingDescriptor(const QString& key) const
{
    auto it = std::find_if(d->settingDescriptors.begin(), d->settingDescriptors.end(),
                           [key](const SettingDescriptor& desc) { return desc.key == key; });
    
    if (it != d->settingDescriptors.end()) {
        return *it;
    }
    
    return SettingDescriptor();
}

QList<SettingsWidget::SettingDescriptor> SettingsWidget::getAllSettings() const
{
    return d->settingDescriptors;
}

QList<SettingsWidget::SettingDescriptor> SettingsWidget::getCategorySettings(const QString& category) const
{
    QList<SettingDescriptor> categorySettings;
    
    for (const auto& descriptor : d->settingDescriptors) {
        if (descriptor.category == category) {
            // Filter advanced settings if not enabled
            if (!descriptor.isAdvanced || d->showAdvancedEnabled) {
                categorySettings.append(descriptor);
            }
        }
    }
    
    return categorySettings;
}

void SettingsWidget::addSettings(const QList<SettingDescriptor>& descriptors)
{
    for (const auto& descriptor : descriptors) {
        addSetting(descriptor);
    }
}

bool SettingsWidget::loadSettingsFromJson(const QJsonObject& json)
{
    try {
        d->settingDescriptors.clear();
        
        QJsonArray settingsArray = json["settings"].toArray();
        for (const auto& value : settingsArray) {
            QJsonObject settingObj = value.toObject();
            
            SettingDescriptor descriptor;
            descriptor.key = settingObj["key"].toString();
            descriptor.displayName = settingObj["displayName"].toString();
            descriptor.description = settingObj["description"].toString();
            descriptor.category = settingObj["category"].toString();
            descriptor.defaultValue = settingObj["defaultValue"].toVariant();
            descriptor.isAdvanced = settingObj["isAdvanced"].toBool(false);
            descriptor.isReadOnly = settingObj["isReadOnly"].toBool(false);
            descriptor.tooltip = settingObj["tooltip"].toString();
            descriptor.placeholder = settingObj["placeholder"].toString();
            
            // Parse setting type
            QString typeStr = settingObj["type"].toString();
            if (typeStr == "string") descriptor.type = StringSetting;
            else if (typeStr == "integer") descriptor.type = IntegerSetting;
            else if (typeStr == "double") descriptor.type = DoubleSetting;
            else if (typeStr == "boolean") descriptor.type = BooleanSetting;
            else if (typeStr == "enum") descriptor.type = EnumSetting;
            else if (typeStr == "path") descriptor.type = PathSetting;
            else if (typeStr == "color") descriptor.type = ColorSetting;
            else if (typeStr == "font") descriptor.type = FontSetting;
            else if (typeStr == "datetime") descriptor.type = DateTimeSetting;
            else if (typeStr == "list") descriptor.type = ListSetting;
            else descriptor.type = CustomSetting;
            
            // Parse enum values
            QJsonArray enumArray = settingObj["enumValues"].toArray();
            for (const auto& enumValue : enumArray) {
                descriptor.enumValues.append(enumValue.toString());
            }
            
            addSetting(descriptor);
        }
        
        return true;
    } catch (...) {
        return false;
    }
}void S
ettingsWidget::setupUI()
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
    
    // Setup based on view mode
    switch (d->currentViewMode) {
        case TabView:
            setupTabView();
            break;
        case TreeView:
            setupTreeView();
            break;
        case ListView:
            setupListView();
            break;
        case WizardView:
            setupWizardView();
            break;
    }
    
    // Setup button layout
    d->buttonLayout = new QHBoxLayout();
    d->buttonLayout->addStretch();
    
    d->resetButton = new QPushButton(tr("Reset"), this);
    d->resetButton->setToolTip(tr("Reset all settings to default values"));
    connect(d->resetButton, &QPushButton::clicked, this, &SettingsWidget::onResetButtonClicked);
    d->buttonLayout->addWidget(d->resetButton);
    
    d->cancelButton = new QPushButton(tr("Cancel"), this);
    d->cancelButton->setToolTip(tr("Cancel changes and close"));
    connect(d->cancelButton, &QPushButton::clicked, this, &SettingsWidget::onCancelButtonClicked);
    d->buttonLayout->addWidget(d->cancelButton);
    
    d->applyButton = new QPushButton(tr("Apply"), this);
    d->applyButton->setToolTip(tr("Apply and save settings"));
    d->applyButton->setDefault(true);
    connect(d->applyButton, &QPushButton::clicked, this, &SettingsWidget::onApplyButtonClicked);
    d->buttonLayout->addWidget(d->applyButton);
    
    d->mainLayout->addLayout(d->buttonLayout);
}

void SettingsWidget::setupTabView()
{
    d->tabWidget = new QTabWidget(this);
    d->tabWidget->setTabPosition(QTabWidget::North);
    d->tabWidget->setMovable(false);
    
    // Group settings by category
    QMap<QString, QList<SettingDescriptor>> categorizedSettings;
    for (const auto& descriptor : d->settingDescriptors) {
        if (!descriptor.isAdvanced || d->showAdvancedEnabled) {
            categorizedSettings[descriptor.category].append(descriptor);
        }
    }
    
    // Create tabs for each category
    for (auto it = categorizedSettings.begin(); it != categorizedSettings.end(); ++it) {
        QString category = it.key();
        QList<SettingDescriptor> settings = it.value();
        
        QScrollArea* scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        
        QWidget* categoryWidget = new QWidget();
        QVBoxLayout* categoryLayout = new QVBoxLayout(categoryWidget);
        categoryLayout->setContentsMargins(12, 12, 12, 12);
        categoryLayout->setSpacing(8);
        
        // Create settings widgets for this category
        for (const auto& descriptor : settings) {
            createSettingWidget(descriptor, categoryWidget);
        }
        
        categoryLayout->addStretch();
        scrollArea->setWidget(categoryWidget);
        
        QString displayName = getCategoryDisplayName(category);
        d->tabWidget->addTab(scrollArea, displayName);
    }
    
    connect(d->tabWidget, &QTabWidget::currentChanged, this, &SettingsWidget::onCategorySelectionChanged);
    d->mainLayout->addWidget(d->tabWidget);
}

void SettingsWidget::setupTreeView()
{
    d->splitter = new QSplitter(Qt::Horizontal, this);
    
    // Create tree widget for categories
    d->treeWidget = new QTreeWidget();
    d->treeWidget->setHeaderHidden(true);
    d->treeWidget->setRootIsDecorated(false);
    d->treeWidget->setMaximumWidth(200);
    
    // Populate tree with categories
    QStringList categories = this->categories();
    for (const QString& category : categories) {
        QTreeWidgetItem* item = new QTreeWidgetItem(d->treeWidget);
        item->setText(0, getCategoryDisplayName(category));
        item->setData(0, Qt::UserRole, category);
        
        QString iconPath = getCategoryIcon(category);
        if (!iconPath.isEmpty()) {
            item->setIcon(0, QIcon(iconPath));
        }
    }
    
    // Create content area
    QScrollArea* contentArea = new QScrollArea();
    contentArea->setWidgetResizable(true);
    contentArea->setFrameShape(QFrame::NoFrame);
    
    d->splitter->addWidget(d->treeWidget);
    d->splitter->addWidget(contentArea);
    d->splitter->setStretchFactor(0, 0);
    d->splitter->setStretchFactor(1, 1);
    
    connect(d->treeWidget, &QTreeWidget::currentItemChanged, this, &SettingsWidget::onCategorySelectionChanged);
    d->mainLayout->addWidget(d->splitter);
}

void SettingsWidget::setupListView()
{
    // Similar to tab view but in a single scrollable list
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget* contentWidget = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(12, 12, 12, 12);
    contentLayout->setSpacing(16);
    
    // Group by categories but show all in one list
    QStringList categories = this->categories();
    for (const QString& category : categories) {
        QList<SettingDescriptor> categorySettings = getCategorySettings(category);
        if (categorySettings.isEmpty()) continue;
        
        // Category header
        QLabel* categoryLabel = new QLabel(getCategoryDisplayName(category));
        QFont headerFont = categoryLabel->font();
        headerFont.setBold(true);
        headerFont.setPointSize(headerFont.pointSize() + 2);
        categoryLabel->setFont(headerFont);
        contentLayout->addWidget(categoryLabel);
        
        // Category settings
        for (const auto& descriptor : categorySettings) {
            createSettingWidget(descriptor, contentWidget);
        }
        
        // Add separator
        QFrame* separator = new QFrame();
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Sunken);
        contentLayout->addWidget(separator);
    }
    
    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    d->mainLayout->addWidget(scrollArea);
}

void SettingsWidget::setupWizardView()
{
    // Simplified wizard-like interface
    setupTabView(); // Use tab view as base for wizard
    
    if (d->tabWidget) {
        d->tabWidget->setTabPosition(QTabWidget::West);
        
        // Add navigation buttons
        QHBoxLayout* navLayout = new QHBoxLayout();
        QPushButton* prevButton = new QPushButton(tr("< Previous"));
        QPushButton* nextButton = new QPushButton(tr("Next >"));
        
        navLayout->addWidget(prevButton);
        navLayout->addStretch();
        navLayout->addWidget(nextButton);
        
        d->mainLayout->insertLayout(d->mainLayout->count() - 1, navLayout);
        
        // Connect navigation
        connect(prevButton, &QPushButton::clicked, [this]() {
            int current = d->tabWidget->currentIndex();
            if (current > 0) {
                d->tabWidget->setCurrentIndex(current - 1);
            }
        });
        
        connect(nextButton, &QPushButton::clicked, [this]() {
            int current = d->tabWidget->currentIndex();
            if (current < d->tabWidget->count() - 1) {
                d->tabWidget->setCurrentIndex(current + 1);
            }
        });
    }
}void
 SettingsWidget::createSettingWidget(const SettingDescriptor& descriptor, QWidget* parent)
{
    QWidget* settingWidget = nullptr;
    
    switch (descriptor.type) {
        case StringSetting:
            settingWidget = createStringWidget(descriptor);
            break;
        case IntegerSetting:
            settingWidget = createIntegerWidget(descriptor);
            break;
        case DoubleSetting:
            settingWidget = createDoubleWidget(descriptor);
            break;
        case BooleanSetting:
            settingWidget = createBooleanWidget(descriptor);
            break;
        case EnumSetting:
            settingWidget = createEnumWidget(descriptor);
            break;
        case PathSetting:
            settingWidget = createPathWidget(descriptor);
            break;
        case ColorSetting:
            settingWidget = createColorWidget(descriptor);
            break;
        case FontSetting:
            settingWidget = createFontWidget(descriptor);
            break;
        case DateTimeSetting:
            settingWidget = createDateTimeWidget(descriptor);
            break;
        case ListSetting:
            settingWidget = createListWidget(descriptor);
            break;
        default:
            settingWidget = createStringWidget(descriptor); // Fallback
            break;
    }
    
    if (settingWidget) {
        d->settingWidgets[descriptor.key] = settingWidget;
        connectWidgetSignals(descriptor.key, settingWidget);
        
        // Add to parent layout
        if (parent && parent->layout()) {
            parent->layout()->addWidget(settingWidget);
        }
    }
}

QWidget* SettingsWidget::createStringWidget(const SettingDescriptor& descriptor)
{
    QWidget* container = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(descriptor.displayName);
    label->setToolTip(descriptor.description);
    layout->addWidget(label);
    
    QLineEdit* lineEdit = new QLineEdit();
    lineEdit->setObjectName(descriptor.key);
    lineEdit->setText(d->currentValues[descriptor.key].toString());
    lineEdit->setPlaceholderText(descriptor.placeholder);
    lineEdit->setReadOnly(descriptor.isReadOnly);
    lineEdit->setToolTip(descriptor.tooltip);
    layout->addWidget(lineEdit);
    
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    
    return container;
}

QWidget* SettingsWidget::createIntegerWidget(const SettingDescriptor& descriptor)
{
    QWidget* container = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(descriptor.displayName);
    label->setToolTip(descriptor.description);
    layout->addWidget(label);
    
    QSpinBox* spinBox = new QSpinBox();
    spinBox->setObjectName(descriptor.key);
    spinBox->setValue(d->currentValues[descriptor.key].toInt());
    spinBox->setReadOnly(descriptor.isReadOnly);
    spinBox->setToolTip(descriptor.tooltip);
    
    if (descriptor.minValue.isValid()) {
        spinBox->setMinimum(descriptor.minValue.toInt());
    }
    if (descriptor.maxValue.isValid()) {
        spinBox->setMaximum(descriptor.maxValue.toInt());
    }
    
    layout->addWidget(spinBox);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    
    return container;
}

QWidget* SettingsWidget::createDoubleWidget(const SettingDescriptor& descriptor)
{
    QWidget* container = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(descriptor.displayName);
    label->setToolTip(descriptor.description);
    layout->addWidget(label);
    
    QDoubleSpinBox* spinBox = new QDoubleSpinBox();
    spinBox->setObjectName(descriptor.key);
    spinBox->setValue(d->currentValues[descriptor.key].toDouble());
    spinBox->setReadOnly(descriptor.isReadOnly);
    spinBox->setToolTip(descriptor.tooltip);
    
    if (descriptor.minValue.isValid()) {
        spinBox->setMinimum(descriptor.minValue.toDouble());
    }
    if (descriptor.maxValue.isValid()) {
        spinBox->setMaximum(descriptor.maxValue.toDouble());
    }
    
    layout->addWidget(spinBox);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    
    return container;
}

QWidget* SettingsWidget::createBooleanWidget(const SettingDescriptor& descriptor)
{
    QCheckBox* checkBox = new QCheckBox(descriptor.displayName);
    checkBox->setObjectName(descriptor.key);
    checkBox->setChecked(d->currentValues[descriptor.key].toBool());
    checkBox->setEnabled(!descriptor.isReadOnly);
    checkBox->setToolTip(descriptor.tooltip.isEmpty() ? descriptor.description : descriptor.tooltip);
    
    return checkBox;
}

QWidget* SettingsWidget::createEnumWidget(const SettingDescriptor& descriptor)
{
    QWidget* container = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(descriptor.displayName);
    label->setToolTip(descriptor.description);
    layout->addWidget(label);
    
    QComboBox* comboBox = new QComboBox();
    comboBox->setObjectName(descriptor.key);
    comboBox->addItems(descriptor.enumValues);
    comboBox->setCurrentText(d->currentValues[descriptor.key].toString());
    comboBox->setEnabled(!descriptor.isReadOnly);
    comboBox->setToolTip(descriptor.tooltip);
    
    layout->addWidget(comboBox);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    
    return container;
}

QWidget* SettingsWidget::createPathWidget(const SettingDescriptor& descriptor)
{
    QWidget* container = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(descriptor.displayName);
    label->setToolTip(descriptor.description);
    layout->addWidget(label);
    
    QLineEdit* lineEdit = new QLineEdit();
    lineEdit->setObjectName(descriptor.key);
    lineEdit->setText(d->currentValues[descriptor.key].toString());
    lineEdit->setReadOnly(descriptor.isReadOnly);
    lineEdit->setToolTip(descriptor.tooltip);
    
    QPushButton* browseButton = new QPushButton(tr("Browse..."));
    browseButton->setEnabled(!descriptor.isReadOnly);
    
    connect(browseButton, &QPushButton::clicked, [this, lineEdit]() {
        QString path = QFileDialog::getExistingDirectory(this, tr("Select Directory"), lineEdit->text());
        if (!path.isEmpty()) {
            lineEdit->setText(path);
        }
    });
    
    layout->addWidget(lineEdit);
    layout->addWidget(browseButton);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    layout->setStretch(2, 0);
    
    return container;
}

// Placeholder implementations for other widget types
QWidget* SettingsWidget::createColorWidget(const SettingDescriptor& descriptor)
{
    return createStringWidget(descriptor); // Simplified for now
}

QWidget* SettingsWidget::createFontWidget(const SettingDescriptor& descriptor)
{
    return createStringWidget(descriptor); // Simplified for now
}

QWidget* SettingsWidget::createDateTimeWidget(const SettingDescriptor& descriptor)
{
    return createStringWidget(descriptor); // Simplified for now
}

QWidget* SettingsWidget::createListWidget(const SettingDescriptor& descriptor)
{
    return createStringWidget(descriptor); // Simplified for now
}void
 SettingsWidget::connectWidgetSignals(const QString& key, QWidget* widget)
{
    // Connect appropriate signals based on widget type
    if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget->findChild<QLineEdit*>(key))) {
        connect(lineEdit, &QLineEdit::textChanged, this, &SettingsWidget::onSettingValueChanged);
    } else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget->findChild<QSpinBox*>(key))) {
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWidget::onSettingValueChanged);
    } else if (QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget->findChild<QDoubleSpinBox*>(key))) {
        connect(doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsWidget::onSettingValueChanged);
    } else if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
        connect(checkBox, &QCheckBox::toggled, this, &SettingsWidget::onSettingValueChanged);
    } else if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget->findChild<QComboBox*>(key))) {
        connect(comboBox, &QComboBox::currentTextChanged, this, &SettingsWidget::onSettingValueChanged);
    }
}

void SettingsWidget::onSettingValueChanged()
{
    QObject* sender = this->sender();
    if (!sender) return;
    
    QString key = sender->objectName();
    if (key.isEmpty()) return;
    
    QVariant newValue = getWidgetValue(key);
    QVariant oldValue = d->currentValues[key];
    
    if (newValue != oldValue) {
        d->currentValues[key] = newValue;
        d->hasUnsavedChanges = true;
        
        emit settingChanged(key, newValue);
        
        // Start auto-save timer if enabled
        if (d->autoSaveEnabled) {
            d->autoSaveTimer->start();
        }
    }
}

QVariant SettingsWidget::getWidgetValue(const QString& key) const
{
    QWidget* widget = d->settingWidgets[key];
    if (!widget) return QVariant();
    
    if (QLineEdit* lineEdit = widget->findChild<QLineEdit*>(key)) {
        return lineEdit->text();
    } else if (QSpinBox* spinBox = widget->findChild<QSpinBox*>(key)) {
        return spinBox->value();
    } else if (QDoubleSpinBox* doubleSpinBox = widget->findChild<QDoubleSpinBox*>(key)) {
        return doubleSpinBox->value();
    } else if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
        return checkBox->isChecked();
    } else if (QComboBox* comboBox = widget->findChild<QComboBox*>(key)) {
        return comboBox->currentText();
    }
    
    return QVariant();
}

void SettingsWidget::updateWidgetValue(const QString& key, const QVariant& value)
{
    QWidget* widget = d->settingWidgets[key];
    if (!widget) return;
    
    if (QLineEdit* lineEdit = widget->findChild<QLineEdit*>(key)) {
        lineEdit->setText(value.toString());
    } else if (QSpinBox* spinBox = widget->findChild<QSpinBox*>(key)) {
        spinBox->setValue(value.toInt());
    } else if (QDoubleSpinBox* doubleSpinBox = widget->findChild<QDoubleSpinBox*>(key)) {
        doubleSpinBox->setValue(value.toDouble());
    } else if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
        checkBox->setChecked(value.toBool());
    } else if (QComboBox* comboBox = widget->findChild<QComboBox*>(key)) {
        comboBox->setCurrentText(value.toString());
    }
}

// Category management methods
void SettingsWidget::addCategory(const QString& category, const QString& displayName, const QString& icon)
{
    if (!d->categoryDisplayNames.contains(category)) {
        d->categoryDisplayNames[category] = displayName.isEmpty() ? category : displayName;
        if (!icon.isEmpty()) {
            d->categoryIcons[category] = icon;
        }
    }
}

void SettingsWidget::removeCategory(const QString& category)
{
    d->categoryDisplayNames.remove(category);
    d->categoryIcons.remove(category);
    
    // Remove all settings in this category
    d->settingDescriptors.erase(
        std::remove_if(d->settingDescriptors.begin(), d->settingDescriptors.end(),
                       [category](const SettingDescriptor& desc) { return desc.category == category; }),
        d->settingDescriptors.end());
    
    refresh();
}

QStringList SettingsWidget::categories() const
{
    return d->categoryDisplayNames.keys();
}

QString SettingsWidget::getCategoryDisplayName(const QString& category) const
{
    return d->categoryDisplayNames.value(category, category);
}

void SettingsWidget::setCategoryDisplayName(const QString& category, const QString& displayName)
{
    d->categoryDisplayNames[category] = displayName;
}

QString SettingsWidget::getCategoryIcon(const QString& category) const
{
    return d->categoryIcons.value(category);
}

void SettingsWidget::setCategoryIcon(const QString& category, const QString& icon)
{
    d->categoryIcons[category] = icon;
}

// Value operations
void SettingsWidget::setValue(const QString& key, const QVariant& value)
{
    d->currentValues[key] = value;
    updateWidgetValue(key, value);
}

QVariant SettingsWidget::value(const QString& key) const
{
    return d->currentValues.value(key);
}

void SettingsWidget::resetValue(const QString& key)
{
    SettingDescriptor descriptor = getSettingDescriptor(key);
    if (!descriptor.key.isEmpty()) {
        setValue(key, descriptor.defaultValue);
    }
}

void SettingsWidget::resetCategory(const QString& category)
{
    QList<SettingDescriptor> categorySettings = getCategorySettings(category);
    for (const auto& descriptor : categorySettings) {
        resetValue(descriptor.key);
    }
}

void SettingsWidget::resetAll()
{
    for (const auto& descriptor : d->settingDescriptors) {
        resetValue(descriptor.key);
    }
}

// Validation and saving
bool SettingsWidget::validateSettings()
{
    d->validationErrors.clear();
    
    for (const auto& descriptor : d->settingDescriptors) {
        QVariant currentValue = d->currentValues[descriptor.key];
        
        // Basic validation
        if (descriptor.type == IntegerSetting) {
            int intValue = currentValue.toInt();
            if (descriptor.minValue.isValid() && intValue < descriptor.minValue.toInt()) {
                d->validationErrors << QString("%1: Value %2 is below minimum %3")
                                          .arg(descriptor.displayName)
                                          .arg(intValue)
                                          .arg(descriptor.minValue.toInt());
            }
            if (descriptor.maxValue.isValid() && intValue > descriptor.maxValue.toInt()) {
                d->validationErrors << QString("%1: Value %2 is above maximum %3")
                                          .arg(descriptor.displayName)
                                          .arg(intValue)
                                          .arg(descriptor.maxValue.toInt());
            }
        }
        
        // Add more validation rules as needed
    }
    
    bool isValid = d->validationErrors.isEmpty();
    emit validationCompleted(isValid, d->validationErrors);
    
    return isValid;
}

QStringList SettingsWidget::validationErrors() const
{
    return d->validationErrors;
}

bool SettingsWidget::saveSettings()
{
    if (!validateSettings()) {
        return false;
    }
    
    bool success = true;
    
    if (d->settingsManager) {
        // Save through settings manager
        for (auto it = d->currentValues.begin(); it != d->currentValues.end(); ++it) {
            // This would call the actual settings manager save method
            // success &= d->settingsManager->setValue(it.key(), it.value());
        }
    }
    
    if (success) {
        d->originalValues = d->currentValues;
        d->hasUnsavedChanges = false;
    }
    
    emit settingsSaved(success);
    return success;
}

bool SettingsWidget::loadSettings()
{
    bool success = true;
    
    if (d->settingsManager) {
        // Load through settings manager
        for (const auto& descriptor : d->settingDescriptors) {
            // QVariant value = d->settingsManager->value(descriptor.key, descriptor.defaultValue);
            // d->currentValues[descriptor.key] = value;
            // updateWidgetValue(descriptor.key, value);
        }
    }
    
    if (success) {
        d->originalValues = d->currentValues;
        d->hasUnsavedChanges = false;
    }
    
    emit settingsLoaded(success);
    return success;
}

bool SettingsWidget::hasUnsavedChanges() const
{
    return d->hasUnsavedChanges;
}

// Slots
void SettingsWidget::refresh()
{
    setupUI();
}

void SettingsWidget::onCategorySelectionChanged()
{
    if (d->tabWidget) {
        int currentIndex = d->tabWidget->currentIndex();
        if (currentIndex >= 0) {
            QString tabText = d->tabWidget->tabText(currentIndex);
            // Find category by display name
            for (auto it = d->categoryDisplayNames.begin(); it != d->categoryDisplayNames.end(); ++it) {
                if (it.value() == tabText) {
                    setCurrentCategory(it.key());
                    break;
                }
            }
        }
    } else if (d->treeWidget) {
        QTreeWidgetItem* current = d->treeWidget->currentItem();
        if (current) {
            QString category = current->data(0, Qt::UserRole).toString();
            setCurrentCategory(category);
        }
    }
}

void SettingsWidget::onApplyButtonClicked()
{
    applySettings();
}

void SettingsWidget::onCancelButtonClicked()
{
    cancelChanges();
}

void SettingsWidget::onResetButtonClicked()
{
    restoreDefaults();
}

void SettingsWidget::applySettings()
{
    saveSettings();
}

void SettingsWidget::cancelChanges()
{
    // Restore original values
    d->currentValues = d->originalValues;
    for (auto it = d->currentValues.begin(); it != d->currentValues.end(); ++it) {
        updateWidgetValue(it.key(), it.value());
    }
    d->hasUnsavedChanges = false;
}

void SettingsWidget::restoreDefaults()
{
    resetAll();
}