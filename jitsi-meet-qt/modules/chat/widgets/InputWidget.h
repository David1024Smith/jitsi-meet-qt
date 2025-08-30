#ifndef INPUTWIDGET_H
#define INPUTWIDGET_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <memory>

// Forward declarations
class QTextEdit;
class QPlainTextEdit;
class QLineEdit;
class QPushButton;
class QToolButton;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QProgressBar;
class QCompleter;
class QTimer;
class QMimeData;

/**
 * @brief 输入组件类
 * 
 * InputWidget提供聊天消息输入功能，支持文本输入、
 * 表情符号、文件上传、自动完成等功能。
 */
class InputWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int maxLength READ maxLength WRITE setMaxLength NOTIFY maxLengthChanged)
    Q_PROPERTY(bool multiLine READ isMultiLine WRITE setMultiLine NOTIFY multiLineChanged)
    Q_PROPERTY(bool emojiEnabled READ isEmojiEnabled WRITE setEmojiEnabled NOTIFY emojiEnabledChanged)
    Q_PROPERTY(bool fileUploadEnabled READ isFileUploadEnabled WRITE setFileUploadEnabled NOTIFY fileUploadEnabledChanged)

public:
    /**
     * @brief 输入模式枚举
     */
    enum InputMode {
        SingleLine,         ///< 单行模式
        MultiLine,          ///< 多行模式
        RichText            ///< 富文本模式
    };
    Q_ENUM(InputMode)

    /**
     * @brief 发送触发方式枚举
     */
    enum SendTrigger {
        EnterKey,           ///< 回车键
        CtrlEnter,          ///< Ctrl+回车
        ShiftEnter,         ///< Shift+回车
        SendButton          ///< 发送按钮
    };
    Q_ENUM(SendTrigger)

    /**
     * @brief 自动完成类型枚举
     */
    enum AutoCompleteType {
        NoAutoComplete,     ///< 无自动完成
        UserNames,          ///< 用户名自动完成
        Emojis,             ///< 表情符号自动完成
        Commands,           ///< 命令自动完成
        All                 ///< 所有类型
    };
    Q_ENUM(AutoCompleteType)

    /**
     * @brief 构造函数
     * @param parent 父组件
     */
    explicit InputWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~InputWidget();

    /**
     * @brief 获取输入文本
     * @return 输入文本
     */
    QString text() const;

    /**
     * @brief 设置输入文本
     * @param text 输入文本
     */
    void setText(const QString& text);

    /**
     * @brief 获取占位符文本
     * @return 占位符文本
     */
    QString placeholderText() const;

    /**
     * @brief 设置占位符文本
     * @param text 占位符文本
     */
    void setPlaceholderText(const QString& text);

    /**
     * @brief 检查是否启用
     * @return 是否启用
     */
    bool isEnabled() const;

    /**
     * @brief 设置启用状态
     * @param enabled 是否启用
     */
    void setEnabled(bool enabled);

    /**
     * @brief 获取最大长度
     * @return 最大长度
     */
    int maxLength() const;

    /**
     * @brief 设置最大长度
     * @param length 最大长度
     */
    void setMaxLength(int length);

    /**
     * @brief 检查是否为多行模式
     * @return 是否为多行模式
     */
    bool isMultiLine() const;

    /**
     * @brief 设置多行模式
     * @param multiLine 是否为多行模式
     */
    void setMultiLine(bool multiLine);

    /**
     * @brief 获取输入模式
     * @return 输入模式
     */
    InputMode inputMode() const;

    /**
     * @brief 设置输入模式
     * @param mode 输入模式
     */
    void setInputMode(InputMode mode);

    /**
     * @brief 获取发送触发方式
     * @return 发送触发方式
     */
    SendTrigger sendTrigger() const;

    /**
     * @brief 设置发送触发方式
     * @param trigger 发送触发方式
     */
    void setSendTrigger(SendTrigger trigger);

    /**
     * @brief 检查是否启用表情符号
     * @return 是否启用表情符号
     */
    bool isEmojiEnabled() const;

    /**
     * @brief 设置表情符号启用状态
     * @param enabled 是否启用
     */
    void setEmojiEnabled(bool enabled);

    /**
     * @brief 检查是否启用文件上传
     * @return 是否启用文件上传
     */
    bool isFileUploadEnabled() const;

    /**
     * @brief 设置文件上传启用状态
     * @param enabled 是否启用
     */
    void setFileUploadEnabled(bool enabled);

    /**
     * @brief 获取自动完成类型
     * @return 自动完成类型
     */
    AutoCompleteType autoCompleteType() const;

    /**
     * @brief 设置自动完成类型
     * @param type 自动完成类型
     */
    void setAutoCompleteType(AutoCompleteType type);

    /**
     * @brief 检查是否显示发送按钮
     * @return 是否显示发送按钮
     */
    bool isSendButtonVisible() const;

    /**
     * @brief 设置发送按钮可见性
     * @param visible 是否可见
     */
    void setSendButtonVisible(bool visible);

    /**
     * @brief 检查是否显示表情按钮
     * @return 是否显示表情按钮
     */
    bool isEmojiButtonVisible() const;

    /**
     * @brief 设置表情按钮可见性
     * @param visible 是否可见
     */
    void setEmojiButtonVisible(bool visible);

    /**
     * @brief 检查是否显示文件按钮
     * @return 是否显示文件按钮
     */
    bool isFileButtonVisible() const;

    /**
     * @brief 设置文件按钮可见性
     * @param visible 是否可见
     */
    void setFileButtonVisible(bool visible);

    /**
     * @brief 获取字符计数器可见性
     * @return 是否显示字符计数器
     */
    bool isCharCounterVisible() const;

    /**
     * @brief 设置字符计数器可见性
     * @param visible 是否可见
     */
    void setCharCounterVisible(bool visible);

    /**
     * @brief 获取当前字符数
     * @return 当前字符数
     */
    int currentCharCount() const;

    /**
     * @brief 获取剩余字符数
     * @return 剩余字符数
     */
    int remainingCharCount() const;

    /**
     * @brief 检查是否有文本
     * @return 是否有文本
     */
    bool hasText() const;

    /**
     * @brief 检查文本是否为空
     * @return 文本是否为空
     */
    bool isEmpty() const;

    /**
     * @brief 获取纯文本
     * @return 纯文本（去除格式）
     */
    QString plainText() const;

    /**
     * @brief 获取HTML文本
     * @return HTML格式文本
     */
    QString htmlText() const;

    /**
     * @brief 插入文本
     * @param text 要插入的文本
     */
    void insertText(const QString& text);

    /**
     * @brief 插入表情符号
     * @param emoji 表情符号
     */
    void insertEmoji(const QString& emoji);

    /**
     * @brief 插入提及
     * @param username 用户名
     */
    void insertMention(const QString& username);

    /**
     * @brief 设置自动完成数据
     * @param type 自动完成类型
     * @param data 数据列表
     */
    void setAutoCompleteData(AutoCompleteType type, const QStringList& data);

    /**
     * @brief 获取自动完成数据
     * @param type 自动完成类型
     * @return 数据列表
     */
    QStringList getAutoCompleteData(AutoCompleteType type) const;

    /**
     * @brief 添加自动完成项
     * @param type 自动完成类型
     * @param item 项目
     */
    void addAutoCompleteItem(AutoCompleteType type, const QString& item);

    /**
     * @brief 移除自动完成项
     * @param type 自动完成类型
     * @param item 项目
     */
    void removeAutoCompleteItem(AutoCompleteType type, const QString& item);

    /**
     * @brief 设置输入验证器
     * @param validator 验证器函数
     */
    void setInputValidator(std::function<bool(const QString&)> validator);

    /**
     * @brief 验证当前输入
     * @return 验证是否通过
     */
    bool validateInput() const;

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
     * @brief 获取输入历史
     * @return 历史记录列表
     */
    QStringList inputHistory() const;

    /**
     * @brief 设置输入历史
     * @param history 历史记录列表
     */
    void setInputHistory(const QStringList& history);

    /**
     * @brief 添加到输入历史
     * @param text 文本
     */
    void addToHistory(const QString& text);

    /**
     * @brief 清除输入历史
     */
    void clearHistory();

    /**
     * @brief 获取历史记录限制
     * @return 历史记录数量限制
     */
    int historyLimit() const;

    /**
     * @brief 设置历史记录限制
     * @param limit 历史记录数量限制
     */
    void setHistoryLimit(int limit);

public slots:
    /**
     * @brief 清除输入
     */
    void clear();

    /**
     * @brief 发送消息
     */
    void sendMessage();

    /**
     * @brief 显示表情选择器
     */
    void showEmojiPicker();

    /**
     * @brief 显示文件选择对话框
     */
    void showFileDialog();

    /**
     * @brief 设置焦点
     */
    void setFocus();

    /**
     * @brief 选择全部文本
     */
    void selectAll();

    /**
     * @brief 复制文本
     */
    void copy();

    /**
     * @brief 剪切文本
     */
    void cut();

    /**
     * @brief 粘贴文本
     */
    void paste();

    /**
     * @brief 撤销操作
     */
    void undo();

    /**
     * @brief 重做操作
     */
    void redo();

    /**
     * @brief 上一个历史记录
     */
    void previousHistory();

    /**
     * @brief 下一个历史记录
     */
    void nextHistory();

    /**
     * @brief 开始输入指示
     */
    void startTypingIndicator();

    /**
     * @brief 停止输入指示
     */
    void stopTypingIndicator();

signals:
    /**
     * @brief 文本改变信号
     * @param text 新文本
     */
    void textChanged(const QString& text);

    /**
     * @brief 占位符文本改变信号
     * @param text 新占位符文本
     */
    void placeholderTextChanged(const QString& text);

    /**
     * @brief 启用状态改变信号
     * @param enabled 是否启用
     */
    void enabledChanged(bool enabled);

    /**
     * @brief 最大长度改变信号
     * @param length 新最大长度
     */
    void maxLengthChanged(int length);

    /**
     * @brief 多行模式改变信号
     * @param multiLine 是否为多行模式
     */
    void multiLineChanged(bool multiLine);

    /**
     * @brief 表情启用状态改变信号
     * @param enabled 是否启用
     */
    void emojiEnabledChanged(bool enabled);

    /**
     * @brief 文件上传启用状态改变信号
     * @param enabled 是否启用
     */
    void fileUploadEnabledChanged(bool enabled);

    /**
     * @brief 发送消息信号
     * @param text 消息文本
     */
    void messageSent(const QString& text);

    /**
     * @brief 文件选择信号
     * @param filePaths 文件路径列表
     */
    void filesSelected(const QStringList& filePaths);

    /**
     * @brief 表情选择信号
     * @param emoji 表情符号
     */
    void emojiSelected(const QString& emoji);

    /**
     * @brief 提及信号
     * @param username 用户名
     */
    void mentionTriggered(const QString& username);

    /**
     * @brief 命令信号
     * @param command 命令
     */
    void commandTriggered(const QString& command);

    /**
     * @brief 输入开始信号
     */
    void typingStarted();

    /**
     * @brief 输入停止信号
     */
    void typingStopped();

    /**
     * @brief 焦点获得信号
     */
    void focusGained();

    /**
     * @brief 焦点失去信号
     */
    void focusLost();

    /**
     * @brief 字符数改变信号
     * @param count 当前字符数
     * @param remaining 剩余字符数
     */
    void charCountChanged(int count, int remaining);

    /**
     * @brief 验证失败信号
     * @param reason 失败原因
     */
    void validationFailed(const QString& reason);

protected:
    /**
     * @brief 重写键盘事件
     * @param event 事件对象
     */
    void keyPressEvent(QKeyEvent* event) override;

    /**
     * @brief 重写焦点进入事件
     * @param event 事件对象
     */
    void focusInEvent(QFocusEvent* event) override;

    /**
     * @brief 重写焦点离开事件
     * @param event 事件对象
     */
    void focusOutEvent(QFocusEvent* event) override;

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
     * @brief 处理文本改变
     */
    void handleTextChanged();

    /**
     * @brief 处理发送按钮点击
     */
    void handleSendButtonClicked();

    /**
     * @brief 处理表情按钮点击
     */
    void handleEmojiButtonClicked();

    /**
     * @brief 处理文件按钮点击
     */
    void handleFileButtonClicked();

    /**
     * @brief 处理输入定时器超时
     */
    void handleTypingTimer();

    /**
     * @brief 处理自动完成激活
     * @param text 完成文本
     */
    void handleAutoCompleteActivated(const QString& text);

private:
    /**
     * @brief 初始化UI
     */
    void initializeUI();

    /**
     * @brief 创建输入控件
     */
    void createInputControl();

    /**
     * @brief 创建按钮
     */
    void createButtons();

    /**
     * @brief 创建字符计数器
     */
    void createCharCounter();

    /**
     * @brief 设置自动完成器
     */
    void setupAutoCompleter();

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
     * @brief 更新字符计数器
     */
    void updateCharCounter();

    /**
     * @brief 更新按钮状态
     */
    void updateButtonStates();

    /**
     * @brief 处理键盘快捷键
     * @param event 键盘事件
     * @return 是否处理了事件
     */
    bool handleKeyboardShortcut(QKeyEvent* event);

    /**
     * @brief 检查是否应该发送
     * @param event 键盘事件
     * @return 是否应该发送
     */
    bool shouldSend(QKeyEvent* event) const;

    /**
     * @brief 处理文件拖放
     * @param mimeData MIME数据
     */
    void handleFileDrop(const QMimeData* mimeData);

    /**
     * @brief 验证文件
     * @param filePath 文件路径
     * @return 验证是否通过
     */
    bool validateFile(const QString& filePath) const;

    /**
     * @brief 格式化文本
     * @param text 原始文本
     * @return 格式化后的文本
     */
    QString formatText(const QString& text) const;

    /**
     * @brief 检测提及和命令
     * @param text 文本
     */
    void detectMentionsAndCommands(const QString& text);

private:
    class Private;
    std::unique_ptr<Private> d;
};

Q_DECLARE_METATYPE(InputWidget::InputMode)
Q_DECLARE_METATYPE(InputWidget::SendTrigger)
Q_DECLARE_METATYPE(InputWidget::AutoCompleteType)

#endif // INPUTWIDGET_H