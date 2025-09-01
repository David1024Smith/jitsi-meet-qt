#include "ConfigEditor.h"
#include "../validators/ConfigValidator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QTabWidget>
#include <QToolBar>
#include <QAction>
#include <QMenu>
#include <QCompleter>
#include <QSyntaxHighlighter>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QDebug>

class ConfigEditor::Private
{
public:
    Private()
        : currentMode(TextMode)
        , currentFormat(JsonFormat)
        , readOnly(false)
        , syntaxHighlightingEnabled(true)
        , autoValidationEnabled(true)
        , isModified(false)
        , validator(nullptr)
        , validationTimer(new QTimer())
        , syntaxHighlighter(nullptr)
    {
        validationTimer->setSingleShot(true);
        validationTimer->setInterval(500); // 500ms delay
    }

    // Core settings
    EditorMode currentMode;
    ConfigFormat currentFormat;
    bool readOnly;
    bool syntaxHighlightingEnabled;
    bool autoValidationEnabled;
    bool isModified;
    QString currentFilePath;
    
    // UI components
    QVBoxLayout* mainLayout;
    QToolBar* toolBar;
    QSplitter* splitter;
    QTreeWidget* treeWidget;
    QPlainTextEdit* textEditor;
    QTabWidget* tabWidget;
    QLabel* statusLabel;
    
    // Data
    QJsonObject currentConfig;
    QString configText;
    ValidationStatus currentValidationStatus;
    QStringList validationErrors;
    QStringList validationWarnings;
    
    // Tools
    ConfigValidator* validator;
    QTimer* validationTimer;
    QSyntaxHighlighter* syntaxHighlighter;
    QCompleter* completer;
    
    // Editor options
    EditorOptions options;
    
    // Bookmarks
    QMap<int, QString> bookmarks;
};

ConfigEditor::ConfigEditor(QWidget* parent)
    : QWidget(parent)
    , d(std::make_unique<Private>())
{
    setupUI();
    connectSignals();
    
    // Set default content
    setConfigText("{\n    \n}");
}

ConfigEditor::~ConfigEditor() = default;

ConfigEditor::EditorMode ConfigEditor::editorMode() const
{
    return d->currentMode;
}

void ConfigEditor::setEditorMode(EditorMode mode)
{
    if (d->currentMode != mode) {
        d->currentMode = mode;
        emit editorModeChanged(mode);
        setupUI(); // Rebuild UI for new mode
    }
}

QString ConfigEditor::configFormat() const
{
    return formatToString(d->currentFormat);
}

void ConfigEditor::setConfigFormat(const QString& format)
{
    ConfigFormat newFormat = stringToFormat(format);
    if (d->currentFormat != newFormat) {
        d->currentFormat = newFormat;
        emit configFormatChanged(format);
        
        // Update syntax highlighting
        if (d->syntaxHighlightingEnabled) {
            setupSyntaxHighlighter();
        }
    }
}

void ConfigEditor::setConfigFormat(ConfigFormat format)
{
    setConfigFormat(formatToString(format));
}

ConfigEditor::ConfigFormat ConfigEditor::configFormatEnum() const
{
    return d->currentFormat;
}

bool ConfigEditor::isReadOnly() const
{
    return d->readOnly;
}

void ConfigEditor::setReadOnly(bool readOnly)
{
    if (d->readOnly != readOnly) {
        d->readOnly = readOnly;
        
        if (d->textEditor) {
            d->textEditor->setReadOnly(readOnly);
        }
        
        if (d->treeWidget) {
            d->treeWidget->setEnabled(!readOnly);
        }
        
        emit readOnlyChanged(readOnly);
    }
}

bool ConfigEditor::isSyntaxHighlightingEnabled() const
{
    return d->syntaxHighlightingEnabled;
}

void ConfigEditor::setSyntaxHighlightingEnabled(bool enabled)
{
    if (d->syntaxHighlightingEnabled != enabled) {
        d->syntaxHighlightingEnabled = enabled;
        
        if (enabled) {
            setupSyntaxHighlighter();
        } else if (d->syntaxHighlighter) {
            d->syntaxHighlighter->setDocument(nullptr);
        }
        
        emit syntaxHighlightingChanged(enabled);
    }
}

bool ConfigEditor::isAutoValidationEnabled() const
{
    return d->autoValidationEnabled;
}

void ConfigEditor::setAutoValidationEnabled(bool enabled)
{
    if (d->autoValidationEnabled != enabled) {
        d->autoValidationEnabled = enabled;
        emit autoValidationChanged(enabled);
    }
}

void ConfigEditor::setConfig(const QJsonObject& config)
{
    d->currentConfig = config;
    
    // Update text editor
    QJsonDocument doc(config);
    QString text = doc.toJson(QJsonDocument::Indented);
    setConfigText(text);
    
    // Update tree widget
    if (d->treeWidget) {
        updateTreeFromText();
    }
    
    setModified(false);
}

QJsonObject ConfigEditor::getConfig() const
{
    if (d->currentMode == TreeMode) {
        return treeToJson();
    } else {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(d->configText.toUtf8(), &error);
        
        if (error.error == QJsonParseError::NoError) {
            return doc.object();
        }
    }
    
    return d->currentConfig;
}

void ConfigEditor::setConfigText(const QString& text)
{
    d->configText = text;
    
    if (d->textEditor) {
        d->textEditor->setPlainText(text);
    }
    
    // Update tree if in split mode
    if (d->currentMode == SplitMode && d->treeWidget) {
        updateTreeFromText();
    }
    
    emit configChanged();
    
    // Start validation timer if auto-validation is enabled
    if (d->autoValidationEnabled) {
        d->validationTimer->start();
    }
}

QString ConfigEditor::getConfigText() const
{
    if (d->textEditor) {
        return d->textEditor->toPlainText();
    }
    return d->configText;
}

void ConfigEditor::setConfigMap(const QVariantMap& config)
{
    QJsonObject jsonObj = QJsonObject::fromVariantMap(config);
    setConfig(jsonObj);
}

QVariantMap ConfigEditor::getConfigMap() const
{
    return getConfig().toVariantMap();
}

void ConfigEditor::clear()
{
    setConfigText("");
    d->currentConfig = QJsonObject();
    setModified(false);
}

bool ConfigEditor::isEmpty() const
{
    return d->configText.trimmed().isEmpty();
}

bool ConfigEditor::isModified() const
{
    return d->isModified;
}

void ConfigEditor::setModified(bool modified)
{
    if (d->isModified != modified) {
        d->isModified = modified;
        emit modifiedChanged(modified);
    }
}

void ConfigEditor::setupUI()
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
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->mainLayout->setSpacing(0);
    
    // Setup toolbar
    setupToolBar();
    
    // Setup based on editor mode
    switch (d->currentMode) {
        case TreeMode:
            setupTreeMode();
            break;
        case TextMode:
            setupTextMode();
            break;
        case SplitMode:
            setupSplitMode();
            break;
        case FormMode:
            setupFormMode();
            break;
    }
    
    // Status bar
    d->statusLabel = new QLabel();
    d->statusLabel->setStyleSheet("QLabel { padding: 2px 6px; border-top: 1px solid palette(mid); }");
    d->mainLayout->addWidget(d->statusLabel);
    
    // Apply editor options
    applyEditorOptions();
}

void ConfigEditor::setupTreeMode()
{
    d->treeWidget = new QTreeWidget();
    d->treeWidget->setHeaderLabels(QStringList() << tr("Key") << tr("Value") << tr("Type"));
    d->treeWidget->setAlternatingRowColors(true);
    d->treeWidget->setRootIsDecorated(true);
    d->treeWidget->header()->setStretchLastSection(false);
    d->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    d->treeWidget->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    d->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    
    d->mainLayout->addWidget(d->treeWidget);
    
    // Populate tree with current config
    updateTreeFromText();
}

void ConfigEditor::setupTextMode()
{
    d->textEditor = new QPlainTextEdit();
    d->textEditor->setPlainText(d->configText);
    d->textEditor->setReadOnly(d->readOnly);
    d->textEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
    
    d->mainLayout->addWidget(d->textEditor);
    
    // Setup syntax highlighting
    if (d->syntaxHighlightingEnabled) {
        setupSyntaxHighlighter();
    }
}

void ConfigEditor::setupSplitMode()
{
    d->splitter = new QSplitter(Qt::Horizontal);
    
    // Tree widget on the left
    d->treeWidget = new QTreeWidget();
    d->treeWidget->setHeaderLabels(QStringList() << tr("Key") << tr("Value"));
    d->treeWidget->setMaximumWidth(300);
    
    // Text editor on the right
    d->textEditor = new QPlainTextEdit();
    d->textEditor->setPlainText(d->configText);
    d->textEditor->setReadOnly(d->readOnly);
    
    d->splitter->addWidget(d->treeWidget);
    d->splitter->addWidget(d->textEditor);
    d->splitter->setStretchFactor(0, 0);
    d->splitter->setStretchFactor(1, 1);
    
    d->mainLayout->addWidget(d->splitter);
    
    // Setup syntax highlighting
    if (d->syntaxHighlightingEnabled) {
        setupSyntaxHighlighter();
    }
    
    // Populate tree
    updateTreeFromText();
}

void ConfigEditor::setupFormMode()
{
    // Form mode with tabbed interface
    d->tabWidget = new QTabWidget();
    
    // General tab
    QWidget* generalTab = new QWidget();
    QVBoxLayout* generalLayout = new QVBoxLayout(generalTab);
    
    // Add form controls based on config structure
    // This is a simplified implementation
    QLabel* infoLabel = new QLabel(tr("Form mode is not fully implemented yet.\nPlease use text or tree mode."));
    infoLabel->setAlignment(Qt::AlignCenter);
    generalLayout->addWidget(infoLabel);
    
    d->tabWidget->addTab(generalTab, tr("General"));
    d->mainLayout->addWidget(d->tabWidget);
}

void ConfigEditor::setupToolBar()
{
    d->toolBar = new QToolBar();
    d->toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    // File operations
    QAction* openAction = d->toolBar->addAction(QIcon(":/icons/open.png"), tr("Open"));
    QAction* saveAction = d->toolBar->addAction(QIcon(":/icons/save.png"), tr("Save"));
    QAction* saveAsAction = d->toolBar->addAction(QIcon(":/icons/saveas.png"), tr("Save As"));
    
    d->toolBar->addSeparator();
    
    // Edit operations
    QAction* undoAction = d->toolBar->addAction(QIcon(":/icons/undo.png"), tr("Undo"));
    QAction* redoAction = d->toolBar->addAction(QIcon(":/icons/redo.png"), tr("Redo"));
    
    d->toolBar->addSeparator();
    
    // Format operations
    QAction* formatAction = d->toolBar->addAction(QIcon(":/icons/format.png"), tr("Format"));
    QAction* validateAction = d->toolBar->addAction(QIcon(":/icons/validate.png"), tr("Validate"));
    
    d->toolBar->addSeparator();
    
    // Mode selector
    QComboBox* modeCombo = new QComboBox();
    modeCombo->addItems(QStringList() << tr("Tree") << tr("Text") << tr("Split") << tr("Form"));
    modeCombo->setCurrentIndex(static_cast<int>(d->currentMode));
    d->toolBar->addWidget(modeCombo);
    
    // Format selector
    QComboBox* formatCombo = new QComboBox();
    formatCombo->addItems(QStringList() << tr("JSON") << tr("INI") << tr("XML") << tr("YAML") << tr("TOML"));
    formatCombo->setCurrentIndex(static_cast<int>(d->currentFormat));
    d->toolBar->addWidget(formatCombo);
    
    d->mainLayout->addWidget(d->toolBar);
    
    // Connect toolbar actions
    connect(openAction, &QAction::triggered, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Config File"));
        if (!fileName.isEmpty()) {
            loadFromFile(fileName);
        }
    });
    
    connect(saveAction, &QAction::triggered, this, &ConfigEditor::save);
    connect(saveAsAction, &QAction::triggered, this, &ConfigEditor::saveAs);
    connect(undoAction, &QAction::triggered, this, &ConfigEditor::undo);
    connect(redoAction, &QAction::triggered, this, &ConfigEditor::redo);
    connect(formatAction, &QAction::triggered, this, &ConfigEditor::formatConfig);
    connect(validateAction, &QAction::triggered, [this]() { validate(); });
    
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        setEditorMode(static_cast<EditorMode>(index));
    });
    
    connect(formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        setConfigFormat(static_cast<ConfigFormat>(index));
    });
}

void ConfigEditor::connectSignals()
{
    // Validation timer
    connect(d->validationTimer, &QTimer::timeout, this, &ConfigEditor::onValidationTimer);
    
    // Text editor signals
    if (d->textEditor) {
        connect(d->textEditor, &QPlainTextEdit::textChanged, this, &ConfigEditor::onTextChanged);
        connect(d->textEditor, &QPlainTextEdit::cursorPositionChanged, this, &ConfigEditor::onCursorPositionChanged);
    }
    
    // Tree widget signals
    if (d->treeWidget) {
        connect(d->treeWidget, &QTreeWidget::itemChanged, this, &ConfigEditor::onTreeItemChanged);
    }
}

void ConfigEditor::onTextChanged()
{
    d->configText = getConfigText();
    setModified(true);
    emit configChanged();
    
    // Update tree in split mode
    if (d->currentMode == SplitMode) {
        updateTreeFromText();
    }
    
    // Start validation timer
    if (d->autoValidationEnabled) {
        d->validationTimer->start();
    }
}

void ConfigEditor::onTreeItemChanged(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(item)
    Q_UNUSED(column)
    
    setModified(true);
    
    // Update text in split mode
    if (d->currentMode == SplitMode) {
        updateTextFromTree();
    }
    
    emit configChanged();
}

void ConfigEditor::onCursorPositionChanged()
{
    if (d->textEditor) {
        QTextCursor cursor = d->textEditor->textCursor();
        int line = cursor.blockNumber() + 1;
        int column = cursor.columnNumber() + 1;
        
        d->statusLabel->setText(tr("Line: %1, Column: %2").arg(line).arg(column));
        emit cursorPositionChanged(line, column);
    }
}

void ConfigEditor::onValidationTimer()
{
    validate();
}

// Helper methods
QString ConfigEditor::formatToString(ConfigFormat format) const
{
    switch (format) {
        case JsonFormat: return "json";
        case IniFormat: return "ini";
        case XmlFormat: return "xml";
        case YamlFormat: return "yaml";
        case TomlFormat: return "toml";
        default: return "json";
    }
}

ConfigEditor::ConfigFormat ConfigEditor::stringToFormat(const QString& str) const
{
    if (str == "ini") return IniFormat;
    if (str == "xml") return XmlFormat;
    if (str == "yaml") return YamlFormat;
    if (str == "toml") return TomlFormat;
    return JsonFormat;
}

void ConfigEditor::updateTreeFromText()
{
    if (!d->treeWidget) return;
    
    d->treeWidget->clear();
    
    if (d->currentFormat == JsonFormat) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(d->configText.toUtf8(), &error);
        
        if (error.error == QJsonParseError::NoError) {
            populateTree(doc.object());
        }
    }
}

void ConfigEditor::updateTextFromTree()
{
    if (!d->treeWidget || !d->textEditor) return;
    
    if (d->currentFormat == JsonFormat) {
        QJsonObject obj = treeToJson();
        QJsonDocument doc(obj);
        QString text = doc.toJson(QJsonDocument::Indented);
        
        // Update text editor without triggering signals
        d->textEditor->blockSignals(true);
        d->textEditor->setPlainText(text);
        d->textEditor->blockSignals(false);
        
        d->configText = text;
    }
}

void ConfigEditor::populateTree(const QJsonObject& obj, QTreeWidgetItem* parent)
{
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        QTreeWidgetItem* item = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(d->treeWidget);
        item->setText(0, it.key());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        
        QJsonValue value = it.value();
        if (value.isObject()) {
            item->setText(1, tr("[Object]"));
            item->setText(2, tr("Object"));
            populateTree(value.toObject(), item);
        } else if (value.isArray()) {
            item->setText(1, tr("[Array]"));
            item->setText(2, tr("Array"));
            // Handle arrays (simplified)
        } else {
            item->setText(1, value.toVariant().toString());
            item->setText(2, value.toVariant().typeName());
        }
    }
}

QJsonObject ConfigEditor::treeToJson(QTreeWidgetItem* item) const
{
    QJsonObject obj;
    
    QTreeWidgetItem* root = item ? item : d->treeWidget->invisibleRootItem();
    
    for (int i = 0; i < root->childCount(); ++i) {
        QTreeWidgetItem* child = root->child(i);
        QString key = child->text(0);
        
        if (child->childCount() > 0) {
            // Has children - it's an object
            obj[key] = treeToJson(child);
        } else {
            // Leaf node - convert value
            QString valueStr = child->text(1);
            QString type = child->text(2);
            
            if (type == "bool") {
                obj[key] = (valueStr.toLower() == "true");
            } else if (type == "int") {
                obj[key] = valueStr.toInt();
            } else if (type == "double") {
                obj[key] = valueStr.toDouble();
            } else {
                obj[key] = valueStr;
            }
        }
    }
    
    return obj;
}

void ConfigEditor::setupSyntaxHighlighter()
{
    if (!d->textEditor) return;
    
    // Simple JSON syntax highlighter (placeholder)
    // In a real implementation, you would create a proper syntax highlighter
    // based on the current format
}

void ConfigEditor::applyEditorOptions()
{
    if (d->textEditor) {
        QFont font(d->options.fontFamily, d->options.fontSize);
        d->textEditor->setFont(font);
        
        d->textEditor->setLineWrapMode(d->options.wordWrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
        d->textEditor->setTabStopDistance(d->options.tabSize * QFontMetrics(font).horizontalAdvance(' '));
    }
}

// File operations
bool ConfigEditor::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred(tr("Cannot open file: %1").arg(file.errorString()));
        return false;
    }
    
    QString content = file.readAll();
    setConfigText(content);
    setCurrentFilePath(filePath);
    setModified(false);
    
    emit fileLoaded(true, filePath);
    return true;
}

bool ConfigEditor::saveToFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred(tr("Cannot save file: %1").arg(file.errorString()));
        return false;
    }
    
    file.write(getConfigText().toUtf8());
    
    emit fileSaved(true, filePath);
    return true;
}

QString ConfigEditor::currentFilePath() const
{
    return d->currentFilePath;
}

void ConfigEditor::setCurrentFilePath(const QString& filePath)
{
    d->currentFilePath = filePath;
}

// Validation
ConfigEditor::ValidationStatus ConfigEditor::validate()
{
    d->validationErrors.clear();
    d->validationWarnings.clear();
    
    ValidationStatus status = Valid;
    
    if (d->currentFormat == JsonFormat) {
        QJsonParseError error;
        QJsonDocument::fromJson(d->configText.toUtf8(), &error);
        
        if (error.error != QJsonParseError::NoError) {
            d->validationErrors << tr("JSON Parse Error: %1 at offset %2")
                                      .arg(error.errorString())
                                      .arg(error.offset);
            status = Invalid;
        }
    }
    
    // Use custom validator if available
    if (d->validator && status == Valid) {
        // Custom validation logic would go here
    }
    
    d->currentValidationStatus = status;
    emit validationCompleted(status, d->validationErrors, d->validationWarnings);
    
    return status;
}

void ConfigEditor::setValidator(ConfigValidator* validator)
{
    d->validator = validator;
}

ConfigValidator* ConfigEditor::validator() const
{
    return d->validator;
}

QStringList ConfigEditor::validationErrors() const
{
    return d->validationErrors;
}

QStringList ConfigEditor::validationWarnings() const
{
    return d->validationWarnings;
}

// Edit operations
void ConfigEditor::undo()
{
    if (d->textEditor) {
        d->textEditor->undo();
    }
}

void ConfigEditor::redo()
{
    if (d->textEditor) {
        d->textEditor->redo();
    }
}

bool ConfigEditor::canUndo() const
{
    return d->textEditor ? d->textEditor->document()->isUndoAvailable() : false;
}

bool ConfigEditor::canRedo() const
{
    return d->textEditor ? d->textEditor->document()->isRedoAvailable() : false;
}

void ConfigEditor::formatConfig()
{
    if (d->currentFormat == JsonFormat) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(d->configText.toUtf8(), &error);
        
        if (error.error == QJsonParseError::NoError) {
            QString formatted = doc.toJson(QJsonDocument::Indented);
            setConfigText(formatted);
        }
    }
}

// Slots
void ConfigEditor::save()
{
    if (d->currentFilePath.isEmpty()) {
        saveAs();
    } else {
        saveToFile(d->currentFilePath);
        setModified(false);
    }
}

void ConfigEditor::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Config File"));
    if (!fileName.isEmpty()) {
        if (saveToFile(fileName)) {
            setCurrentFilePath(fileName);
            setModified(false);
        }
    }
}