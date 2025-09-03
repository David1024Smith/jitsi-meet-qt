#ifndef CONFERENCEWINDOW_H
#define CONFERENCEWINDOW_H

#include <QMainWindow>
// WebEngine相关头文件
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QLabel>
#include <QWidget>
#include <QTextEdit>
#include <QWebChannel>
#include <QWebSocket>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QUrl>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslConfiguration>
#include "NetworkDiagnostics.h"


QT_BEGIN_NAMESPACE
// WebEngine classes
class QWebEngineView;
class QWebEnginePage;
class QWebEngineProfile;
class QLabel;
class QWidget;
class QVBoxLayout;
class QHBoxLayout;
class QToolBar;
class QAction;
class QProgressBar;
class QTimer;
QT_END_NAMESPACE

class ConfigurationManager;
class JitsiMeetAPI;

/**
 * @brief 会议窗口类 - 负责显示Jitsi Meet会议界面
 * 
 * 这个类是应用程序的核心窗口，负责：
 * - 加载和显示Jitsi Meet Web界面
 * - 处理会议相关的用户交互
 * - 管理窗口状态和配置
 * - 与Jitsi Meet API进行通信
 * - 处理会议事件和回调
 */
class ConferenceWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit ConferenceWindow(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ConferenceWindow();
    
    /**
     * @brief 加载会议URL
     * @param url 会议URL
     * @param displayName 显示名称
     * @param password 会议密码（可选）
     * @return 是否成功开始加载
     */
    bool loadConference(const QString& url, const QString& displayName = QString(), const QString& password = QString());
    
    /**
     * @brief 加载会议房间
     * @param roomName 房间名称
     * @param serverUrl 服务器URL（可选，使用默认服务器）
     * @param displayName 显示名称
     * @param password 会议密码（可选）
     * @return 是否成功开始加载
     */
    bool loadRoom(const QString& roomName, const QString& serverUrl = QString(), 
                  const QString& displayName = QString(), const QString& password = QString());
    
    /**
     * @brief 获取当前会议URL
     * @return 当前会议URL
     */
    QString getCurrentUrl() const;
    
    /**
     * @brief 获取当前房间名称
     * @return 当前房间名称
     */
    QString getCurrentRoom() const;
    
    /**
     * @brief 检查是否正在会议中
     * @return 是否在会议中
     */
    bool isInConference() const;
    
    /**
     * @brief 离开会议
     */
    void leaveConference();
    
    /**
     * @brief 加入会议
     * @param roomName 房间名称
     * @param serverUrl 服务器URL
     */
    void joinConference(const QString& roomName, const QString& serverUrl = QString());
    
    /**
     * @brief 切换静音状态
     */
    void toggleMute();
    
    /**
     * @brief 切换摄像头状态
     */
    void toggleCamera();
    
    /**
     * @brief 切换屏幕共享
     */
    void toggleScreenShare();
    
    /**
     * @brief 显示/隐藏聊天面板
     */
    void toggleChat();
    
    /**
     * @brief 切换全屏模式
     */
    void toggleFullscreen();
    
    /**
     * @brief 设置显示名称
     * @param displayName 显示名称
     */
    void setDisplayName(const QString& displayName);
    
    /**
     * @brief 发送聊天消息
     * @param message 消息内容
     */
    void sendChatMessage(const QString& message);

protected:
    /**
     * @brief 窗口关闭事件
     * @param event 关闭事件
     */
    void closeEvent(QCloseEvent *event) override;
    
    /**
     * @brief 窗口大小改变事件
     * @param event 大小改变事件
     */
    void resizeEvent(QResizeEvent *event) override;
    
    /**
     * @brief 窗口显示事件
     * @param event 显示事件
     */
    void showEvent(QShowEvent *event) override;
    
    /**
     * @brief 窗口隐藏事件
     * @param event 隐藏事件
     */
    void hideEvent(QHideEvent *event) override;

public slots:
    /**
     * @brief 处理页面加载开始
     */
    void onLoadStarted();
    
    /**
     * @brief 处理页面加载进度
     * @param progress 加载进度（0-100）
     */
    void onLoadProgress(int progress);
    
    /**
     * @brief 处理页面加载完成
     * @param success 是否加载成功
     */
    void onLoadFinished(bool success);
    
    /**
     * @brief 处理页面标题变化
     * @param title 新标题
     */
    void onTitleChanged(const QString& title);
    
    /**
     * @brief 处理页面URL变化
     * @param url 新URL
     */
    void onUrlChanged(const QUrl& url);
    
    /**
     * @brief 处理JavaScript消息
     * @param message 消息内容
     */
    void onJavaScriptMessage(const QJsonObject& message);
    
    /**
     * @brief 处理网络错误
     * @param error 错误信息
     */
    void onNetworkError(const QString& error);
    
    /**
     * @brief 处理会议加入事件
     */
    void onConferenceJoined();
    
    /**
     * @brief 处理会议离开事件
     */
    void onConferenceLeft();
    
    /**
     * @brief 处理参与者加入事件
     * @param participantId 参与者ID
     * @param displayName 显示名称
     */
    void onParticipantJoined(const QString& participantId, const QString& displayName);
    
    /**
     * @brief 处理参与者离开事件
     * @param participantId 参与者ID
     */
    void onParticipantLeft(const QString& participantId);
    
    /**
     * @brief 处理聊天消息接收
     * @param senderId 发送者ID
     * @param message 消息内容
     * @param timestamp 时间戳
     */
    void onChatMessageReceived(const QString& senderId, const QString& message, qint64 timestamp);
    
    /**
     * @brief 处理音频状态变化
     * @param muted 是否静音
     */
    void onAudioMuteChanged(bool muted);
    
    /**
     * @brief 处理视频状态变化
     * @param muted 是否关闭摄像头
     */
    void onVideoMuteChanged(bool muted);
    
    /**
     * @brief 处理屏幕共享状态变化事件
     * @param sharing 是否正在共享屏幕
     */
    void onScreenShareChanged(bool sharing);
    
    /**
     * @brief 处理功能权限请求事件
     * @param url 请求权限的URL
     * @param feature 请求的功能
     */
    void onFeaturePermissionRequested(const QUrl& url, QWebEnginePage::Feature feature);
    
    /**
     * @brief 处理Jitsi Meet加载完成事件
     */
    void onJitsiMeetLoaded();
    
    /**
     * @brief 处理会议状态更新事件
     * @param state 会议状态JSON对象
     */
    void onConferenceStateUpdated(const QJsonObject& state);
    
    /**
     * @brief 处理JavaScript错误事件
     * @param error 错误信息
     */
    void onJavaScriptError(const QString& error);
    
    /**
     * @brief 处理Promise拒绝事件
     * @param reason 拒绝原因
     */
    void onPromiseRejected(const QString& reason);
    
    /**
     * @brief 处理API连接成功
     */
    void onApiConnected();
    
    /**
     * @brief 处理API连接断开
     */
    void onApiDisconnected();
    
    /**
     * @brief 处理房间加入成功
     * @param roomName 房间名称
     */
    void onRoomJoined(const QString& roomName);
    
    /**
     * @brief 处理房间离开
     * @param roomName 房间名称
     */
    void onRoomLeft(const QString& roomName);
    
    /**
     * @brief 处理API错误
     * @param error 错误信息
     */
    void onApiError(const QString& error);

private:
    /**
     * @brief 启用或禁用会议控制按钮
     * @param enabled 是否启用
     */
    void enableConferenceControls(bool enabled);
    
    /**
     * @brief 显示错误消息
     * @param message 错误消息
     */
    void showErrorMessage(const QString& message);

signals:
    /**
     * @brief 会议窗口关闭信号
     */
    void windowClosed();
    
    /**
     * @brief 会议加入成功信号
     * @param roomName 房间名称
     */
    void conferenceJoined(const QString& roomName);
    
    /**
     * @brief 会议离开信号
     * @param roomName 房间名称
     */
    void conferenceLeft(const QString& roomName);
    
    /**
     * @brief 会议加载失败信号
     * @param error 错误信息
     */
    void conferenceLoadFailed(const QString& error);
    
    /**
     * @brief 参与者数量变化信号
     * @param count 参与者数量
     */
    void participantCountChanged(int count);
    
    /**
     * @brief 聊天消息信号
     * @param senderId 发送者ID
     * @param message 消息内容
     * @param timestamp 时间戳
     */
    void chatMessageReceived(const QString& senderId, const QString& message, qint64 timestamp);

private slots:
    /**
     * @brief 处理工具栏动作
     */
    void onMuteAction();
    void onCameraAction();
    void onScreenShareAction();
    void onChatAction();
    void onFullscreenAction();
    void onLeaveAction();
    void onSettingsAction();
    
    /**
     * @brief 处理连接超时
     */
    void onConnectionTimeout();
    
    /**
     * @brief 处理重连定时器
     */
    void onReconnectTimer();
    
    /**
     * @brief 处理重连请求
     */
    void onReconnectRequested();
    
    /**
     * @brief 处理网络请求完成
     */
    void onNetworkReplyFinished();
    
    /**
     * @brief 处理网络错误
     * @param error 网络错误类型
     */
    void onNetworkError(QNetworkReply::NetworkError error);
    
    /**
     * @brief 网络诊断完成槽函数
     * @param results 诊断结果
     */
    void onNetworkDiagnosticsCompleted(const QJsonObject& results);
    
    /**
     * @brief 网络诊断错误槽函数
     * @param error 错误信息
     */
    void onNetworkDiagnosticsError(const QString& error);
    
    // WebEngine信号处理
    // onLoadStarted、onLoadProgress、onLoadFinished、onTitleChanged和onUrlChanged方法已在前面声明，此处删除重复声明

private:
    /**
     * @brief 初始化UI界面
     */
    void initializeUI();
    
    /**
     * @brief 初始化WebEngine
     */
    void initializeWebEngine();
    
    /**
     * @brief 初始化工具栏
     */
    void initializeToolbar();
    
    /**
     * @brief 初始化JavaScript桥接
     */
    void initializeJavaScriptBridge();
    
    /**
     * @brief 设置WebEngine配置
     */
    void setupWebEngineSettings();
    
    /**
     * @brief 注入JavaScript代码
     */
    void injectJavaScript();
    
    /**
     * @brief 构建会议URL
     * @param roomName 房间名称
     * @param serverUrl 服务器URL
     * @param displayName 显示名称
     * @param password 密码
     * @return 完整的会议URL
     */
    QString buildConferenceUrl(const QString& roomName, const QString& serverUrl, 
                              const QString& displayName, const QString& password) const;
    
    /**
     * @brief 解析会议URL
     * @param url 会议URL
     * @return 解析结果（房间名、服务器等）
     */
    QJsonObject parseConferenceUrl(const QString& url) const;
    
    /**
     * @brief 保存窗口状态
     */
    void saveWindowState();
    
    /**
     * @brief 恢复窗口状态
     */
    void restoreWindowState();
    
    /**
     * @brief 更新窗口标题
     */
    void updateWindowTitle();
    
    /**
     * @brief 更新工具栏状态
     */
    void updateToolbarState();
    
    /**
     * @brief 显示加载指示器
     * @param show 是否显示
     */
    void showLoadingIndicator(bool show);
    
    /**
     * @brief 显示错误消息
     * @param error 错误信息
     */
    void showError(const QString& error);
    
    /**
     * @brief 处理JavaScript调用
     * @param functionName 函数名
     * @param args 参数
     */
    void handleJavaScriptCall(const QString& functionName, const QJsonObject& args);
    
    /**
     * @brief 执行JavaScript代码
     * @param script JavaScript代码
     * @param callback 回调函数
     */
    void executeJavaScript(const QString& script, std::function<void(const QVariant&)> callback = nullptr);

private:
    // UI组件
    QWidget* m_centralWidget;           ///< 中央窗口部件
    QVBoxLayout* m_mainLayout;          ///< 主布局
    // WebEngine组件 - 已启用MSVC WebEngine支持
#ifdef _MSC_VER
    QWebEngineView* m_webView;          ///< Web视图
    QWebEnginePage* m_webPage;          ///< Web页面
#endif
    QWebChannel* m_webChannel;          ///< Web通道
    QWebSocket* m_webSocket;            ///< Web套接字
    QLabel* m_statusDisplay;            ///< 状态显示
    QWidget* m_webContainer;            ///< Web容器
    
    // 工具栏和控件
    QToolBar* m_toolbar;                ///< 工具栏
    QAction* m_muteAction;              ///< 静音动作
    QAction* m_cameraAction;            ///< 摄像头动作
    QAction* m_screenShareAction;       ///< 屏幕共享动作
    QAction* m_chatAction;              ///< 聊天动作
    QAction* m_fullscreenAction;        ///< 全屏动作
    QAction* m_leaveAction;             ///< 离开动作
    QAction* m_settingsAction;          ///< 设置动作
    
    // 状态指示器
    QLabel* m_statusLabel;              ///< 状态标签
    QProgressBar* m_progressBar;        ///< 进度条
    QLabel* m_participantCountLabel;    ///< 参与者数量标签
    
    // 定时器
    QTimer* m_connectionTimer;          ///< 连接超时定时器
    QTimer* m_reconnectTimer;           ///< 重连定时器
    
    // 网络管理
    QNetworkAccessManager* m_networkManager; ///< 网络访问管理器
    
    // 配置和API
    ConfigurationManager* m_configManager; ///< 配置管理器
    JitsiMeetAPI* m_jitsiAPI;           ///< Jitsi Meet API
    NetworkDiagnostics* m_networkDiagnostics; ///< 网络诊断工具
    
    // 状态变量
    QString m_currentUrl;               ///< 当前URL
    QString m_currentRoom;              ///< 当前房间名
    QString m_currentServer;            ///< 当前服务器
    QString m_displayName;              ///< 显示名称
    bool m_isInConference;              ///< 是否在会议中
    bool m_isLoading;                   ///< 是否正在加载
    bool m_isMuted;                     ///< 是否静音
    bool m_isCameraOff;                 ///< 是否关闭摄像头
    bool m_isScreenSharing;             ///< 是否屏幕共享
    bool m_isChatVisible;               ///< 聊天是否可见
    bool m_isFullscreen;                ///< 是否全屏
    int m_participantCount;             ///< 参与者数量
    int m_loadProgress;                 ///< 加载进度
    
    // 重连相关
    int m_reconnectAttempts;            ///< 重连尝试次数
    static const int MAX_RECONNECT_ATTEMPTS = 3; ///< 最大重连次数
    static const int RECONNECT_DELAY = 5000;     ///< 重连延迟（毫秒）
    static const int CONNECTION_TIMEOUT = 30000; ///< 连接超时（毫秒）
};

#endif // CONFERENCEWINDOW_H