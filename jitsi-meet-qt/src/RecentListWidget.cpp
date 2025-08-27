#include "RecentListWidget.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QListWidgetItem>
#include <QDateTime>
#include <QFont>
#include <QStyle>
#include <algorithm>

RecentListWidget::RecentListWidget(QWidget *parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_listWidget(nullptr)
    , m_emptyLabel(nullptr)
    , m_maxItems(DEFAULT_MAX_ITEMS)
{
    setupUI();
}

RecentListWidget::~RecentListWidget()
{
    // Qt handles cleanup automatically
}

void RecentListWidget::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    
    // Create list widget
    m_listWidget = new QListWidget(this);
    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    
    // Create empty state label
    m_emptyLabel = new QLabel(tr("no_recent_meetings"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: #666; font-style: italic; padding: 20px;");
    
    // Add widgets to layout
    m_layout->addWidget(m_listWidget);
    m_layout->addWidget(m_emptyLabel);
    
    // Connect signals
    connect(m_listWidget, &QListWidget::itemClicked, 
            this, &RecentListWidget::onItemClicked);
    connect(m_listWidget, &QListWidget::itemDoubleClicked, 
            this, &RecentListWidget::onItemDoubleClicked);
    
    // Initially show empty state
    updateDisplay();
}

void RecentListWidget::addRecentItem(const RecentItem& item)
{
    if (!item.isValid()) {
        return;
    }
    
    // Check if item already exists and update it
    auto it = std::find(m_recentItems.begin(), m_recentItems.end(), item);
    if (it != m_recentItems.end()) {
        // Update existing item
        it->updateAccess();
    } else {
        // Add new item
        m_recentItems.append(item);
    }
    
    // Sort items by timestamp (newest first)
    sortItems();
    
    // Limit to max items
    while (m_recentItems.size() > m_maxItems) {
        m_recentItems.removeLast();
    }
    
    updateDisplay();
    emit listChanged();
}

void RecentListWidget::removeRecentItem(const QString& url)
{
    auto it = std::remove_if(m_recentItems.begin(), m_recentItems.end(),
                            [&url](const RecentItem& item) {
                                return item.url == url;
                            });
    
    if (it != m_recentItems.end()) {
        m_recentItems.erase(it, m_recentItems.end());
        updateDisplay();
        emit listChanged();
    }
}

void RecentListWidget::clearRecentItems()
{
    m_recentItems.clear();
    updateDisplay();
    emit listChanged();
}

void RecentListWidget::setRecentItems(const QList<RecentItem>& items)
{
    m_recentItems = items;
    sortItems();
    
    // Limit to max items
    while (m_recentItems.size() > m_maxItems) {
        m_recentItems.removeLast();
    }
    
    updateDisplay();
    emit listChanged();
}

QList<RecentItem> RecentListWidget::getRecentItems() const
{
    return m_recentItems;
}

void RecentListWidget::setMaxItems(int maxItems)
{
    if (maxItems <= 0) {
        return;
    }
    
    m_maxItems = maxItems;
    
    // Remove excess items if necessary
    while (m_recentItems.size() > m_maxItems) {
        m_recentItems.removeLast();
    }
    
    updateDisplay();
}

int RecentListWidget::maxItems() const
{
    return m_maxItems;
}

bool RecentListWidget::isEmpty() const
{
    return m_recentItems.isEmpty();
}

void RecentListWidget::onItemClicked(QListWidgetItem* item)
{
    if (!item) {
        return;
    }
    
    QString url = item->data(Qt::UserRole).toString();
    if (!url.isEmpty()) {
        emit itemClicked(url);
    }
}

void RecentListWidget::onItemDoubleClicked(QListWidgetItem* item)
{
    if (!item) {
        return;
    }
    
    QString url = item->data(Qt::UserRole).toString();
    if (!url.isEmpty()) {
        emit itemDoubleClicked(url);
    }
}

void RecentListWidget::updateDisplay()
{
    m_listWidget->clear();
    
    if (m_recentItems.isEmpty()) {
        showEmptyState();
    } else {
        hideEmptyState();
        
        // Add items to list widget
        for (const RecentItem& item : m_recentItems) {
            QListWidgetItem* listItem = createListItem(item);
            m_listWidget->addItem(listItem);
        }
    }
}

void RecentListWidget::showEmptyState()
{
    m_listWidget->hide();
    m_emptyLabel->show();
}

void RecentListWidget::hideEmptyState()
{
    m_emptyLabel->hide();
    m_listWidget->show();
}

QListWidgetItem* RecentListWidget::createListItem(const RecentItem& item)
{
    QListWidgetItem* listItem = new QListWidgetItem();
    
    // Set the display text
    QString displayText = formatItemText(item);
    listItem->setText(displayText);
    
    // Store the URL in user data for easy retrieval
    listItem->setData(Qt::UserRole, item.url);
    
    // Set tooltip with full URL
    listItem->setToolTip(item.url);
    
    // Set icon (you can customize this later)
    listItem->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    
    return listItem;
}

QString RecentListWidget::formatItemText(const RecentItem& item) const
{
    QString displayName = item.getDisplayText();
    QString timeStr = formatTimestamp(item.timestamp);
    
    return QString("%1\n%2").arg(displayName, timeStr);
}

QString RecentListWidget::formatTimestamp(const QDateTime& timestamp) const
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsAgo = timestamp.secsTo(now);
    
    if (secondsAgo < 60) {
        return tr("Just now");
    } else if (secondsAgo < 3600) {
        int minutes = secondsAgo / 60;
        return tr("%1 minutes ago").arg(minutes);
    } else if (secondsAgo < 86400) {
        int hours = secondsAgo / 3600;
        return tr("%1 hours ago").arg(hours);
    } else if (secondsAgo < 604800) {
        int days = secondsAgo / 86400;
        return tr("%1 days ago").arg(days);
    } else {
        return timestamp.toString("MMM dd, yyyy");
    }
}

void RecentListWidget::sortItems()
{
    std::sort(m_recentItems.begin(), m_recentItems.end());
}

void RecentListWidget::retranslateUi()
{
    if (m_emptyLabel) {
        m_emptyLabel->setText(tr("no_recent_meetings"));
    }
}