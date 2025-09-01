#include "InputWidget.h"

#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QCompleter>
#include <QTimer>
#include <QMimeData>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QTextCursor>
#include <QTextDocument>
#include <QStringListModel>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>
//QRegExp已弃用，使用QRegularExpression替代
#include <QRegularExpression>

class InputWidget::Private
{
public:
    Private(InputWidget* parent) : q(parent) {
        initializeDefaults();
    }
    
    InputWidget* q;
    
    // Input controls
    QLineEdit* lineEdit = nullptr;
    QTextEdit* textEdit = nullptr;
    QPlainTextEdit* plainTextEdit = nullptr;
    QWidget* currentInputControl = nullptr;
    
    // Buttons
    QPushButton* sendButton = nullptr;
    QToolButton* emojiButton = nullptr;
    QToolButton* fileButton = nullptr;
    
    // Layout
    QVBoxLayout* mainLayout = nullptr;
    QVBoxLayout* inputLayout = nullptr;
    QHBoxLayout* buttonLayout = nullptr;
    
    // Status widgets
    QLabel* charCounterLabel = nullptr;
    QProgressBar* uploadProgress = nullptr;
    
    // Auto completion
    QCompleter* completer = nullptr;
    QStringListModel* userNamesModel = nullptr;
    QStringListModel* emojisModel = nullptr;
    QStringListModel* commandsModel = nullptr;
    
    // Configuration
    QString text;
    QString placeholderText = "Type a message...";
    bool enabled = true;
    int maxLength = 1000;
    bool multiLine = false;
    InputMode inputMode = SingleLine;
    SendTrigger sendTrigger = EnterKey;
    bool emojiEnabled = true;
    bool fileUploadEnabled = true;
    AutoCompleteType autoCompleteType = All;
    
    // UI state
    bool sendButtonVisible = true;
    bool emojiButtonVisible = true;
    bool fileButtonVisible = true;
    bool charCounterVisible = false;
    
    // Input history
    QStringList inputHistory;
    int historyLimit = 50;
    int currentHistoryIndex = -1;
    
    // Typing indicator
    QTimer* typingTimer = nullptr;
    bool isTyping = false;
    int typingTimeout = 3000; // 3 seconds
    
    // Validation
    std::function<bool(const QString&)> inputValidator;
    
    // Auto complete data
    QStringList userNames;
    QStringList emojis;
    QStringList commands;
    
    // Style
    QString customStyleSheet;
    
    void initializeDefaults() {
        // Initialize emoji list
        emojis << "😀" << "😃" << "😄" << "😁" << "😆" << "😅" << "😂" << "🤣"
               << "😊" << "😇" << "🙂" << "🙃" << "😉" << "😌" << "😍" << "🥰"
               << "😘" << "😗" << "😙" << "😚" << "😋" << "😛" << "😝" << "😜"
               << "🤪" << "🤨" << "🧐" << "🤓" << "😎" << "🤩" << "🥳" << "😏";
        
        // Initialize commands
        commands << "/help" << "/clear" << "/quit" << "/join" << "/leave"
                << "/mute" << "/unmute" << "/kick" << "/ban" << "/unban";
    }
};

InputWidget::InputWidget(QWidget *parent)
    : QWidget(parent)
    , d(std::make_unique<Private>(this))
{
    initializeUI();
    setupAutoCompleter();
    connectSignals();
    applyStyles();
    updateUIState();
    
    setAcceptDrops(true);
    
    // Setup typing timer
    d->typingTimer = new QTimer(this);
    d->typingTimer->setSingleShot(true);
    connect(d->typingTimer, &QTimer::timeout,
            this, &InputWidget::stopTypingIndicator);
}

InputWidget::~InputWidget() = default;

QString InputWidget::text() const
{
    if (d->currentInputControl == d->lineEdit) {
        return d->lineEdit->text();
    } else if (d->currentInputControl == d->textEdit) {
        return d->textEdit->toPlainText();
    } else if (d->currentInputControl == d->plainTextEdit) {
        return d->plainTextEdit->toPlainText();
    }
    return QString();
}

void InputWidget::setText(const QString& text)
{
    if (d->currentInputControl == d->lineEdit) {
        d->lineEdit->setText(text);
    } else if (d->currentInputControl == d->textEdit) {
        d->textEdit->setPlainText(text);
    } else if (d->currentInputControl == d->plainTextEdit) {
        d->plainTextEdit->setPlainText(text);
    }
    
    d->text = text;
    updateCharCounter();
    emit textChanged(text);
}

QString InputWidget::placeholderText() const
{
    return d->placeholderText;
}

void InputWidget::setPlaceholderText(const QString& text)
{
    if (d->placeholderText != text) {
        d->placeholderText = text;
        
        if (d->lineEdit) {
            d->lineEdit->setPlaceholderText(text);
        }
        if (d->textEdit) {
            d->textEdit->setPlaceholderText(text);
        }
        if (d->plainTextEdit) {
            d->plainTextEdit->setPlaceholderText(text);
        }
        
        emit placeholderTextChanged(text);
    }
}

bool InputWidget::isEnabled() const
{
    return d->enabled;
}

void InputWidget::setEnabled(bool enabled)
{
    if (d->enabled != enabled) {
        d->enabled = enabled;
        QWidget::setEnabled(enabled);
        updateButtonStates();
        emit enabledChanged(enabled);
    }
}

int InputWidget::maxLength() const
{
    return d->maxLength;
}

void InputWidget::setMaxLength(int length)
{
    if (d->maxLength != length && length > 0) {
        d->maxLength = length;
        
        if (d->lineEdit) {
            d->lineEdit->setMaxLength(length);
        }
        
        updateCharCounter();
        emit maxLengthChanged(length);
    }
}

bool InputWidget::isMultiLine() const
{
    return d->multiLine;
}

void InputWidget::setMultiLine(bool multiLine)
{
    if (d->multiLine != multiLine) {
        d->multiLine = multiLine;
        
        if (multiLine) {
            setInputMode(MultiLine);
        } else {
            setInputMode(SingleLine);
        }
        
        emit multiLineChanged(multiLine);
    }
}

InputWidget::InputMode InputWidget::inputMode() const
{
    return d->inputMode;
}

void InputWidget::setInputMode(InputMode mode)
{
    if (d->inputMode != mode) {
        d->inputMode = mode;
        d->multiLine = (mode != SingleLine);
        
        createInputControl();
        updateUIState();
    }
}

InputWidget::SendTrigger InputWidget::sendTrigger() const
{
    return d->sendTrigger;
}

void InputWidget::setSendTrigger(SendTrigger trigger)
{
    d->sendTrigger = trigger;
}

bool InputWidget::isEmojiEnabled() const
{
    return d->emojiEnabled;
}

void InputWidget::setEmojiEnabled(bool enabled)
{
    if (d->emojiEnabled != enabled) {
        d->emojiEnabled = enabled;
        updateButtonStates();
        emit emojiEnabledChanged(enabled);
    }
}

bool InputWidget::isFileUploadEnabled() const
{
    return d->fileUploadEnabled;
}

void InputWidget::setFileUploadEnabled(bool enabled)
{
    if (d->fileUploadEnabled != enabled) {
        d->fileUploadEnabled = enabled;
        updateButtonStates();
        emit fileUploadEnabledChanged(enabled);
    }
}

InputWidget::AutoCompleteType InputWidget::autoCompleteType() const
{
    return d->autoCompleteType;
}

void InputWidget::setAutoCompleteType(AutoCompleteType type)
{
    if (d->autoCompleteType != type) {
        d->autoCompleteType = type;
        setupAutoCompleter();
    }
}

bool InputWidget::isSendButtonVisible() const
{
    return d->sendButtonVisible;
}

void InputWidget::setSendButtonVisible(bool visible)
{
    if (d->sendButtonVisible != visible) {
        d->sendButtonVisible = visible;
        if (d->sendButton) {
            d->sendButton->setVisible(visible);
        }
    }
}

bool InputWidget::isEmojiButtonVisible() const
{
    return d->emojiButtonVisible;
}

void InputWidget::setEmojiButtonVisible(bool visible)
{
    if (d->emojiButtonVisible != visible) {
        d->emojiButtonVisible = visible;
        if (d->emojiButton) {
            d->emojiButton->setVisible(visible);
        }
    }
}

bool InputWidget::isFileButtonVisible() const
{
    return d->fileButtonVisible;
}

void InputWidget::setFileButtonVisible(bool visible)
{
    if (d->fileButtonVisible != visible) {
        d->fileButtonVisible = visible;
        if (d->fileButton) {
            d->fileButton->setVisible(visible);
        }
    }
}

bool InputWidget::isCharCounterVisible() const
{
    return d->charCounterVisible;
}

void InputWidget::setCharCounterVisible(bool visible)
{
    if (d->charCounterVisible != visible) {
        d->charCounterVisible = visible;
        if (d->charCounterLabel) {
            d->charCounterLabel->setVisible(visible);
        }
    }
}

int InputWidget::currentCharCount() const
{
    return text().length();
}

int InputWidget::remainingCharCount() const
{
    return qMax(0, d->maxLength - currentCharCount());
}

bool InputWidget::hasText() const
{
    return !text().trimmed().isEmpty();
}

bool InputWidget::isEmpty() const
{
    return text().trimmed().isEmpty();
}

QString InputWidget::plainText() const
{
    return text();
}

QString InputWidget::htmlText() const
{
    if (d->currentInputControl == d->textEdit) {
        return d->textEdit->toHtml();
    }
    return text();
}

void InputWidget::insertText(const QString& text)
{
    if (d->currentInputControl == d->lineEdit) {
        d->lineEdit->insert(text);
    } else if (d->currentInputControl == d->textEdit) {
        d->textEdit->insertPlainText(text);
    } else if (d->currentInputControl == d->plainTextEdit) {
        d->plainTextEdit->insertPlainText(text);
    }
}

void InputWidget::insertEmoji(const QString& emoji)
{
    insertText(emoji);
    emit emojiSelected(emoji);
}

void InputWidget::insertMention(const QString& username)
{
    insertText("@" + username + " ");
    emit mentionTriggered(username);
}

void InputWidget::setAutoCompleteData(AutoCompleteType type, const QStringList& data)
{
    switch (type) {
        case UserNames:
            d->userNames = data;
            break;
        case Emojis:
            d->emojis = data;
            break;
        case Commands:
            d->commands = data;
            break;
        default:
            break;
    }
    
    setupAutoCompleter();
}

QStringList InputWidget::getAutoCompleteData(AutoCompleteType type) const
{
    switch (type) {
        case UserNames:
            return d->userNames;
        case Emojis:
            return d->emojis;
        case Commands:
            return d->commands;
        default:
            return QStringList();
    }
}

void InputWidget::addAutoCompleteItem(AutoCompleteType type, const QString& item)
{
    QStringList* list = nullptr;
    switch (type) {
        case UserNames:
            list = &d->userNames;
            break;
        case Emojis:
            list = &d->emojis;
            break;
        case Commands:
            list = &d->commands;
            break;
        default:
            return;
    }
    
    if (list && !list->contains(item)) {
        list->append(item);
        setupAutoCompleter();
    }
}

void InputWidget::removeAutoCompleteItem(AutoCompleteType type, const QString& item)
{
    QStringList* list = nullptr;
    switch (type) {
        case UserNames:
            list = &d->userNames;
            break;
        case Emojis:
            list = &d->emojis;
            break;
        case Commands:
            list = &d->commands;
            break;
        default:
            return;
    }
    
    if (list && list->removeAll(item) > 0) {
        setupAutoCompleter();
    }
}

void InputWidget::setInputValidator(std::function<bool(const QString&)> validator)
{
    d->inputValidator = validator;
}

bool InputWidget::validateInput() const
{
    if (d->inputValidator) {
        return d->inputValidator(text());
    }
    return true;
}

void InputWidget::setCustomStyleSheet(const QString& styleSheet)
{
    d->customStyleSheet = styleSheet;
    applyStyles();
}

QString InputWidget::customStyleSheet() const
{
    return d->customStyleSheet;
}

QStringList InputWidget::inputHistory() const
{
    return d->inputHistory;
}

void InputWidget::setInputHistory(const QStringList& history)
{
    d->inputHistory = history;
    d->currentHistoryIndex = -1;
}

void InputWidget::addToHistory(const QString& text)
{
    if (text.trimmed().isEmpty()) {
        return;
    }
    
    // Remove if already exists
    d->inputHistory.removeAll(text);
    
    // Add to beginning
    d->inputHistory.prepend(text);
    
    // Limit history size
    while (d->inputHistory.size() > d->historyLimit) {
        d->inputHistory.removeLast();
    }
    
    d->currentHistoryIndex = -1;
}

void InputWidget::clearHistory()
{
    d->inputHistory.clear();
    d->currentHistoryIndex = -1;
}

int InputWidget::historyLimit() const
{
    return d->historyLimit;
}

void InputWidget::setHistoryLimit(int limit)
{
    if (d->historyLimit != limit && limit > 0) {
        d->historyLimit = limit;
        
        // Trim history if needed
        while (d->inputHistory.size() > limit) {
            d->inputHistory.removeLast();
        }
    }
}

// Public slots
void InputWidget::clear()
{
    setText(QString());
}

void InputWidget::sendMessage()
{
    QString messageText = text().trimmed();
    
    if (messageText.isEmpty()) {
        return;
    }
    
    if (!validateInput()) {
        emit validationFailed(tr("Invalid input"));
        return;
    }
    
    // Add to history
    addToHistory(messageText);
    
    // Emit signal
    emit messageSent(messageText);
    
    // Clear input
    clear();
    
    // Stop typing indicator
    stopTypingIndicator();
}

void InputWidget::showEmojiPicker()
{
    // This would typically show an emoji picker dialog
    // For now, just emit the signal
    emit emojiSelected("😊");
}

void InputWidget::showFileDialog()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this, tr("Select Files"), QString(),
        tr("All Files (*.*);;Images (*.png *.jpg *.jpeg *.gif);;Documents (*.pdf *.doc *.docx *.txt)"));
    
    if (!filePaths.isEmpty()) {
        emit filesSelected(filePaths);
    }
}

void InputWidget::setFocus()
{
    if (d->currentInputControl) {
        d->currentInputControl->setFocus();
    }
}

void InputWidget::selectAll()
{
    if (d->currentInputControl == d->lineEdit) {
        d->lineEdit->selectAll();
    } else if (d->currentInputControl == d->textEdit) {
        d->textEdit->selectAll();
    } else if (d->currentInputControl == d->plainTextEdit) {
        d->plainTextEdit->selectAll();
    }
}

void InputWidget::copy()
{
    if (d->currentInputControl == d->lineEdit) {
        d->lineEdit->copy();
    } else if (d->currentInputControl == d->textEdit) {
        d->textEdit->copy();
    } else if (d->currentInputControl == d->plainTextEdit) {
        d->plainTextEdit->copy();
    }
}

void InputWidget::cut()
{
    if (d->currentInputControl == d->lineEdit) {
        d->lineEdit->cut();
    } else if (d->currentInputControl == d->textEdit) {
        d->textEdit->cut();
    } else if (d->currentInputControl == d->plainTextEdit) {
        d->plainTextEdit->cut();
    }
}

void InputWidget::paste()
{
    if (d->currentInputControl == d->lineEdit) {
        d->lineEdit->paste();
    } else if (d->currentInputControl == d->textEdit) {
        d->textEdit->paste();
    } else if (d->currentInputControl == d->plainTextEdit) {
        d->plainTextEdit->paste();
    }
}

void InputWidget::undo()
{
    if (d->currentInputControl == d->lineEdit) {
        d->lineEdit->undo();
    } else if (d->currentInputControl == d->textEdit) {
        d->textEdit->undo();
    } else if (d->currentInputControl == d->plainTextEdit) {
        d->plainTextEdit->undo();
    }
}

void InputWidget::redo()
{
    if (d->currentInputControl == d->textEdit) {
        d->textEdit->redo();
    } else if (d->currentInputControl == d->plainTextEdit) {
        d->plainTextEdit->redo();
    }
}

void InputWidget::previousHistory()
{
    if (d->inputHistory.isEmpty()) {
        return;
    }
    
    if (d->currentHistoryIndex < d->inputHistory.size() - 1) {
        d->currentHistoryIndex++;
        setText(d->inputHistory[d->currentHistoryIndex]);
    }
}

void InputWidget::nextHistory()
{
    if (d->inputHistory.isEmpty()) {
        return;
    }
    
    if (d->currentHistoryIndex > 0) {
        d->currentHistoryIndex--;
        setText(d->inputHistory[d->currentHistoryIndex]);
    } else if (d->currentHistoryIndex == 0) {
        d->currentHistoryIndex = -1;
        clear();
    }
}

void InputWidget::startTypingIndicator()
{
    if (!d->isTyping) {
        d->isTyping = true;
        emit typingStarted();
    }
    
    // Reset timer
    d->typingTimer->start(d->typingTimeout);
}

void InputWidget::stopTypingIndicator()
{
    if (d->isTyping) {
        d->isTyping = false;
        d->typingTimer->stop();
        emit typingStopped();
    }
}

// Private methods
void InputWidget::initializeUI()
{
    d->mainLayout = new QVBoxLayout(this);
    d->mainLayout->setContentsMargins(5, 5, 5, 5);
    d->mainLayout->setSpacing(5);
    
    // Create input area
    QHBoxLayout* inputAreaLayout = new QHBoxLayout();
    
    // Create input control
    createInputControl();
    
    // Create buttons
    createButtons();
    
    // Add to input area
    inputAreaLayout->addWidget(d->currentInputControl, 1);
    inputAreaLayout->addLayout(d->buttonLayout);
    
    d->mainLayout->addLayout(inputAreaLayout);
    
    // Create character counter
    createCharCounter();
}

void InputWidget::createInputControl()
{
    // Remove old control
    if (d->currentInputControl) {
        d->mainLayout->removeWidget(d->currentInputControl);
        d->currentInputControl->deleteLater();
        d->currentInputControl = nullptr;
    }
    
    switch (d->inputMode) {
        case SingleLine:
            d->lineEdit = new QLineEdit(this);
            d->lineEdit->setPlaceholderText(d->placeholderText);
            d->lineEdit->setMaxLength(d->maxLength);
            d->currentInputControl = d->lineEdit;
            break;
            
        case MultiLine:
            d->plainTextEdit = new QPlainTextEdit(this);
            d->plainTextEdit->setPlaceholderText(d->placeholderText);
            d->plainTextEdit->setMaximumHeight(100);
            d->currentInputControl = d->plainTextEdit;
            break;
            
        case RichText:
            d->textEdit = new QTextEdit(this);
            d->textEdit->setPlaceholderText(d->placeholderText);
            d->textEdit->setMaximumHeight(100);
            d->currentInputControl = d->textEdit;
            break;
    }
    
    d->currentInputControl->setEnabled(d->enabled);
}

void InputWidget::createButtons()
{
    d->buttonLayout = new QHBoxLayout();
    
    // Emoji button
    d->emojiButton = new QToolButton(this);
    d->emojiButton->setText("😊");
    d->emojiButton->setToolTip(tr("Insert Emoji"));
    d->emojiButton->setVisible(d->emojiButtonVisible);
    
    // File button
    d->fileButton = new QToolButton(this);
    d->fileButton->setText("📎");
    d->fileButton->setToolTip(tr("Attach File"));
    d->fileButton->setVisible(d->fileButtonVisible);
    
    // Send button
    d->sendButton = new QPushButton(tr("Send"), this);
    d->sendButton->setVisible(d->sendButtonVisible);
    
    d->buttonLayout->addWidget(d->emojiButton);
    d->buttonLayout->addWidget(d->fileButton);
    d->buttonLayout->addWidget(d->sendButton);
}

void InputWidget::createCharCounter()
{
    d->charCounterLabel = new QLabel(this);
    d->charCounterLabel->setVisible(d->charCounterVisible);
    d->charCounterLabel->setAlignment(Qt::AlignRight);
    d->charCounterLabel->setStyleSheet("color: #666666; font-size: 10px;");
    
    d->mainLayout->addWidget(d->charCounterLabel);
    
    updateCharCounter();
}

void InputWidget::setupAutoCompleter()
{
    if (d->completer) {
        delete d->completer;
        d->completer = nullptr;
    }
    
    if (d->autoCompleteType == NoAutoComplete) {
        return;
    }
    
    QStringList completionList;
    
    if (d->autoCompleteType & UserNames) {
        completionList.append(d->userNames);
    }
    if (d->autoCompleteType & Emojis) {
        completionList.append(d->emojis);
    }
    if (d->autoCompleteType & Commands) {
        completionList.append(d->commands);
    }
    
    if (!completionList.isEmpty()) {
        d->completer = new QCompleter(completionList, this);
        d->completer->setCaseSensitivity(Qt::CaseInsensitive);
        d->completer->setFilterMode(Qt::MatchContains);
        
        if (d->lineEdit) {
            d->lineEdit->setCompleter(d->completer);
        }
        
        connect(d->completer, QOverload<const QString&>::of(&QCompleter::activated),
                this, &InputWidget::handleAutoCompleteActivated);
    }
}

void InputWidget::connectSignals()
{
    // Input control signals
    if (d->lineEdit) {
        connect(d->lineEdit, &QLineEdit::textChanged,
                this, &InputWidget::handleTextChanged);
        connect(d->lineEdit, &QLineEdit::returnPressed,
                this, [this]() {
                    if (d->sendTrigger == EnterKey) {
                        sendMessage();
                    }
                });
    }
    
    if (d->textEdit) {
        connect(d->textEdit, &QTextEdit::textChanged,
                this, &InputWidget::handleTextChanged);
    }
    
    if (d->plainTextEdit) {
        connect(d->plainTextEdit, &QPlainTextEdit::textChanged,
                this, &InputWidget::handleTextChanged);
    }
    
    // Button signals
    if (d->sendButton) {
        connect(d->sendButton, &QPushButton::clicked,
                this, &InputWidget::handleSendButtonClicked);
    }
    
    if (d->emojiButton) {
        connect(d->emojiButton, &QToolButton::clicked,
                this, &InputWidget::handleEmojiButtonClicked);
    }
    
    if (d->fileButton) {
        connect(d->fileButton, &QToolButton::clicked,
                this, &InputWidget::handleFileButtonClicked);
    }
}

void InputWidget::applyStyles()
{
    QString styleSheet = d->customStyleSheet;
    
    if (styleSheet.isEmpty()) {
        styleSheet = R"(
            QLineEdit, QTextEdit, QPlainTextEdit {
                border: 1px solid #d0d0d0;
                border-radius: 5px;
                padding: 5px;
                font-size: 12px;
            }
            QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
                border-color: #2196f3;
            }
            QPushButton {
                background-color: #2196f3;
                color: white;
                border: none;
                border-radius: 5px;
                padding: 8px 16px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #1976d2;
            }
            QPushButton:pressed {
                background-color: #1565c0;
            }
            QPushButton:disabled {
                background-color: #cccccc;
                color: #666666;
            }
            QToolButton {
                border: 1px solid #d0d0d0;
                border-radius: 3px;
                padding: 5px;
                background-color: #f9f9f9;
            }
            QToolButton:hover {
                background-color: #e9e9e9;
            }
            QToolButton:pressed {
                background-color: #d9d9d9;
            }
        )";
    }
    
    setStyleSheet(styleSheet);
}

void InputWidget::updateUIState()
{
    updateButtonStates();
    updateCharCounter();
}

void InputWidget::updateCharCounter()
{
    if (d->charCounterLabel && d->charCounterVisible) {
        int current = currentCharCount();
        int remaining = remainingCharCount();
        
        QString counterText = QString("%1/%2").arg(current).arg(d->maxLength);
        d->charCounterLabel->setText(counterText);
        
        // Change color based on remaining characters
        QString color = "#666666";
        if (remaining < 50) {
            color = "#ff9800";
        }
        if (remaining < 10) {
            color = "#f44336";
        }
        
        d->charCounterLabel->setStyleSheet(
            QString("color: %1; font-size: 10px;").arg(color));
        
        emit charCountChanged(current, remaining);
    }
}

void InputWidget::updateButtonStates()
{
    if (d->sendButton) {
        d->sendButton->setEnabled(d->enabled && hasText());
    }
    
    if (d->emojiButton) {
        d->emojiButton->setEnabled(d->enabled && d->emojiEnabled);
    }
    
    if (d->fileButton) {
        d->fileButton->setEnabled(d->enabled && d->fileUploadEnabled);
    }
}

// Event handlers
void InputWidget::keyPressEvent(QKeyEvent* event)
{
    if (handleKeyboardShortcut(event)) {
        return;
    }
    
    // Handle send triggers
    if (shouldSend(event)) {
        sendMessage();
        event->accept();
        return;
    }
    
    // Handle history navigation
    if (event->key() == Qt::Key_Up && event->modifiers() == Qt::ControlModifier) {
        previousHistory();
        event->accept();
        return;
    }
    
    if (event->key() == Qt::Key_Down && event->modifiers() == Qt::ControlModifier) {
        nextHistory();
        event->accept();
        return;
    }
    
    QWidget::keyPressEvent(event);
}

void InputWidget::focusInEvent(QFocusEvent* event)
{
    QWidget::focusInEvent(event);
    emit focusGained();
}

void InputWidget::focusOutEvent(QFocusEvent* event)
{
    QWidget::focusOutEvent(event);
    stopTypingIndicator();
    emit focusLost();
}

void InputWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (d->fileUploadEnabled && event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void InputWidget::dragMoveEvent(QDragMoveEvent* event)
{
    if (d->fileUploadEnabled && event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void InputWidget::dropEvent(QDropEvent* event)
{
    if (d->fileUploadEnabled) {
        handleFileDrop(event->mimeData());
        event->acceptProposedAction();
    }
}

// Private slots
void InputWidget::handleTextChanged()
{
    QString newText = text();
    
    if (d->text != newText) {
        d->text = newText;
        updateCharCounter();
        updateButtonStates();
        
        // Start typing indicator
        if (!newText.isEmpty()) {
            startTypingIndicator();
        } else {
            stopTypingIndicator();
        }
        
        // Detect mentions and commands
        detectMentionsAndCommands(newText);
        
        emit textChanged(newText);
    }
}

void InputWidget::handleSendButtonClicked()
{
    sendMessage();
}

void InputWidget::handleEmojiButtonClicked()
{
    showEmojiPicker();
}

void InputWidget::handleFileButtonClicked()
{
    showFileDialog();
}

void InputWidget::handleTypingTimer()
{
    stopTypingIndicator();
}

void InputWidget::handleAutoCompleteActivated(const QString& text)
{
    // Handle auto-completion selection
    insertText(text);
}

// Helper methods
bool InputWidget::handleKeyboardShortcut(QKeyEvent* event)
{
    if (event->matches(QKeySequence::Copy)) {
        copy();
        return true;
    }
    
    if (event->matches(QKeySequence::Cut)) {
        cut();
        return true;
    }
    
    if (event->matches(QKeySequence::Paste)) {
        paste();
        return true;
    }
    
    if (event->matches(QKeySequence::Undo)) {
        undo();
        return true;
    }
    
    if (event->matches(QKeySequence::Redo)) {
        redo();
        return true;
    }
    
    if (event->matches(QKeySequence::SelectAll)) {
        selectAll();
        return true;
    }
    
    return false;
}

bool InputWidget::shouldSend(QKeyEvent* event) const
{
    switch (d->sendTrigger) {
        case EnterKey:
            return event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter;
        case CtrlEnter:
            return (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
                   event->modifiers() == Qt::ControlModifier;
        case ShiftEnter:
            return (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
                   event->modifiers() == Qt::ShiftModifier;
        case SendButton:
        default:
            return false;
    }
}

void InputWidget::handleFileDrop(const QMimeData* mimeData)
{
    if (!mimeData->hasUrls()) {
        return;
    }
    
    QStringList filePaths;
    for (const QUrl& url : mimeData->urls()) {
        if (url.isLocalFile()) {
            QString filePath = url.toLocalFile();
            if (validateFile(filePath)) {
                filePaths.append(filePath);
            }
        }
    }
    
    if (!filePaths.isEmpty()) {
        emit filesSelected(filePaths);
    }
}

bool InputWidget::validateFile(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isFile();
}

QString InputWidget::formatText(const QString& text) const
{
    // Basic text formatting (could be extended)
    return text;
}

void InputWidget::detectMentionsAndCommands(const QString& text)
{
    // Detect @mentions
    QRegularExpression mentionRegex("@(\\w+)");
    QRegularExpressionMatchIterator matchIterator = mentionRegex.globalMatch(text);
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        QString username = match.captured(1);
        emit mentionTriggered(username);
    }
    
    // Detect /commands
    if (text.startsWith('/')) {
        QStringList words = text.split(' ', Qt::SkipEmptyParts);
        if (!words.isEmpty()) {
            QString command = words.first();
            emit commandTriggered(command);
        }
    }
}