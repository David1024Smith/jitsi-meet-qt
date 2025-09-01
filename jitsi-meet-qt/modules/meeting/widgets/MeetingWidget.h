#ifndef MEETINGWIDGET_H
#define MEETINGWIDGET_H

#include <QWidget>
#include <QString>
#include <QVariantMap>
#include <memory>

// 前向声明
class IMeetingManager;

class MeetingManager;
class Meeting;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QProgressBar;
class QGroupBox;
class QListWidget;
class QTabWidget;

/**
 * @brief 会议组件类
 * 
 * 提供完整的会议管理界面，包括会议信息显示、控制按钮和状态监控
 */
class MeetingWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 显示模式枚举
     */
    enum DisplayMode {
        CompactMode,     ///< 紧凑模式
        NormalMode,      ///< 正常模式
        DetailedMode     ///< 详细模式
    };
    Q_ENUM(DisplayMode)

    /**
     * @brief 构造函数
     * @param parent 父组件
     */
    explicit MeetingWidget(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MeetingWidget();

    /**
     * @brief 设置会议管理器
     * @param manager 会议管理器指针
     */
    void setMeetingManager(MeetingManager* manager);

    /**
     * @brief 获取会议管理器
     * @return 会议管理器指针
     */
    MeetingManager* meetingManager() const;

    /**
     * @brief 设置当前会议
     * @param meeting 会议对象指针
     */
    void setCurrentMeeting(Meeting* meeting);

    /**
     * @brief 获取当前会议
     * @return 会议对象指针
     */
    Meeting* currentMeeting() const;

    /**
     * @brief 设置显示模式
     * @param mode 显示模式
     */
    void setDisplayMode(DisplayMode mode);

    /**
     * @brief 获取显示模式
     * @return 显示模式
     */
    DisplayMode displayMode() const;

    /**
     * @brief 设置是否显示控制按钮
     * @param show 是否显示
     */
    void setShowControls(bool show);

    /**
     * @brief 获取是否显示控制按钮
     * @return 是否显示
     */
    bool showControls() const;

    /**
     * @brief 设置是否显示参与者列表
     * @param show 是否显示
     */
    void setShowParticipants(bool show);

    /**
     * @brief 获取是否显示参与者列表
     * @return 是否显示
     */
    bool showParticipants() const;

    /**
     * @brief 设置是否显示统计信息
     * @param show 是否显示
     */
    void setShowStatistics(bool show);

    /**
     * @brief 获取是否显示统计信息
     * @return 是否显示
     */
    bool showStatistics() const;

    /**
     * @brief 更新会议信息显示
     */
    void updateMeetingInfo();

    /**
     * @brief 更新参与者列表
     */
    void updateParticipantsList();

    /**
     * @brief 更新统计信息
     */
    void updateStatistics();

    /**
     * @brief 更新控制按钮状态
     */
    void updateControlsState();

    /**
     * @brief 设置主题样式
     * @param theme 主题名称
     */
    void setTheme(const QString& theme);

    /**
     * @brief 获取当前主题
     * @return 主题名称
     */
    QString theme() const;

public slots:
    /**
     * @brief 刷新界面
     */
    void refresh();

    /**
     * @brief 重置界面
     */
    void reset();

    /**
     * @brief 显示会议信息
     * @param meetingInfo 会议信息映射
     */
    void showMeetingInfo(const QVariantMap& meetingInfo);

    /**
     * @brief 显示错误信息
     * @param error 错误信息
     */
    void showError(const QString& error);

    /**
     * @brief 显示状态信息
     * @param status 状态信息
     */
    void showStatus(const QString& status);

    /**
     * @brief 开始加载动画
     */
    void startLoading();

    /**
     * @brief 停止加载动画
     */
    void stopLoading();

public slots:
    /**
     * @brief 加入会议
     */
    void joinMeeting();
    
    /**
     * @brief 离开会议
     */
    void leaveMeeting();
    
    /**
     * @brief 创建会议
     */
    void createMeeting();
    
    /**
     * @brief 邀请参与者
     */
    void inviteParticipants();
    
    /**
     * @brief 复制会议链接
     */
    void copyMeetingLink();
    
    /**
     * @brief 设置加载状态
     * @param loading 是否加载中
     * @param message 加载消息
     */
    void setLoading(bool loading, const QString& message = QString());

private slots:
    /**
     * @brief 处理加入按钮点击
     */
    void handleJoinClicked();

    /**
     * @brief 处理离开按钮点击
     */
    void handleLeaveClicked();

    /**
     * @brief 处理创建按钮点击
     */
    void handleCreateClicked();

    /**
     * @brief 处理设置按钮点击
     */
    void handleSettingsClicked();

    /**
     * @brief 处理邀请按钮点击
     */
    void handleInviteClicked();

    /**
     * @brief 处理复制链接按钮点击
     */
    void handleCopyLinkClicked();

    /**
     * @brief 处理会议状态改变
     * @param status 新状态
     */
    void handleMeetingStatusChanged(int status);

    /**
     * @brief 处理参与者加入
     * @param participantInfo 参与者信息
     */
    void handleParticipantJoined(const QVariantMap& participantInfo);
    
    /**
     * @brief 处理参与者加入
     * @param participantId 参与者ID
     * @param info 参与者信息
     */
    void handleParticipantJoined(const QString& participantId, const QVariantMap& info);

    /**
     * @brief 处理参与者离开
     * @param participantId 参与者ID
     */
    void handleParticipantLeft(const QString& participantId);

    /**
     * @brief 处理连接质量改变
     * @param quality 连接质量
     */
    void handleConnectionQualityChanged(int quality);

    /**
     * @brief 处理URL输入改变
     * @param url 新URL
     */
    void handleUrlChanged(const QString& url);

signals:
    /**
     * @brief 加入会议请求信号
     * @param meetingUrl 会议链接
     */
    void joinMeetingRequested(const QString& meetingUrl);

    /**
     * @brief 离开会议请求信号
     */
    void leaveMeetingRequested();

    /**
     * @brief 创建会议请求信号
     * @param meetingName 会议名称
     * @param settings 会议设置
     */
    void createMeetingRequested(const QString& meetingName, const QVariantMap& settings);

    /**
     * @brief 邀请参与者请求信号
     * @param email 邮箱地址
     * @param message 邀请消息
     */
    void inviteParticipantRequested(const QString& email, const QString& message);

    /**
     * @brief 显示设置请求信号
     */
    void showSettingsRequested();

    /**
     * @brief 复制链接请求信号
     * @param url 会议链接
     */
    void copyLinkRequested(const QString& url);

    /**
     * @brief 显示模式改变信号
     * @param mode 新模式
     */
    void displayModeChanged(DisplayMode mode);

protected:
    /**
     * @brief 绘制事件
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent* event) override;

private:
    /**
     * @brief 初始化界面
     */
    void initializeUI();

    /**
     * @brief 创建会议信息区域
     */
    void createMeetingInfoArea();

    /**
     * @brief 创建控制按钮区域
     */
    void createControlsArea();

    /**
     * @brief 创建参与者列表区域
     */
    void createParticipantsArea();

    /**
     * @brief 创建统计信息区域
     */
    void createStatisticsArea();

    /**
     * @brief 创建状态栏
     */
    void createStatusBar();

    /**
     * @brief 设置布局
     */
    void setupLayout();

    /**
     * @brief 连接信号槽
     */
    void connectSignals();

    /**
     * @brief 应用样式
     */
    void applyStyles();

    /**
     * @brief 更新布局
     */
    void updateLayout();

    /**
     * @brief 格式化持续时间
     * @param seconds 秒数
     * @return 格式化字符串
     */
    QString formatDuration(qint64 seconds) const;

    /**
     * @brief 格式化参与者数量
     * @param count 参与者数量
     * @return 格式化字符串
     */
    QString formatParticipantCount(int count) const;

    /**
     * @brief 获取状态图标
     * @param status 状态值
     * @return 图标路径
     */
    QString getStatusIcon(int status) const;

    /**
     * @brief 获取质量颜色
     * @param quality 质量值
     * @return 颜色字符串
     */
    QString getQualityColor(int quality) const;

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // MEETINGWIDGET_H