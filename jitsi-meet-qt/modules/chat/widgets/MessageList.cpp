#include "MessageList.h"
#include "../models/ChatMessage.h"

#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

class MessageList::Private
{
public:
    Private(MessageList* parent) : q(parent) {
        initializeDefaults();
    }
    
    MessageList* q;
    
    // Core components
    QWidget* contentWidget = nullptr;
    QVBoxLayout* contentLayout = nullptr;
    
    // Messages
    QList<ChatMessage*> messages;
    QMap<QString, QWidget*> messageWidgets;
    QStringList selectedMessageIds;
    
    // Configuration
    bool autoScrollEnabled = true;
    bool showTimestamps = true;
    bool showAvatars = true;
    QString dateFormat = "yyyy-MM-dd";
    QString timeFormat = "hh:mm:ss";
    GroupingMode groupingMode = NoGrouping;
    SelectionMode selectionMode = SingleSelection;
    int maxDisplayMessages = 1000;
    int messageSpacing = 5;
    
    // State
    bool isAtBottomFlag = true;
    QString lastHighlightedMessageId;
    QTimer* highlightTimer = nullptr;
    QPropertyAnimation* highlightAnimation = nullptr;
    
    // Filter
    std::function<bool(ChatMessage*)> messageFilter;
    
    // Style
    QString customStyleSheet;
    
    void initializeDefaults() {
        // Default values already set in member initialization
    }
};

MessageList::MessageList(QWidget *parent)
    : QScrollArea(parent)
    , d(std::make_unique<Private>(this))
{
    initializeUI();
    
    // Setup scroll area
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Connect scroll bar signals
    connect(verticalScrollBar(), &QScrollBar::valueChanged,
            this, &MessageList::handleScrollBarValueChanged);
    connect(verticalScrollBar(), &QScrollBar::rangeChanged,
            this, &MessageList::handleScrollBarRangeChanged);
    
    // Setup highlight timer
    d->highlightTimer = new QTimer(this);
    d->highlightTimer->setSingleShot(true);
    connect(d->highlightTimer, &QTimer::timeout,
            this, &MessageList::handleHighlightTimeout);
}

MessageList::~MessageList() = default;

int MessageList::messageCount() const
{
    return d->messages.count();
}

bool MessageList::isAutoScrollEnabled() const
{
    return d->autoScrollEnabled;
}

void MessageList::setAutoScrollEnabled(bool enabled)
{
    if (d->autoScrollEnabled != enabled) {
        d->autoScrollEnabled = enabled;
        emit autoScrollChanged(enabled);
    }
}

bool MessageList::showTimestamps() const
{
    return d->showTimestamps;
}

void MessageList::setShowTimestamps(bool show)
{
    if (d->showTimestamps != show) {
        d->showTimestamps = show;
        refreshDisplay();
        emit showTimestampsChanged(show);
    }
}

bool MessageList::showAvatars() const
{
    return d->showAvatars;
}

void MessageList::setShowAvatars(bool show)
{
    if (d->showAvatars != show) {
        d->showAvatars = show;
        refreshDisplay();
        emit showAvatarsChanged(show);
    }
}

QString MessageList::dateFormat() const
{
    return d->dateFormat;
}

void MessageList::setDateFormat(const QString& format)
{
    if (d->dateFormat != format) {
        d->dateFormat = format;
        refreshDisplay();
        emit dateFormatChanged(format);
    }
}

QString MessageList::timeFormat() const
{
    return d->timeFormat;
}

void MessageList::setTimeFormat(const QString& format)
{
    if (d->timeFormat != format) {
        d->timeFormat = format;
        refreshDisplay();
        emit timeFormatChanged(format);
    }
}

MessageList::GroupingMode MessageList::groupingMode() const
{
    return d->groupingMode;
}

void MessageList::setGroupingMode(GroupingMode mode)
{
    if (d->groupingMode != mode) {
        d->groupingMode = mode;
        applyGrouping();
    }
}

MessageList::SelectionMode MessageList::selectionMode() const
{
    return d->selectionMode;
}

void MessageList::setSelectionMode(SelectionMode mode)
{
    if (d->selectionMode != mode) {
        d->selectionMode = mode;
        if (mode == NoSelection) {
            clearSelection();
        }
    }
}

int MessageList::maxDisplayMessages() const
{
    return d->maxDisplayMessages;
}

void MessageList::setMaxDisplayMessages(int maxMessages)
{
    if (d->maxDisplayMessages != maxMessages && maxMessages > 0) {
        d->maxDisplayMessages = maxMessages;
        
        // Remove excess messages if needed
        while (d->messages.count() > maxMessages) {
            ChatMessage* message = d->messages.takeFirst();
            QWidget* widget = d->messageWidgets.take(message->id());
            if (widget) {
                d->contentLayout->removeWidget(widget);
                delete widget;
            }
            emit messageRemoved(message->id());
        }
        
        updateLayout();
    }
}

void MessageList::addMessage(ChatMessage* message)
{
    if (!message || d->messageWidgets.contains(message->id())) {
        return;
    }
    
    // Apply filter if set
    if (d->messageFilter && !d->messageFilter(message)) {
        return;
    }
    
    // Check max messages limit
    if (d->messages.count() >= d->maxDisplayMessages) {
        ChatMessage* oldMessage = d->messages.takeFirst();
        QWidget* oldWidget = d->messageWidgets.take(oldMessage->id());
        if (oldWidget) {
            d->contentLayout->removeWidget(oldWidget);
            delete oldWidget;
        }
        emit messageRemoved(oldMessage->id());
    }
    
    d->messages.append(message);
    
    // Create widget for message
    QWidget* messageWidget = createMessageWidget(message);
    d->messageWidgets[message->id()] = messageWidget;
    d->contentLayout->addWidget(messageWidget);
    
    emit messageAdded(message);
    emit messageCountChanged(d->messages.count());
    
    // Auto scroll if enabled and at bottom
    if (d->autoScrollEnabled && shouldAutoScroll()) {
        QTimer::singleShot(0, this, &MessageList::scrollToBottom);
    }
    
    updateLayout();
}

void MessageList::addMessages(const QList<ChatMessage*>& messages)
{
    for (ChatMessage* message : messages) {
        addMessage(message);
    }
}

void MessageList::insertMessage(int index, ChatMessage* message)
{
    if (!message || index < 0 || index > d->messages.count()) {
        return;
    }
    
    if (d->messageWidgets.contains(message->id())) {
        return;
    }
    
    // Apply filter if set
    if (d->messageFilter && !d->messageFilter(message)) {
        return;
    }
    
    d->messages.insert(index, message);
    
    // Create widget for message
    QWidget* messageWidget = createMessageWidget(message);
    d->messageWidgets[message->id()] = messageWidget;
    d->contentLayout->insertWidget(index, messageWidget);
    
    emit messageAdded(message);
    emit messageCountChanged(d->messages.count());
    
    updateLayout();
}

void MessageList::updateMessage(ChatMessage* message)
{
    if (!message) {
        return;
    }
    
    QWidget* widget = d->messageWidgets.value(message->id());
    if (widget) {
        updateMessageWidget(widget, message);
        emit messageUpdated(message);
    }
}

void MessageList::removeMessage(const QString& messageId)
{
    ChatMessage* message = getMessage(messageId);
    if (!message) {
        return;
    }
    
    d->messages.removeAll(message);
    
    QWidget* widget = d->messageWidgets.take(messageId);
    if (widget) {
        d->contentLayout->removeWidget(widget);
        delete widget;
    }
    
    d->selectedMessageIds.removeAll(messageId);
    
    emit messageRemoved(messageId);
    emit messageCountChanged(d->messages.count());
    
    if (!d->selectedMessageIds.isEmpty()) {
        emit selectionChanged(d->selectedMessageIds);
    }
    
    updateLayout();
}

ChatMessage* MessageList::getMessage(const QString& messageId) const
{
    for (ChatMessage* message : d->messages) {
        if (message->id() == messageId) {
            return message;
        }
    }
    return nullptr;
}

QList<ChatMessage*> MessageList::getAllMessages() const
{
    return d->messages;
}

QList<ChatMessage*> MessageList::getSelectedMessages() const
{
    QList<ChatMessage*> selected;
    for (const QString& id : d->selectedMessageIds) {
        ChatMessage* message = getMessage(id);
        if (message) {
            selected.append(message);
        }
    }
    return selected;
}

QStringList MessageList::getSelectedMessageIds() const
{
    return d->selectedMessageIds;
}

void MessageList::selectMessage(const QString& messageId, bool selected)
{
    if (d->selectionMode == NoSelection) {
        return;
    }
    
    bool wasSelected = d->selectedMessageIds.contains(messageId);
    
    if (selected && !wasSelected) {
        if (d->selectionMode == SingleSelection) {
            d->selectedMessageIds.clear();
        }
        d->selectedMessageIds.append(messageId);
    } else if (!selected && wasSelected) {
        d->selectedMessageIds.removeAll(messageId);
    }
    
    // Update widget appearance
    QWidget* widget = d->messageWidgets.value(messageId);
    if (widget) {
        widget->setProperty("selected", selected);
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    }
    
    emit selectionChanged(d->selectedMessageIds);
}

void MessageList::selectAllMessages()
{
    if (d->selectionMode == NoSelection) {
        return;
    }
    
    d->selectedMessageIds.clear();
    for (ChatMessage* message : d->messages) {
        d->selectedMessageIds.append(message->id());
        
        QWidget* widget = d->messageWidgets.value(message->id());
        if (widget) {
            widget->setProperty("selected", true);
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
        }
    }
    
    emit selectionChanged(d->selectedMessageIds);
}

void MessageList::clearSelection()
{
    for (const QString& messageId : d->selectedMessageIds) {
        QWidget* widget = d->messageWidgets.value(messageId);
        if (widget) {
            widget->setProperty("selected", false);
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
        }
    }
    
    d->selectedMessageIds.clear();
    emit selectionChanged(d->selectedMessageIds);
}

void MessageList::scrollToMessage(const QString& messageId, ScrollBehavior behavior)
{
    QWidget* widget = findMessageWidget(messageId);
    if (!widget) {
        return;
    }
    
    switch (behavior) {
        case ScrollToTop:
            ensureWidgetVisible(widget, 0, 0);
            break;
        case ScrollToBottom:
            ensureWidgetVisible(widget, 0, height());
            break;
        case ScrollToMessage:
        default:
            ensureWidgetVisible(widget);
            break;
    }
}

void MessageList::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    d->isAtBottomFlag = true;
    emit scrolledToBottom();
}

void MessageList::scrollToTop()
{
    verticalScrollBar()->setValue(verticalScrollBar()->minimum());
    d->isAtBottomFlag = false;
    emit scrolledToTop();
}

void MessageList::highlightMessage(const QString& messageId, int duration)
{
    QWidget* widget = findMessageWidget(messageId);
    if (!widget) {
        return;
    }
    
    // Remove previous highlight
    if (!d->lastHighlightedMessageId.isEmpty()) {
        removeHighlightEffect(d->messageWidgets.value(d->lastHighlightedMessageId));
    }
    
    // Apply new highlight
    createHighlightEffect(widget, duration);
    d->lastHighlightedMessageId = messageId;
    
    // Scroll to message
    scrollToMessage(messageId);
}

QStringList MessageList::searchMessages(const QString& query, bool caseSensitive)
{
    QStringList results;
    
    Qt::CaseSensitivity sensitivity = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    
    for (ChatMessage* message : d->messages) {
        if (message->content().contains(query, sensitivity) ||
            message->senderName().contains(query, sensitivity)) {
            results.append(message->id());
        }
    }
    
    emit searchCompleted(query, results);
    return results;
}

void MessageList::highlightSearchResults(const QString& query, bool caseSensitive)
{
    QStringList results = searchMessages(query, caseSensitive);
    
    // Highlight all matching messages
    for (const QString& messageId : results) {
        QWidget* widget = d->messageWidgets.value(messageId);
        if (widget) {
            widget->setProperty("searchHighlight", true);
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
        }
    }
}

void MessageList::clearSearchHighlight()
{
    for (QWidget* widget : d->messageWidgets.values()) {
        if (widget->property("searchHighlight").toBool()) {
            widget->setProperty("searchHighlight", false);
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
        }
    }
}

void MessageList::setMessageFilter(std::function<bool(ChatMessage*)> filter)
{
    d->messageFilter = filter;
}

void MessageList::clearMessageFilter()
{
    d->messageFilter = nullptr;
}

void MessageList::applyFilter()
{
    if (!d->messageFilter) {
        return;
    }
    
    // Hide/show messages based on filter
    for (ChatMessage* message : d->messages) {
        QWidget* widget = d->messageWidgets.value(message->id());
        if (widget) {
            bool visible = d->messageFilter(message);
            widget->setVisible(visible);
        }
    }
    
    updateLayout();
}

void MessageList::setCustomStyleSheet(const QString& styleSheet)
{
    d->customStyleSheet = styleSheet;
    applyStyles();
}

QString MessageList::customStyleSheet() const
{
    return d->customStyleSheet;
}

void MessageList::setMessageSpacing(int spacing)
{
    if (d->messageSpacing != spacing && spacing >= 0) {
        d->messageSpacing = spacing;
        if (d->contentLayout) {
            d->contentLayout->setSpacing(spacing);
        }
    }
}

int MessageList::messageSpacing() const
{
    return d->messageSpacing;
}

void MessageList::setMargins(int left, int top, int right, int bottom)
{
    if (d->contentLayout) {
        d->contentLayout->setContentsMargins(left, top, right, bottom);
    }
}

bool MessageList::isAtBottom() const
{
    QScrollBar* scrollBar = verticalScrollBar();
    return scrollBar->value() >= scrollBar->maximum() - 10; // 10px tolerance
}

bool MessageList::isAtTop() const
{
    return verticalScrollBar()->value() <= verticalScrollBar()->minimum();
}

int MessageList::visibleMessageCount() const
{
    int count = 0;
    for (QWidget* widget : d->messageWidgets.values()) {
        if (widget->isVisible()) {
            count++;
        }
    }
    return count;
}

QString MessageList::firstVisibleMessageId() const
{
    QRect viewportRect = viewport()->rect();
    
    for (ChatMessage* message : d->messages) {
        QWidget* widget = d->messageWidgets.value(message->id());
        if (widget && widget->isVisible()) {
            QRect widgetRect = widget->geometry();
            if (viewportRect.intersects(widgetRect)) {
                return message->id();
            }
        }
    }
    
    return QString();
}

QString MessageList::lastVisibleMessageId() const
{
    QRect viewportRect = viewport()->rect();
    QString lastVisible;
    
    for (ChatMessage* message : d->messages) {
        QWidget* widget = d->messageWidgets.value(message->id());
        if (widget && widget->isVisible()) {
            QRect widgetRect = widget->geometry();
            if (viewportRect.intersects(widgetRect)) {
                lastVisible = message->id();
            }
        }
    }
    
    return lastVisible;
}

// Public slots
void MessageList::clearMessages()
{
    for (QWidget* widget : d->messageWidgets.values()) {
        d->contentLayout->removeWidget(widget);
        delete widget;
    }
    
    d->messages.clear();
    d->messageWidgets.clear();
    d->selectedMessageIds.clear();
    
    emit messageCountChanged(0);
    emit selectionChanged(QStringList());
}

void MessageList::refreshDisplay()
{
    // Recreate all message widgets
    for (ChatMessage* message : d->messages) {
        QWidget* oldWidget = d->messageWidgets.value(message->id());
        if (oldWidget) {
            d->contentLayout->removeWidget(oldWidget);
            delete oldWidget;
        }
        
        QWidget* newWidget = createMessageWidget(message);
        d->messageWidgets[message->id()] = newWidget;
        d->contentLayout->addWidget(newWidget);
    }
    
    updateLayout();
}

void MessageList::relayout()
{
    updateLayout();
}

void MessageList::updateScrollBar()
{
    // Force scroll bar update
    QTimer::singleShot(0, this, [this]() {
        if (d->autoScrollEnabled && shouldAutoScroll()) {
            scrollToBottom();
        }
    });
}

// Private methods
void MessageList::initializeUI()
{
    d->contentWidget = new QWidget();
    d->contentLayout = new QVBoxLayout(d->contentWidget);
    d->contentLayout->setContentsMargins(10, 10, 10, 10);
    d->contentLayout->setSpacing(d->messageSpacing);
    d->contentLayout->addStretch();
    
    setWidget(d->contentWidget);
    
    applyStyles();
}

QWidget* MessageList::createMessageWidget(ChatMessage* message)
{
    QWidget* widget = new QWidget();
    widget->setProperty("messageId", message->id());
    
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(5, 5, 5, 5);
    
    // Create header with sender and timestamp
    if (d->showTimestamps || !message->senderName().isEmpty()) {
        QHBoxLayout* headerLayout = new QHBoxLayout();
        
        if (!message->senderName().isEmpty()) {
            QLabel* senderLabel = new QLabel(message->senderName());
            senderLabel->setStyleSheet("font-weight: bold; color: #0066cc;");
            headerLayout->addWidget(senderLabel);
        }
        
        if (d->showTimestamps) {
            QLabel* timestampLabel = new QLabel(
                message->timestamp().toString(d->timeFormat));
            timestampLabel->setStyleSheet("color: #666666; font-size: 10px;");
            headerLayout->addWidget(timestampLabel);
        }
        
        headerLayout->addStretch();
        layout->addLayout(headerLayout);
    }
    
    // Create content
    QLabel* contentLabel = new QLabel(message->content());
    contentLabel->setWordWrap(true);
    contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    layout->addWidget(contentLabel);
    
    // Style the widget
    widget->setStyleSheet(R"(
        QWidget[messageId] {
            background-color: #f9f9f9;
            border: 1px solid #e0e0e0;
            border-radius: 5px;
            margin: 2px;
        }
        QWidget[messageId][selected="true"] {
            background-color: #e3f2fd;
            border-color: #2196f3;
        }
        QWidget[messageId][searchHighlight="true"] {
            background-color: #fff3cd;
            border-color: #ffc107;
        }
    )");
    
    return widget;
}

void MessageList::updateMessageWidget(QWidget* widget, ChatMessage* message)
{
    // Find and update content label
    QList<QLabel*> labels = widget->findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label->text() == message->content() || 
            label->text().contains(message->content())) {
            label->setText(message->content());
            break;
        }
    }
}

QWidget* MessageList::createDateSeparator(const QDate& date)
{
    QWidget* separator = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(separator);
    
    QLabel* dateLabel = new QLabel(date.toString(d->dateFormat));
    dateLabel->setAlignment(Qt::AlignCenter);
    dateLabel->setStyleSheet(R"(
        QLabel {
            background-color: #e0e0e0;
            color: #666666;
            padding: 5px 10px;
            border-radius: 10px;
            font-size: 12px;
        }
    )");
    
    layout->addStretch();
    layout->addWidget(dateLabel);
    layout->addStretch();
    
    return separator;
}

void MessageList::applyGrouping()
{
    // Implementation for message grouping
    // This would reorganize messages based on the grouping mode
    refreshDisplay();
}

void MessageList::applyStyles()
{
    QString styleSheet = d->customStyleSheet;
    
    if (styleSheet.isEmpty()) {
        styleSheet = R"(
            MessageList {
                background-color: #ffffff;
                border: none;
            }
            QScrollBar:vertical {
                background-color: #f0f0f0;
                width: 12px;
                border-radius: 6px;
            }
            QScrollBar::handle:vertical {
                background-color: #c0c0c0;
                border-radius: 6px;
                min-height: 20px;
            }
            QScrollBar::handle:vertical:hover {
                background-color: #a0a0a0;
            }
        )";
    }
    
    setStyleSheet(styleSheet);
}

void MessageList::updateLayout()
{
    if (d->contentWidget) {
        d->contentWidget->updateGeometry();
        d->contentWidget->update();
    }
}

bool MessageList::shouldAutoScroll() const
{
    return d->isAtBottomFlag || isAtBottom();
}

void MessageList::performAutoScroll()
{
    if (d->autoScrollEnabled && shouldAutoScroll()) {
        scrollToBottom();
    }
}

QWidget* MessageList::findMessageWidget(const QString& messageId) const
{
    return d->messageWidgets.value(messageId);
}

int MessageList::getMessageWidgetIndex(const QString& messageId) const
{
    QWidget* widget = findMessageWidget(messageId);
    if (!widget) {
        return -1;
    }
    
    return d->contentLayout->indexOf(widget);
}

void MessageList::createHighlightEffect(QWidget* widget, int duration)
{
    if (!widget) {
        return;
    }
    
    widget->setProperty("highlighted", true);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    
    if (duration > 0) {
        d->highlightTimer->start(duration);
    }
}

void MessageList::removeHighlightEffect(QWidget* widget)
{
    if (!widget) {
        return;
    }
    
    widget->setProperty("highlighted", false);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
}

// Event handlers
void MessageList::wheelEvent(QWheelEvent* event)
{
    QScrollArea::wheelEvent(event);
    d->isAtBottomFlag = isAtBottom();
}

void MessageList::resizeEvent(QResizeEvent* event)
{
    QScrollArea::resizeEvent(event);
    updateScrollBar();
}

void MessageList::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete && !d->selectedMessageIds.isEmpty()) {
        deleteSelectedMessages();
        return;
    }
    
    if (event->matches(QKeySequence::Copy) && !d->selectedMessageIds.isEmpty()) {
        copySelectedMessages();
        return;
    }
    
    if (event->matches(QKeySequence::SelectAll)) {
        selectAllMessages();
        return;
    }
    
    QScrollArea::keyPressEvent(event);
}

void MessageList::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // Handle message selection
        QWidget* clickedWidget = childAt(event->pos());
        if (clickedWidget) {
            QString messageId = clickedWidget->property("messageId").toString();
            if (!messageId.isEmpty()) {
                bool isSelected = d->selectedMessageIds.contains(messageId);
                
                if (event->modifiers() & Qt::ControlModifier) {
                    selectMessage(messageId, !isSelected);
                } else {
                    clearSelection();
                    selectMessage(messageId, true);
                }
                
                emit messageClicked(messageId);
            }
        }
    }
    
    QScrollArea::mousePressEvent(event);
}

void MessageList::contextMenuEvent(QContextMenuEvent* event)
{
    QWidget* widget = childAt(event->pos());
    if (!widget) {
        return;
    }
    
    QString messageId = widget->property("messageId").toString();
    if (messageId.isEmpty()) {
        return;
    }
    
    QMenu contextMenu(this);
    
    QAction* copyAction = contextMenu.addAction(tr("Copy"));
    QAction* deleteAction = contextMenu.addAction(tr("Delete"));
    contextMenu.addSeparator();
    QAction* selectAllAction = contextMenu.addAction(tr("Select All"));
    
    QAction* selectedAction = contextMenu.exec(event->globalPos());
    
    if (selectedAction == copyAction) {
        copySelectedMessages();
    } else if (selectedAction == deleteAction) {
        deleteSelectedMessages();
    } else if (selectedAction == selectAllAction) {
        selectAllMessages();
    }
    
    emit messageRightClicked(messageId, event->pos());
}

// Private slots
void MessageList::handleScrollBarValueChanged(int value)
{
    d->isAtBottomFlag = (value >= verticalScrollBar()->maximum() - 10);
    
    if (d->isAtBottomFlag) {
        emit scrolledToBottom();
    } else if (value <= verticalScrollBar()->minimum()) {
        emit scrolledToTop();
        emit loadMoreRequested();
    }
}

void MessageList::handleScrollBarRangeChanged(int min, int max)
{
    Q_UNUSED(min)
    Q_UNUSED(max)
    
    if (d->autoScrollEnabled && shouldAutoScroll()) {
        QTimer::singleShot(0, this, &MessageList::scrollToBottom);
    }
}

void MessageList::handleHighlightTimeout()
{
    if (!d->lastHighlightedMessageId.isEmpty()) {
        removeHighlightEffect(d->messageWidgets.value(d->lastHighlightedMessageId));
        d->lastHighlightedMessageId.clear();
    }
}

void MessageList::handleAnimationFinished()
{
    // Handle animation completion if needed
}

// Additional public slots
void MessageList::loadMoreMessages()
{
    emit loadMoreRequested();
}

void MessageList::markMessageAsRead(const QString& messageId)
{
    ChatMessage* message = getMessage(messageId);
    if (message) {
        message->setRead(true);
        updateMessage(message);
    }
}

void MessageList::markVisibleMessagesAsRead()
{
    QString firstVisible = firstVisibleMessageId();
    QString lastVisible = lastVisibleMessageId();
    
    bool inRange = false;
    for (ChatMessage* message : d->messages) {
        if (message->id() == firstVisible) {
            inRange = true;
        }
        
        if (inRange && !message->isRead()) {
            message->setRead(true);
            updateMessage(message);
        }
        
        if (message->id() == lastVisible) {
            break;
        }
    }
}

void MessageList::copySelectedMessages()
{
    if (d->selectedMessageIds.isEmpty()) {
        return;
    }
    
    QStringList textLines;
    for (const QString& messageId : d->selectedMessageIds) {
        ChatMessage* message = getMessage(messageId);
        if (message) {
            QString line = QString("[%1] %2: %3")
                .arg(message->timestamp().toString(d->timeFormat))
                .arg(message->senderName())
                .arg(message->content());
            textLines.append(line);
        }
    }
    
    QApplication::clipboard()->setText(textLines.join("\n"));
}

void MessageList::deleteSelectedMessages()
{
    for (const QString& messageId : d->selectedMessageIds) {
        removeMessage(messageId);
    }
}

void MessageList::exportMessages(const QString& filePath, const QString& format)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Export Error"), 
                           tr("Could not open file for writing: %1").arg(filePath));
        return;
    }
    
    QTextStream out(&file);
    
    if (format.toLower() == "html") {
        out << "<html><body>\n";
        for (ChatMessage* message : d->messages) {
            out << QString("<p><strong>%1</strong> [%2]: %3</p>\n")
                   .arg(message->senderName())
                   .arg(message->timestamp().toString(d->timeFormat))
                   .arg(message->content());
        }
        out << "</body></html>\n";
    } else {
        // Default to plain text
        for (ChatMessage* message : d->messages) {
            out << QString("[%1] %2: %3\n")
                   .arg(message->timestamp().toString(d->timeFormat))
                   .arg(message->senderName())
                   .arg(message->content());
        }
    }
}