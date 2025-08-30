#ifndef MESSAGELIST_H
#define MESSAGELIST_H

#include <QScrollArea>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <memory>

// Forward declarations
class ChatMessage;
class QVBoxLayout;
class QWidget;
class QLabel;
class QScrollBar;
class QTimer;
class QPropertyAnimation;
class QGraphicsOpacityEffect;

/**
 * @brief 消息列表组件类
 * 
 * MessageList负责显示聊天消息列表，支持消息的
 * 滚动、选择、搜索和各种显示效果。
 */
class MessageList : public QScrollArea
{
    Q_OBJECT
    Q_PROPERTY(int messageCount READ messageCount NOTIFY messageCountChanged)
    Q_PROPERTY(bool autoScroll READ isAutoScrollEnabled WRITE setAutoScrollEnabled NOTIFY autoScrollChanged)
    Q_PROPERTY(bool showTimestamps READ showTimestamps WRITE setShowTimestamps NOTIFY showTimestampsChanged)
    Q_PROPERTY(bool showAvatars READ showAvatars WRITE setShowAvatars NOTIFY showAvatarsChanged)
    Q_PROPERTY(QString dateFormat READ dateFormat WRITE setDateFormat NOTIFY dateFormatChanged)
    Q_PROPERTY(QString timeFormat READ timeFormat WRITE setTimeFormat NOTIFY timeFormatChanged)

public:
    /**
     * @brief 消息分组模式枚举
     */
    enum GroupingMode {
        NoGrouping,         ///< 不分组
        GroupByTime,        ///< 按时间分组
        GroupBySender,      ///< 按发送者分组
        GroupByDay          ///< 按日期分组
    };
    Q_ENUM(GroupingMode)

    /**
     * @brief 滚动行为枚举
     */
    enum ScrollBehavior {
        ScrollToBottom,     ///< 滚动到底部
        ScrollToTop,        ///< 滚动到顶部
        ScrollToMessage,    ///< 滚动到指定消息
        NoScroll            ///< 不滚动
    };
    Q_ENUM(ScrollBehavior)

    /**
     * @brief 选择模式枚举
     */
    enum SelectionMode {
        NoSelection,        ///< 不可选择
        SingleSelection,    ///< 单选
        MultiSelection      ///< 多选
    };
    Q_ENUM(SelectionMode)

    /**
     * @brief 构造函数
     * @param parent 父组件
     */
    explicit MessageList(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MessageList();

    /**
     * @brief 获取消息数量
     * @return 消息数量
     */
    int messageCount() const;

    /**
     * @brief 检查是否启用自动滚动
     * @return 是否启用自动滚动
     */
    bool isAutoScrollEnabled() const;

    /**
     * @brief 设置自动滚动启用状态
     * @param enabled 是否启用
     */
    void setAutoScrollEnabled(bool enabled);

    /**
     * @brief 检查是否显示时间戳
     * @return 是否显示时间戳
     */
    bool showTimestamps() const;

    /**
     * @brief 设置时间戳显示状态
     * @param show 是否显示
     */
    void setShowTimestamps(bool show);

    /**
     * @brief 检查是否显示头像
     * @return 是否显示头像
     */
    bool showAvatars() const;

    /**
     * @brief 设置头像显示状态
     * @param show 是否显示
     */
    void setShowAvatars(bool show);

    /**
     * @brief 获取日期格式
     * @return 日期格式字符串
     */
    QString dateFormat() const;

    /**
     * @brief 设置日期格式
     * @param format 日期格式字符串
     */
    void setDateFormat(const QString& format);

    /**
     * @brief 获取时间格式
     * @return 时间格式字符串
     */
    QString timeFormat() const;

    /**
     * @brief 设置时间格式
     * @param format 时间格式字符串
     */
    void setTimeFormat(const QString& format);

    /**
     * @brief 获取分组模式
     * @return 分组模式
     */
    GroupingMode groupingMode() const;

    /**
     * @brief 设置分组模式
     * @param mode 分组模式
     */
    void setGroupingMode(GroupingMode mode);

    /**
     * @brief 获取选择模式
     * @return 选择模式
     */
    SelectionMode selectionMode() const;

    /**
     * @brief 设置选择模式
     * @param mode 选择模式
     */
    void setSelectionMode(SelectionMode mode);

    /**
     * @brief 获取最大显示消息数量
     * @return 最大消息数量
     */
    int maxDisplayMessages() const;

    /**
     * @brief 设置最大显示消息数量
     * @param maxMessages 最大消息数量
     */
    void setMaxDisplayMessages(int maxMessages);

    /**
     * @brief 添加消息
     * @param message 消息对象
     */
    void addMessage(ChatMessage* message);

    /**
     * @brief 批量添加消息
     * @param messages 消息列表
     */
    void addMessages(const QList<ChatMessage*>& messages);

    /**
     * @brief 插入消息
     * @param index 插入位置
     * @param message 消息对象
     */
    void insertMessage(int index, ChatMessage* message);

    /**
     * @brief 更新消息
     * @param message 消息对象
     */
    void updateMessage(ChatMessage* message);

    /**
     * @brief 移除消息
     * @param messageId 消息ID
     */
    void removeMessage(const QString& messageId);

    /**
     * @brief 获取消息
     * @param messageId 消息ID
     * @return 消息对象，如果不存在则返回nullptr
     */
    ChatMessage* getMessage(const QString& messageId) const;

    /**
     * @brief 获取所有消息
     * @return 消息列表
     */
    QList<ChatMessage*> getAllMessages() const;

    /**
     * @brief 获取选中的消息
     * @return 选中的消息列表
     */
    QList<ChatMessage*> getSelectedMessages() const;

    /**
     * @brief 获取选中的消息ID
     * @return 选中的消息ID列表
     */
    QStringList getSelectedMessageIds() const;

    /**
     * @brief 选择消息
     * @param messageId 消息ID
     * @param selected 是否选中
     */
    void selectMessage(const QString& messageId, bool selected = true);

    /**
     * @brief 选择所有消息
     */
    void selectAllMessages();

    /**
     * @brief 清除选择
     */
    void clearSelection();

    /**
     * @brief 滚动到消息
     * @param messageId 消息ID
     * @param behavior 滚动行为
     */
    void scrollToMessage(const QString& messageId, ScrollBehavior behavior = ScrollToMessage);

    /**
     * @brief 滚动到底部
     */
    void scrollToBottom();

    /**
     * @brief 滚动到顶部
     */
    void scrollToTop();

    /**
     * @brief 高亮显示消息
     * @param messageId 消息ID
     * @param duration 高亮持续时间（毫秒）
     */
    void highlightMessage(const QString& messageId, int duration = 3000);

    /**
     * @brief 搜索消息
     * @param query 搜索关键词
     * @param caseSensitive 是否区分大小写
     * @return 匹配的消息ID列表
     */
    QStringList searchMessages(const QString& query, bool caseSensitive = false);

    /**
     * @brief 高亮搜索结果
     * @param query 搜索关键词
     * @param caseSensitive 是否区分大小写
     */
    void highlightSearchResults(const QString& query, bool caseSensitive = false);

    /**
     * @brief 清除搜索高亮
     */
    void clearSearchHighlight();

    /**
     * @brief 设置消息过滤器
     * @param filter 过滤器函数
     */
    void setMessageFilter(std::function<bool(ChatMessage*)> filter);

    /**
     * @brief 清除消息过滤器
     */
    void clearMessageFilter();

    /**
     * @brief 应用过滤器
     */
    void applyFilter();

    /**
     * @brief 设置自定义样式表
     * @param styleSheet 样式表
     */
    void setCustomStyleSheet(const QString& styleSheet);

    /**
     * @brief 获取自定义样式表
     * @return 样式表
     */
    QString customStyleSheet() const;

    /**
     * @brief 设置消息间距
     * @param spacing 间距像素
     */
    void setMessageSpacing(int spacing);

    /**
     * @brief 获取消息间距
     * @return 间距像素
     */
    int messageSpacing() const;

    /**
     * @brief 设置边距
     * @param left 左边距
     * @param top 上边距
     * @param right 右边距
     * @param bottom 下边距
     */
    void setMargins(int left, int top, int right, int bottom);

    /**
     * @brief 检查是否在底部
     * @return 是否在底部
     */
    bool isAtBottom() const;

    /**
     * @brief 检查是否在顶部
     * @return 是否在顶部
     */
    bool isAtTop() const;

    /**
     * @brief 获取可见消息数量
     * @return 可见消息数量
     */
    int visibleMessageCount() const;

    /**
     * @brief 获取第一个可见消息ID
     * @return 消息ID
     */
    QString firstVisibleMessageId() const;

    /**
     * @brief 获取最后一个可见消息ID
     * @return 消息ID
     */
    QString lastVisibleMessageId() const;

public slots:
    /**
     * @brief 清除所有消息
     */
    void clearMessages();

    /**
     * @brief 刷新显示
     */
    void refreshDisplay();

    /**
     * @brief 重新布局
     */
    void relayout();

    /**
     * @brief 更新滚动条
     */
    void updateScrollBar();

    /**
     * @brief 加载更多消息
     */
    void loadMoreMessages();

    /**
     * @brief 标记消息为已读
     * @param messageId 消息ID
     */
    void markMessageAsRead(const QString& messageId);

    /**
     * @brief 标记所有可见消息为已读
     */
    void markVisibleMessagesAsRead();

    /**
     * @brief 复制选中的消息
     */
    void copySelectedMessages();

    /**
     * @brief 删除选中的消息
     */
    void deleteSelectedMessages();

    /**
     * @brief 导出消息
     * @param filePath 文件路径
     * @param format 导出格式
     */
    void exportMessages(const QString& filePath, const QString& format = "txt");

signals:
    /**
     * @brief 消息数量改变信号
     * @param count 新数量
     */
    void messageCountChanged(int count);

    /**
     * @brief 自动滚动状态改变信号
     * @param enabled 是否启用
     */
    void autoScrollChanged(bool enabled);

    /**
     * @brief 时间戳显示状态改变信号
     * @param show 是否显示
     */
    void showTimestampsChanged(bool show);

    /**
     * @brief 头像显示状态改变信号
     * @param show 是否显示
     */
    void showAvatarsChanged(bool show);

    /**
     * @brief 日期格式改变信号
     * @param format 新格式
     */
    void dateFormatChanged(const QString& format);

    /**
     * @brief 时间格式改变信号
     * @param format 新格式
     */
    void timeFormatChanged(const QString& format);

    /**
     * @brief 消息添加信号
     * @param message 添加的消息
     */
    void messageAdded(ChatMessage* message);

    /**
     * @brief 消息更新信号
     * @param message 更新的消息
     */
    void messageUpdated(ChatMessage* message);

    /**
     * @brief 消息移除信号
     * @param messageId 移除的消息ID
     */
    void messageRemoved(const QString& messageId);

    /**
     * @brief 消息选择改变信号
     * @param selectedIds 选中的消息ID列表
     */
    void selectionChanged(const QStringList& selectedIds);

    /**
     * @brief 消息点击信号
     * @param messageId 点击的消息ID
     */
    void messageClicked(const QString& messageId);

    /**
     * @brief 消息双击信号
     * @param messageId 双击的消息ID
     */
    void messageDoubleClicked(const QString& messageId);

    /**
     * @brief 消息右键点击信号
     * @param messageId 右键点击的消息ID
     * @param position 点击位置
     */
    void messageRightClicked(const QString& messageId, const QPoint& position);

    /**
     * @brief 滚动到底部信号
     */
    void scrolledToBottom();

    /**
     * @brief 滚动到顶部信号
     */
    void scrolledToTop();

    /**
     * @brief 需要加载更多消息信号
     */
    void loadMoreRequested();

    /**
     * @brief 搜索完成信号
     * @param query 搜索关键词
     * @param results 搜索结果
     */
    void searchCompleted(const QString& query, const QStringList& results);

protected:
    /**
     * @brief 重写滚轮事件
     * @param event 事件对象
     */
    void wheelEvent(QWheelEvent* event) override;

    /**
     * @brief 重写大小改变事件
     * @param event 事件对象
     */
    void resizeEvent(QResizeEvent* event) override;

    /**
     * @brief 重写键盘事件
     * @param event 事件对象
     */
    void keyPressEvent(QKeyEvent* event) override;

    /**
     * @brief 重写鼠标按下事件
     * @param event 事件对象
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief 重写上下文菜单事件
     * @param event 事件对象
     */
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    /**
     * @brief 处理滚动条值改变
     * @param value 新值
     */
    void handleScrollBarValueChanged(int value);

    /**
     * @brief 处理滚动条范围改变
     * @param min 最小值
     * @param max 最大值
     */
    void handleScrollBarRangeChanged(int min, int max);

    /**
     * @brief 处理高亮定时器超时
     */
    void handleHighlightTimeout();

    /**
     * @brief 处理动画完成
     */
    void handleAnimationFinished();

private:
    /**
     * @brief 初始化UI
     */
    void initializeUI();

    /**
     * @brief 创建消息组件
     * @param message 消息对象
     * @return 消息组件
     */
    QWidget* createMessageWidget(ChatMessage* message);

    /**
     * @brief 更新消息组件
     * @param widget 消息组件
     * @param message 消息对象
     */
    void updateMessageWidget(QWidget* widget, ChatMessage* message);

    /**
     * @brief 创建日期分隔符
     * @param date 日期
     * @return 分隔符组件
     */
    QWidget* createDateSeparator(const QDate& date);

    /**
     * @brief 应用分组
     */
    void applyGrouping();

    /**
     * @brief 应用样式
     */
    void applyStyles();

    /**
     * @brief 更新布局
     */
    void updateLayout();

    /**
     * @brief 检查是否需要自动滚动
     * @return 是否需要自动滚动
     */
    bool shouldAutoScroll() const;

    /**
     * @brief 执行自动滚动
     */
    void performAutoScroll();

    /**
     * @brief 查找消息组件
     * @param messageId 消息ID
     * @return 消息组件，如果不存在则返回nullptr
     */
    QWidget* findMessageWidget(const QString& messageId) const;

    /**
     * @brief 获取消息组件索引
     * @param messageId 消息ID
     * @return 组件索引，如果不存在则返回-1
     */
    int getMessageWidgetIndex(const QString& messageId) const;

    /**
     * @brief 创建高亮效果
     * @param widget 目标组件
     * @param duration 持续时间
     */
    void createHighlightEffect(QWidget* widget, int duration);

    /**
     * @brief 移除高亮效果
     * @param widget 目标组件
     */
    void removeHighlightEffect(QWidget* widget);

private:
    class Private;
    std::unique_ptr<Private> d;
};

Q_DECLARE_METATYPE(MessageList::GroupingMode)
Q_DECLARE_METATYPE(MessageList::ScrollBehavior)
Q_DECLARE_METATYPE(MessageList::SelectionMode)

#endif // MESSAGELIST_H