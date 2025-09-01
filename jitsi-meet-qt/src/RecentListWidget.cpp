#include "RecentListWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QCheckBox>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QDebug>

RecentListWidget::RecentListWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_listWidget(nullptr)
    , m_searchBox(nullptr)
    , m_clearSearchButton(nullptr)
    , m_emptyLabel(nullptr)
    , m_contextMenu(nullptr)
    , m_sortMode(SortByDate)
    , m_showFavoritesOnly(false)
    , m_showSearchBox(true)
    , m_maxDisplayItems(50)
{
    setupUI();
    setupConnections();
    createContextMenu();
}

RecentListWidget::~RecentListWidget()
{
}

void RecentListWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(5);

    // Search box
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText(tr("Search recent meetings..."));
    m_clearSearchButton = new QPushButton(tr("Clear"), this);
    m_clearSearchButton->setMaximumWidth(60);

    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->addWidget(m_searchBox);
    searchLayout->addWidget(m_clearSearchButton);
    m_mainLayout->addLayout(searchLayout);

    // List widget
    m_listWidget = new QListWidget(this);
    m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_mainLayout->addWidget(m_listWidget);

    // Empty state label
    m_emptyLabel = new QLabel(tr("No recent meetings"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: gray; font-size: 14px; padding: 20px;");
    m_emptyLabel->hide();
    m_mainLayout->addWidget(m_emptyLabel);

    updateList();
}

void RecentListWidget::setupConnections()
{
    connect(m_listWidget, &QListWidget::itemClicked, this, &RecentListWidget::onItemClicked);
    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &RecentListWidget::onItemDoubleClicked);
    connect(m_listWidget, &QListWidget::customContextMenuRequested, this, &RecentListWidget::onContextMenuRequested);
    
    connect(m_searchBox, &QLineEdit::textChanged, this, &RecentListWidget::onSearchTextChanged);
    connect(m_clearSearchButton, &QPushButton::clicked, this, &RecentListWidget::onClearSearchClicked);

    // Connect to OptimizedRecentManager signals
    if (OptimizedRecentManager::instance()) {
        connect(OptimizedRecentManager::instance(), &OptimizedRecentManager::recentMeetingsChanged,
                this, &RecentListWidget::onRecentMeetingsChanged);
        connect(OptimizedRecentManager::instance(), &OptimizedRecentManager::recentMeetingAdded,
                this, &RecentListWidget::onRecentMeetingAdded);
        connect(OptimizedRecentManager::instance(), &OptimizedRecentManager::recentMeetingUpdated,
                this, &RecentListWidget::onRecentMeetingUpdated);
        connect(OptimizedRecentManager::instance(), &OptimizedRecentManager::recentMeetingRemoved,
                this, &RecentListWidget::onRecentMeetingRemoved);
    }
}

void RecentListWidget::createContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    QAction* favoriteAction = m_contextMenu->addAction(tr("Toggle Favorite"));
    connect(favoriteAction, &QAction::triggered, this, &RecentListWidget::onFavoriteActionTriggered);
    
    QAction* copyLinkAction = m_contextMenu->addAction(tr("Copy Link"));
    connect(copyLinkAction, &QAction::triggered, this, &RecentListWidget::onCopyLinkActionTriggered);
    
    m_contextMenu->addSeparator();
    
    QAction* deleteAction = m_contextMenu->addAction(tr("Delete"));
    connect(deleteAction, &QAction::triggered, this, &RecentListWidget::onDeleteActionTriggered);
}

void RecentListWidget::refreshList()
{
    updateList();
}

void RecentListWidget::setSortMode(SortMode mode)
{
    if (m_sortMode != mode) {
        m_sortMode = mode;
        sortItems();
    }
}

RecentListWidget::SortMode RecentListWidget::sortMode() const
{
    return m_sortMode;
}

void RecentListWidget::setShowFavoritesOnly(bool show)
{
    if (m_showFavoritesOnly != show) {
        m_showFavoritesOnly = show;
        filterItems();
    }
}

bool RecentListWidget::showFavoritesOnly() const
{
    return m_showFavoritesOnly;
}

void RecentListWidget::setShowSearchBox(bool show)
{
    m_showSearchBox = show;
    m_searchBox->setVisible(show);
    m_clearSearchButton->setVisible(show);
}

bool RecentListWidget::showSearchBox() const
{
    return m_showSearchBox;
}

void RecentListWidget::setMaxDisplayItems(int count)
{
    m_maxDisplayItems = qMax(1, count);
    updateList();
}

int RecentListWidget::maxDisplayItems() const
{
    return m_maxDisplayItems;
}

QString RecentListWidget::selectedMeetingId() const
{
    return m_selectedMeetingId;
}

bool RecentListWidget::selectMeeting(const QString& meetingId)
{
    QListWidgetItem* item = findItemByMeetingId(meetingId);
    if (item) {
        m_listWidget->setCurrentItem(item);
        m_selectedMeetingId = meetingId;
        return true;
    }
    return false;
}

void RecentListWidget::clearSelection()
{
    m_listWidget->clearSelection();
    m_selectedMeetingId.clear();
}

void RecentListWidget::onItemClicked(QListWidgetItem* item)
{
    if (item) {
        QString meetingId = getMeetingIdFromItem(item);
        if (!meetingId.isEmpty()) {
            m_selectedMeetingId = meetingId;
            emit meetingSelected(meetingId);
        }
    }
}

void RecentListWidget::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item) {
        QString meetingId = getMeetingIdFromItem(item);
        if (!meetingId.isEmpty()) {
            emit meetingDoubleClicked(meetingId);
        }
    }
}

void RecentListWidget::onSearchTextChanged(const QString& text)
{
    m_searchText = text;
    filterItems();
}

void RecentListWidget::onClearSearchClicked()
{
    m_searchBox->clear();
}

void RecentListWidget::onSortModeChanged(int index)
{
    setSortMode(static_cast<SortMode>(index));
}

void RecentListWidget::onShowFavoritesToggled(bool checked)
{
    setShowFavoritesOnly(checked);
}

void RecentListWidget::onRecentMeetingsChanged()
{
    updateList();
}

void RecentListWidget::onRecentMeetingAdded(const QString& meetingId)
{
    Q_UNUSED(meetingId)
    updateList();
}

void RecentListWidget::onRecentMeetingUpdated(const QString& meetingId)
{
    Q_UNUSED(meetingId)
    updateList();
}

void RecentListWidget::onRecentMeetingRemoved(const QString& meetingId)
{
    Q_UNUSED(meetingId)
    updateList();
}

void RecentListWidget::onContextMenuRequested(const QPoint& pos)
{
    QListWidgetItem* item = m_listWidget->itemAt(pos);
    if (item && m_contextMenu) {
        m_contextMenu->exec(m_listWidget->mapToGlobal(pos));
    }
}

void RecentListWidget::onDeleteActionTriggered()
{
    QListWidgetItem* currentItem = m_listWidget->currentItem();
    if (currentItem) {
        QString meetingId = getMeetingIdFromItem(currentItem);
        if (!meetingId.isEmpty()) {
            int ret = QMessageBox::question(this, tr("Delete Meeting"), 
                                          tr("Are you sure you want to delete this meeting from recent list?"),
                                          QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                emit meetingDeleted(meetingId);
            }
        }
    }
}

void RecentListWidget::onFavoriteActionTriggered()
{
    QListWidgetItem* currentItem = m_listWidget->currentItem();
    if (currentItem) {
        QString meetingId = getMeetingIdFromItem(currentItem);
        if (!meetingId.isEmpty() && OptimizedRecentManager::instance()) {
            auto meeting = OptimizedRecentManager::instance()->getMeetingDetails(meetingId);
            bool newFavoriteState = !meeting.favorite;
            emit meetingFavoriteChanged(meetingId, newFavoriteState);
        }
    }
}

void RecentListWidget::onCopyLinkActionTriggered()
{
    QListWidgetItem* currentItem = m_listWidget->currentItem();
    if (currentItem) {
        QString meetingId = getMeetingIdFromItem(currentItem);
        if (!meetingId.isEmpty() && OptimizedRecentManager::instance()) {
            auto meeting = OptimizedRecentManager::instance()->getMeetingDetails(meetingId);
            if (!meeting.url.isEmpty()) {
                QApplication::clipboard()->setText(meeting.url);
            }
        }
    }
}

void RecentListWidget::updateList()
{
    m_listWidget->clear();
    
    if (!OptimizedRecentManager::instance()) {
        m_emptyLabel->show();
        m_listWidget->hide();
        return;
    }

    QList<OptimizedRecentManager::RecentItem> meetings;
    if (m_showFavoritesOnly) {
        meetings = OptimizedRecentManager::instance()->getFavoriteMeetings();
    } else {
        meetings = OptimizedRecentManager::instance()->getRecentMeetings(m_maxDisplayItems);
    }

    if (meetings.isEmpty()) {
        m_emptyLabel->show();
        m_listWidget->hide();
        return;
    }

    m_emptyLabel->hide();
    m_listWidget->show();

    for (const auto& meeting : meetings) {
        QListWidgetItem* item = createItemForMeeting(meeting);
        if (item) {
            m_listWidget->addItem(item);
        }
    }

    filterItems();
    sortItems();
}

void RecentListWidget::filterItems()
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        if (item) {
            bool visible = true;
            
            if (!m_searchText.isEmpty()) {
                QString itemText = item->text();
                visible = itemText.contains(m_searchText, Qt::CaseInsensitive);
            }
            
            item->setHidden(!visible);
        }
    }
}

void RecentListWidget::sortItems()
{
    // Qt's QListWidget doesn't have built-in sorting for custom data
    // We would need to implement custom sorting logic here
    // For now, we rely on the OptimizedRecentManager to provide sorted data
}

QListWidgetItem* RecentListWidget::createItemForMeeting(const OptimizedRecentManager::RecentItem& meeting)
{
    QListWidgetItem* item = new QListWidgetItem();
    updateItemForMeeting(item, meeting);
    return item;
}

void RecentListWidget::updateItemForMeeting(QListWidgetItem* item, const OptimizedRecentManager::RecentItem& meeting)
{
    if (!item) return;
    
    QString displayText = meeting.displayName.isEmpty() ? meeting.meetingId : meeting.displayName;
    QString timeText = meeting.lastJoined.toString("yyyy-MM-dd hh:mm");
    
    item->setText(QString("%1\n%2").arg(displayText, timeText));
    item->setData(Qt::UserRole, meeting.meetingId);
    
    applyItemStyle(item, meeting);
}

QString RecentListWidget::getMeetingIdFromItem(QListWidgetItem* item) const
{
    if (item) {
        return item->data(Qt::UserRole).toString();
    }
    return QString();
}

QListWidgetItem* RecentListWidget::findItemByMeetingId(const QString& meetingId) const
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        if (item && getMeetingIdFromItem(item) == meetingId) {
            return item;
        }
    }
    return nullptr;
}

void RecentListWidget::applyItemStyle(QListWidgetItem* item, const OptimizedRecentManager::RecentItem& meeting)
{
    if (!item) return;
    
    QFont font = item->font();
    if (meeting.favorite) {
        font.setBold(true);
        item->setFont(font);
        item->setIcon(QIcon(":/icons/star.png")); // Assuming we have a star icon
    } else {
        font.setBold(false);
        item->setFont(font);
        item->setIcon(QIcon());
    }
}