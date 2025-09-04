#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <cstdio>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QGroupBox>
#include <QSplitter>
#include <QTextEdit>
#include <QProgressBar>
#include <QTimer>
#include <QCloseEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCompleter>
#include <QStringListModel>
#include <QSvgRenderer>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QComboBox;
class QCheckBox;
class QListWidget;
class QListWidgetItem;
class QGroupBox;
class QSplitter;
class QTextEdit;
class QProgressBar;
class QTimer;
class QCompleter;
class QStringListModel;
QT_END_NAMESPACE

class ConfigurationManager;
class ProtocolHandler;

/**
 * @brief 欢迎窗口类 - 应用程序的主入口界面
 * 
 * 这个类提供了用户友好的界面，用于：
 * - 输入会议室名称或URL
 * - 选择服务器
 * - 设置显示名称
 * - 查看会议历史
 * - 快速加入会议
 * - 管理用户偏好设置
 */
class WelcomeWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit WelcomeWindow(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~WelcomeWindow();
    
    /**
     * @brief 设置会议URL或房间名
     * @param url 会议URL或房间名
     */
    void setMeetingUrl(const QString& url);
    
    /**
     * @brief 获取当前输入的会议URL或房间名
     * @return 会议URL或房间名
     */
    QString getMeetingUrl() const;
    
    /**
     * @brief 设置显示名称
     * @param displayName 显示名称
     */
    void setDisplayName(const QString& displayName);
    
    /**
     * @brief 获取显示名称
     * @return 显示名称
     */
    QString getDisplayName() const;
    
    /**
     * @brief 设置服务器URL
     * @param serverUrl 服务器URL
     */
    void setServerUrl(const QString& serverUrl);
    
    /**
     * @brief 获取服务器URL
     * @return 服务器URL
     */
    QString getServerUrl() const;
    
    /**
     * @brief 刷新会议历史列表
     */
    void refreshMeetingHistory();
    
    /**
     * @brief 清除输入字段
     */
    void clearInputs();

protected:
    /**
     * @brief 窗口关闭事件处理
     * @param event 关闭事件
     */
    void closeEvent(QCloseEvent *event) override;
    
    /**
     * @brief 窗口显示事件处理
     * @param event 显示事件
     */
    void showEvent(QShowEvent *event) override;
    
    /**
     * @brief 窗口大小改变事件处理
     * @param event 大小改变事件
     */
    void resizeEvent(QResizeEvent *event) override;

public slots:
    /**
     * @brief 加入会议按钮点击处理
     */
    void onJoinMeeting();
    
    /**
     * @brief 创建会议按钮点击处理
     */
    void onCreateMeeting();
    
    /**
     * @brief 设置按钮点击处理
     */
    void onSettings();
    
    /**
     * @brief 关于按钮点击处理
     */
    void onAbout();
    
    /**
     * @brief 退出按钮点击处理
     */
    void onExit();
    
    /**
     * @brief 侧边栏设置按钮点击事件处理
     */
    void onSidebarSettings();
    
    /**
     * @brief 帮助按钮点击事件处理
     */
    void onHelp();
    
    /**
     * @brief 会议历史项目双击处理
     * @param item 被双击的项目
     */
    void onHistoryItemDoubleClicked(QListWidgetItem* item);
    
    /**
     * @brief 会议历史项目选择改变处理
     */
    void onHistorySelectionChanged();
    
    /**
     * @brief 服务器选择改变处理
     * @param index 选择的索引
     */
    void onServerChanged(int index);
    
    /**
     * @brief URL输入改变处理
     * @param text 输入的文本
     */
    void onUrlChanged(const QString& text);
    
    /**
     * @brief 显示名称输入改变处理
     * @param text 输入的文本
     */
    void onDisplayNameChanged(const QString& text);
    
    /**
     * @brief 服务器可用性检查完成处理
     * @param available 服务器是否可用
     * @param serverUrl 服务器URL
     */
    void onServerAvailabilityChecked(bool available, const QString& serverUrl);

signals:
    /**
     * @brief 请求加入会议信号
     * @param url 会议URL
     * @param displayName 显示名称
     * @param password 会议密码（可选）
     */
    void joinMeetingRequested(const QString& url, const QString& displayName, const QString& password = QString());
    
    /**
     * @brief 请求创建会议信号
     * @param roomName 房间名称
     * @param serverUrl 服务器URL
     * @param displayName 显示名称
     * @param password 会议密码（可选）
     */
    void createMeetingRequested(const QString& roomName, const QString& serverUrl, const QString& displayName, const QString& password = QString());
    
    /**
     * @brief 请求显示设置对话框信号
     */
    void settingsRequested();
    
    /**
     * @brief 窗口关闭信号
     */
    void windowClosed();

private slots:
    /**
     * @brief 服务器检查超时处理
     */
    void onServerCheckTimeout();
    
    /**
     * @brief URL验证定时器超时处理
     */
    void onUrlValidationTimeout();

private:
    /**
     * @brief 初始化用户界面
     */
    void initializeUI();
    
    /**
     * @brief 初始化布局
     */
    void initializeLayout();
    
    /**
     * @brief 初始化连接
     */
    void initializeConnections();
    
    /**
     * @brief 初始化自动完成
     */
    void initializeAutoComplete();
    
    /**
     * @brief 加载会议历史
     */
    void loadMeetingHistory();
    
    /**
     * @brief 加载服务器列表
     */
    void loadServerList();
    
    /**
     * @brief 保存窗口状态
     */
    void saveWindowState();
    
    /**
     * @brief 恢复窗口状态
     */
    void restoreWindowState();
    
    /**
     * @brief 验证输入
     * @return 输入是否有效
     */
    bool validateInput();
    
    /**
     * @brief 解析会议URL
     * @param url 会议URL
     * @return 解析结果
     */
    QJsonObject parseMeetingUrl(const QString& url);
    
    /**
     * @brief 检查服务器可用性
     * @param serverUrl 服务器URL
     */
    void checkServerAvailability(const QString& serverUrl);
    
    /**
     * @brief 更新UI状态
     */
    void updateUIState();
    
    /**
     * @brief 更新会议历史显示
     */
    void updateHistoryDisplay();
    
    /**
     * @brief 添加到会议历史
     * @param url 会议URL
     * @param displayName 显示名称
     * @param serverUrl 服务器URL
     */
    void addToHistory(const QString& url, const QString& displayName, const QString& serverUrl);

private:
    // UI组件
    QWidget* m_centralWidget;           ///< 中央窗口部件
    QHBoxLayout* m_mainLayout;          ///< 主布局
    QSplitter* m_splitter;              ///< 分割器
    
    // 左侧菜单栏
    QWidget* m_sidebarPanel;            ///< 左侧菜单栏面板
    QVBoxLayout* m_sidebarLayout;       ///< 左侧菜单栏布局
    QLabel* m_logoLabel;                ///< Logo标签
    QPushButton* m_sidebarSettingsButton; ///< 侧边栏设置按钮
    QPushButton* m_helpButton;          ///< 帮助按钮
    
    // 主内容面板
    QWidget* m_leftPanel;               ///< 主内容面板
    QVBoxLayout* m_leftLayout;          ///< 主内容布局
    QGroupBox* m_joinGroup;             ///< 加入会议组
    QGridLayout* m_joinLayout;          ///< 加入会议布局
    
    QLabel* m_urlLabel;                 ///< URL标签
    QLineEdit* m_urlEdit;               ///< URL输入框
    QLabel* m_displayNameLabel;         ///< 显示名称标签
    QLineEdit* m_displayNameEdit;       ///< 显示名称输入框
    QLabel* m_serverLabel;              ///< 服务器标签
    QComboBox* m_serverCombo;           ///< 服务器选择框
    QLabel* m_passwordLabel;            ///< 密码标签
    QLineEdit* m_passwordEdit;          ///< 密码输入框
    
    QHBoxLayout* m_buttonLayout;        ///< 按钮布局
    QPushButton* m_joinButton;          ///< 加入按钮
    QPushButton* m_createButton;        ///< 创建按钮
    
    QHBoxLayout* m_actionLayout;        ///< 动作布局
    QPushButton* m_settingsButton;      ///< 设置按钮
    QPushButton* m_aboutButton;         ///< 关于按钮
    QPushButton* m_exitButton;          ///< 退出按钮
    
    // 右侧面板
    QWidget* m_rightPanel;              ///< 右侧面板
    QVBoxLayout* m_rightLayout;         ///< 右侧布局
    QGroupBox* m_historyGroup;          ///< 历史记录组
    QVBoxLayout* m_historyLayout;       ///< 历史记录布局
    QListWidget* m_historyList;         ///< 历史记录列表
    QPushButton* m_clearHistoryButton;  ///< 清除历史按钮
    
    QGroupBox* m_infoGroup;             ///< 信息组
    QVBoxLayout* m_infoLayout;          ///< 信息布局
    QTextEdit* m_infoText;              ///< 信息文本
    
    // 状态栏组件
    QLabel* m_statusLabel;              ///< 状态标签
    QProgressBar* m_progressBar;        ///< 进度条
    
    // 定时器
    QTimer* m_serverCheckTimer;         ///< 服务器检查定时器
    QTimer* m_urlValidationTimer;       ///< URL验证定时器
    
    // 网络组件
    QNetworkAccessManager* m_networkManager; ///< 网络访问管理器
    
    // 自动完成
    QCompleter* m_urlCompleter;         ///< URL自动完成
    QStringListModel* m_urlModel;       ///< URL模型
    QCompleter* m_nameCompleter;        ///< 名称自动完成
    QStringListModel* m_nameModel;      ///< 名称模型
    
    // 管理器
    ConfigurationManager* m_configManager; ///< 配置管理器
    ProtocolHandler* m_protocolHandler; ///< 协议处理器
    
    // 状态变量
    bool m_isValidatingUrl;             ///< 是否正在验证URL
    bool m_isCheckingServer;            ///< 是否正在检查服务器
    QString m_lastValidatedUrl;         ///< 最后验证的URL
    QString m_lastCheckedServer;        ///< 最后检查的服务器
    FILE* debugFile;                    ///< 调试文件指针
    
    // 常量
    static const int SERVER_CHECK_TIMEOUT = 5000;  ///< 服务器检查超时（毫秒）
    static const int URL_VALIDATION_DELAY = 500;    ///< URL验证延迟（毫秒）
    static const int MAX_HISTORY_ITEMS = 50;        ///< 最大历史记录数量
};

#endif // WELCOMEWINDOW_H