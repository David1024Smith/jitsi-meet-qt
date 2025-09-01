#ifndef RECENTLISTWIDGET_H
#define RECENTLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QList>
#include <QMenu>
#include "OptimizedRecentManager.h"

class QVBoxLayout;
class QLabel;
class QPushButton;
class QLineEdit;

/**
 * @brief 最近会议列表组件
 * 
 * 该组件显示最近参加的会议列表，支持搜索、排序和收藏等功能。
 */
class RecentListWidget : public QWidget
{
    Q_OBJECT

public:
    enum SortMode {
        SortByDate,      ///< 按日期排序
        SortByName,      ///< 按名称排序
        SortByFrequency  ///< 按频率排序
    };
    Q_ENUM(SortMode)

    explicit RecentListWidget(QWidget *parent = nullptr);
    ~RecentListWidget();

    /**
     * @brief 刷新列表
     */
    void refreshList();

    /**
     * @brief 设置排序模式
     * @param mode 排序模式
     */
    void setSortMode(SortMode mode);

    /**
     * @brief 获取排序模式
     * @return 排序模式
     */
    SortMode sortMode() const;

    /**
     * @brief 设置是否显示收藏
     * @param show 是否显示
     */
    void setShowFavoritesOnly(bool show);

    /**
     * @brief 是否仅显示收藏
     * @return 是否仅显示收藏
     */
    bool showFavoritesOnly() const;

    /**
     * @brief 设置是否显示搜索框
     * @param show 是否显示
     */
    void setShowSearchBox(bool show);

    /**
     * @brief 是否显示搜索框
     * @return 是否显示搜索框
     */
    bool showSearchBox() const;

    /**
     * @brief 设置最大显示项数
     * @param count 最大数量
     */
    void setMaxDisplayItems(int count);

    /**
     * @brief 获取最大显示项数
     * @return 最大数量
     */
    int maxDisplayItems() const;

    /**
     * @brief 获取选中的会议ID
     * @return 会议ID
     */
    QString selectedMeetingId() const;

    /**
     * @brief 选择会议
     * @param meetingId 会议ID
     * @return 是否成功选择
     */
    bool selectMeeting(const QString& meetingId);

    /**
     * @brief 清除选择
     */
    void clearSelection();

signals:
    /**
     * @brief 会议选中信号
     * @param meetingId 会议ID
     */
    void meetingSelected(const QString& meetingId);

    /**
     * @brief 会议双击信号
     * @param meetingId 会议ID
     */
    void meetingDoubleClicked(const QString& meetingId);

    /**
     * @brief 会议删除信号
     * @param meetingId 会议ID
     */
    void meetingDeleted(const QString& meetingId);

    /**
     * @brief 会议收藏状态变化信号
     * @param meetingId 会议ID
     * @param favorite 是否收藏
     */
    void meetingFavoriteChanged(const QString& meetingId, bool favorite);

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onItemDoubleClicked(QListWidgetItem* item);
    void onSearchTextChanged(const QString& text);
    void onClearSearchClicked();
    void onSortModeChanged(int index);
    void onShowFavoritesToggled(bool checked);
    void onRecentMeetingsChanged();
    void onRecentMeetingAdded(const QString& meetingId);
    void onRecentMeetingUpdated(const QString& meetingId);
    void onRecentMeetingRemoved(const QString& meetingId);
    void onContextMenuRequested(const QPoint& pos);
    void onDeleteActionTriggered();
    void onFavoriteActionTriggered();
    void onCopyLinkActionTriggered();

private:
    void setupUI();
    void setupConnections();
    void createContextMenu();
    void updateList();
    void filterItems();
    void sortItems();
    QListWidgetItem* createItemForMeeting(const OptimizedRecentManager::RecentItem& meeting);
    void updateItemForMeeting(QListWidgetItem* item, const OptimizedRecentManager::RecentItem& meeting);
    QString getMeetingIdFromItem(QListWidgetItem* item) const;
    QListWidgetItem* findItemByMeetingId(const QString& meetingId) const;
    void applyItemStyle(QListWidgetItem* item, const OptimizedRecentManager::RecentItem& meeting);

    QVBoxLayout* m_mainLayout;
    QListWidget* m_listWidget;
    QLineEdit* m_searchBox;
    QPushButton* m_clearSearchButton;
    QLabel* m_emptyLabel;
    QMenu* m_contextMenu;
    
    SortMode m_sortMode;
    bool m_showFavoritesOnly;
    bool m_showSearchBox;
    int m_maxDisplayItems;
    QString m_searchText;
    QString m_selectedMeetingId;
};

#endif // RECENTLISTWIDGET_H