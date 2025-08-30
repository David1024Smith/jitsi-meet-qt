#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <memory>

// Forward declarations
class ChatManager;
class ChatMessage;
class Participant;
class MessageList;
class InputWidget;
class QVBoxLayout;
class QHBoxLayout;
class QSplitter;
class QLabel;
class QPushButton;
class QToolButton;
class QMenu;
class QAction;

/**
 * @brief 聊天组件类
 * 
 * ChatWidget是主要的聊天界面组件，集成了消息列表、
 * 输入框和其他聊天相关的UI元素。
 */
class ChatWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentRoom READ currentRoom WRITE setCurrentRoom NOTIFY currentRoomChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(int participantCount READ participantCount NOTIFY participantCountChanged)
    Q_PROPERTY(bool inputEnabled READ isInputEnabled WRITE setInputEnabled NOTIFY inputEnabledChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)

public:
    /**
     * @brief 显示模式枚举
     */
    enum DisplayMode {
        CompactMode,        ///< 紧凑模式
        NormalMode,         ///< 普通模式
        ExpandedMode        ///< 扩展模式
    };
    Q_ENUM(DisplayMode)

    /**
     * @brief 工具栏位置枚举
     */
    enum ToolbarPosition {
        TopToolbar,         ///< 顶部工具栏
        BottomToolbar,      ///< 底部工具栏
        LeftToolbar,        ///< 左侧工具栏
        RightToolbar,       ///< 右侧工具栏
        NoToolbar           ///< 无工具栏
    };
    Q_ENUM(ToolbarPosition)

    /**
     * @brief 构造函数
     * @param parent 父组件
     */
    explicit ChatWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ChatWidget();

    /**
     * @brief 设置聊天管理器
     * @param manager 聊天管理器
     */
    void setChatManager(ChatManager* manager);

    /**
     * @brief 获取聊天管理器
     * @return 聊天管理器
     */
    ChatManager* chatManager() const;

    /**
     * @brief 获取当前房间ID
     * @return 当前房间ID
     */
    QString currentRoom() const;

    /**
     * @brief 设置当前房间
     * @param roomId 房间ID
     */
    void setCurrentRoom(const QString& roomId);

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief 获取参与者数量
     * @return 参与者数量
     */
    int participantCount() const;

    /**
     * @brief 检查输入是否启用
     * @return 是否启用输入
     */
    bool isInputEnabled() const;

    /**
     * @brief 设置输入启用状态
     * @param enabled 是否启用
     */
    void setInputEnabled(bool enabled);

    /**
     * @brief 获取主题名称
     * @return 主题名称
     */
    QString theme() const;

    /**
     * @brief 设置主题
     * @param theme 主题名称
     */
    void setTheme(const QString& theme);

    /**
     * @brief 获取显示模式
     * @return 显示模式
     */
    DisplayMode displayMode() const;

    /**
     * @brief 设置显示模式
     * @param mode 显示模式
     */
    void setDisplayMode(DisplayMode mode);

    /**
     * @brief 获取工具栏位置
     * @return 工具栏位置
     */
    ToolbarPosition toolbarPosition() const;

    /**
     * @brief 设置工具栏位置
     * @param position 工具栏位置
     */
    void setToolbarPosition(ToolbarPosition position);

    /**
     * @brief 获取消息列表组件
     * @return 消息列表组件
     */
    MessageList* messageList() const;

    /**
     * @brief 获取输入组件
     * @return 输入组件
     */
    InputWidget* inputWidget() const;

    /**
     * @brief 检查是否显示参与者列表
     * @return 是否显示参与者列表
     */
    bool isParticipantListVisible() const;

    /**
     * @brief 设置参与者列表可见性
     * @param visible 是否可见
     */
    void setParticipantListVisible(bool visible);

    /**
     * @brief 检查是否显示工具栏
     * @return 是否显示工具栏
     */
    bool isToolbarVisible() const;

    /**
     * @brief 设置工具栏可见性
     * @param visible 是否可见
     */
    void setToolbarVisible(bool visible);

    /**
     * @brief 检查是否显示状态栏
     * @return 是否显示状态栏
     */
    bool isStatusBarVisible() const;

    /**
     * @brief 设置状态栏可见性
     * @param visible 是否可见
     */
    void setStatusBarVisible(bool visible);

    /**
     * @brief 添加自定义工具栏动作
     * @param action 动作对象
     */
    void addToolbarAction(QAction* action);

    /**
     * @brief 移除工具栏动作
     * @param action 动作对象
     */
    void removeToolbarAction(QAction* action);

    /**
     * @brief 获取所有工具栏动作
     * @return 动作列表
     */
    QList<QAction*> toolbarActions() const;

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
     * @brief 保存聊天记录
     * @param filePath 文件路径
     * @return 保存是否成功
     */
    bool saveChatHistory(const QString& filePath);

    /**
     * @brief 加载聊天记录
     * @param filePath 文件路径
     * @return 加载是否成功
     */
    bool loadChatHistory(const QString& filePath);

    /**
     * @brief 清除聊天显示
     */
    void clearChatDisplay();

    /**
     * @brief 滚动到底部
     */
    void scrollToBottom();

    /**
     * @brief 滚动到顶部
     */
    void scrollToTop();

    /**
     * @brief 滚动到指定消息
     * @param messageId 消息ID
     */
    void scrollToMessage(const QString& messageId);

    /**
     * @brief 高亮显示消息
     * @param messageId 消息ID
     */
    void highlightMessage(const QString& messageId);

    /**
     * @brief 搜索消息
     * @param query 搜索关键词
     */
    void searchMessages(const QString& query);

    /**
     * @brief 获取组件配置
     * @return 配置映射
     */
    QVariantMap getConfiguration() const;

    /**
     * @brief 设置组件配置
     * @param config 配置映射
     */
    void setConfiguration(const QVariantMap& config);

public slots:
    /**
     * @brief 连接到聊天服务
     * @param serverUrl 服务器地址
     */
    void connectToChat(const QString& serverUrl = QString());

    /**
     * @brief 断开聊天连接
     */
    void disconnectFromChat();

    /**
     * @brief 加入房间
     * @param roomId 房间ID
     * @param password 房间密码（可选）
     */
    void joinRoom(const QString& roomId, const QString& password = QString());

    /**
     * @brief 离开房间
     */
    void leaveRoom();

    /**
     * @brief 发送消息
     * @param message 消息内容
     */
    void sendMessage(const QString& message);

    /**
     * @brief 发送文件
     * @param filePath 文件路径
     */
    void sendFile(const QString& filePath);

    /**
     * @brief 刷新参与者列表
     */
    void refreshParticipants();

    /**
     * @brief 显示表情符号选择器
     */
    void showEmojiPicker();

    /**
     * @brief 显示文件选择对话框
     */
    void showFileDialog();

    /**
     * @brief 显示设置对话框
     */
    void showSettings();

    /**
     * @brief 切换参与者列表可见性
     */
    void toggleParticipantList();

    /**
     * @brief 切换工具栏可见性
     */
    void toggleToolbar();

    /**
     * @brief 切换全屏模式
     */
    void toggleFullScreen();

    /**
     * @brief 复制选中的消息
     */
    void copySelectedMessages();

    /**
     * @brief 删除选中的消息
     */
    void deleteSelectedMessages();

    /**
     * @brief 标记所有消息为已读
     */
    void markAllAsRead();

    /**
     * @brief 清除未读计数
     */
    void clearUnreadCount();

    /**
     * @brief 重新加载消息
     */
    void reloadMessages();

    /**
     * @brief 应用主题
     * @param themeName 主题名称
     */
    void applyTheme(const QString& themeName);

signals:
    /**
     * @brief 当前房间改变信号
     * @param roomId 新房间ID
     */
    void currentRoomChanged(const QString& roomId);

    /**
     * @brief 连接状态改变信号
     * @param connected 是否已连接
     */
    void connectionChanged(bool connected);

    /**
     * @brief 参与者数量改变信号
     * @param count 新数量
     */
    void participantCountChanged(int count);

    /**
     * @brief 输入启用状态改变信号
     * @param enabled 是否启用
     */
    void inputEnabledChanged(bool enabled);

    /**
     * @brief 主题改变信号
     * @param theme 新主题
     */
    void themeChanged(const QString& theme);

    /**
     * @brief 消息发送信号
     * @param message 消息内容
     */
    void messageSent(const QString& message);

    /**
     * @brief 文件发送信号
     * @param filePath 文件路径
     */
    void fileSent(const QString& filePath);

    /**
     * @brief 房间加入信号
     * @param roomId 房间ID
     */
    void roomJoined(const QString& roomId);

    /**
     * @brief 房间离开信号
     * @param roomId 房间ID
     */
    void roomLeft(const QString& roomId);

    /**
     * @brief 参与者选择信号
     * @param participantId 参与者ID
     */
    void participantSelected(const QString& participantId);

    /**
     * @brief 消息选择信号
     * @param messageId 消息ID
     */
    void messageSelected(const QString& messageId);

    /**
     * @brief 搜索请求信号
     * @param query 搜索关键词
     */
    void searchRequested(const QString& query);

    /**
     * @brief 设置请求信号
     */
    void settingsRequested();

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

protected:
    /**
     * @brief 重写大小改变事件
     * @param event 事件对象
     */
    void resizeEvent(QResizeEvent* event) override;

    /**
     * @brief 重写关闭事件
     * @param event 事件对象
     */
    void closeEvent(QCloseEvent* event) override;

    /**
     * @brief 重写键盘事件
     * @param event 事件对象
     */
    void keyPressEvent(QKeyEvent* event) override;

    /**
     * @brief 重写拖拽进入事件
     * @param event 事件对象
     */
    void dragEnterEvent(QDragEnterEvent* event) override;

    /**
     * @brief 重写拖拽移动事件
     * @param event 事件对象
     */
    void dragMoveEvent(QDragMoveEvent* event) override;

    /**
     * @brief 重写放置事件
     * @param event 事件对象
     */
    void dropEvent(QDropEvent* event) override;

private slots:
    /**
     * @brief 处理消息接收
     * @param message 消息对象
     */
    void handleMessageReceived(ChatMessage* message);

    /**
     * @brief 处理消息发送成功
     * @param messageId 消息ID
     */
    void handleMessageSent(const QString& messageId);

    /**
     * @brief 处理消息发送失败
     * @param messageId 消息ID
     * @param error 错误信息
     */
    void handleMessageSendFailed(const QString& messageId, const QString& error);

    /**
     * @brief 处理参与者加入
     * @param participant 参与者对象
     */
    void handleParticipantJoined(Participant* participant);

    /**
     * @brief 处理参与者离开
     * @param participantId 参与者ID
     */
    void handleParticipantLeft(const QString& participantId);

    /**
     * @brief 处理连接状态改变
     * @param connected 是否已连接
     */
    void handleConnectionChanged(bool connected);

    /**
     * @brief 处理输入文本改变
     * @param text 新文本
     */
    void handleInputTextChanged(const QString& text);

    /**
     * @brief 处理工具栏动作触发
     */
    void handleToolbarActionTriggered();

private:
    /**
     * @brief 初始化UI
     */
    void initializeUI();

    /**
     * @brief 创建工具栏
     */
    void createToolbar();

    /**
     * @brief 创建状态栏
     */
    void createStatusBar();

    /**
     * @brief 创建参与者列表
     */
    void createParticipantList();

    /**
     * @brief 创建上下文菜单
     */
    void createContextMenus();

    /**
     * @brief 连接信号槽
     */
    void connectSignals();

    /**
     * @brief 应用样式
     */
    void applyStyles();

    /**
     * @brief 更新UI状态
     */
    void updateUIState();

    /**
     * @brief 更新工具栏状态
     */
    void updateToolbarState();

    /**
     * @brief 更新状态栏
     */
    void updateStatusBar();

    /**
     * @brief 更新参与者列表
     */
    void updateParticipantList();

    /**
     * @brief 处理文件拖放
     * @param filePaths 文件路径列表
     */
    void handleFileDrop(const QStringList& filePaths);

    /**
     * @brief 验证文件
     * @param filePath 文件路径
     * @return 验证是否通过
     */
    bool validateFile(const QString& filePath);

private:
    class Private;
    std::unique_ptr<Private> d;
};

Q_DECLARE_METATYPE(ChatWidget::DisplayMode)
Q_DECLARE_METATYPE(ChatWidget::ToolbarPosition)

#endif // CHATWIDGET_H