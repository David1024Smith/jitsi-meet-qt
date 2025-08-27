#ifndef RECENTLISTWIDGET_H
#define RECENTLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include "models/RecentItem.h"

class QListWidget;
class QLabel;
class QVBoxLayout;

/**
 * @brief Widget for displaying recent meeting list
 * 
 * This widget displays a list of recent meetings with functionality to:
 * - Show recent meeting items
 * - Handle item clicks
 * - Display empty state when no items
 * - Manage maximum item count
 */
class RecentListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RecentListWidget(QWidget *parent = nullptr);
    ~RecentListWidget();
    
    /**
     * @brief Add a recent item to the list
     */
    void addRecentItem(const RecentItem& item);
    
    /**
     * @brief Remove a recent item from the list
     */
    void removeRecentItem(const QString& url);
    
    /**
     * @brief Clear all recent items
     */
    void clearRecentItems();
    
    /**
     * @brief Set the list of recent items
     */
    void setRecentItems(const QList<RecentItem>& items);
    
    /**
     * @brief Get all recent items
     */
    QList<RecentItem> getRecentItems() const;
    
    /**
     * @brief Set maximum number of items to display
     */
    void setMaxItems(int maxItems);
    
    /**
     * @brief Get maximum number of items
     */
    int maxItems() const;
    
    /**
     * @brief Check if the list is empty
     */
    bool isEmpty() const;

public slots:
    /**
     * @brief Update translation text
     */
    void retranslateUi();

signals:
    /**
     * @brief Emitted when a recent item is clicked
     */
    void itemClicked(const QString& url);
    
    /**
     * @brief Emitted when a recent item is double-clicked
     */
    void itemDoubleClicked(const QString& url);
    
    /**
     * @brief Emitted when the list changes
     */
    void listChanged();

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    void setupUI();
    void updateDisplay();
    void showEmptyState();
    void hideEmptyState();
    QListWidgetItem* createListItem(const RecentItem& item);
    QString formatItemText(const RecentItem& item) const;
    QString formatTimestamp(const QDateTime& timestamp) const;
    void sortItems();
    
    QVBoxLayout* m_layout;
    QListWidget* m_listWidget;
    QLabel* m_emptyLabel;
    
    QList<RecentItem> m_recentItems;
    int m_maxItems;
    
    static const int DEFAULT_MAX_ITEMS = 10;
};

#endif // RECENTLISTWIDGET_H