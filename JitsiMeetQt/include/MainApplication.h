#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QTranslator>
#include <QtConcurrent/QtConcurrent>
#include <memory>

class ConferenceWindow;
class WelcomeWindow;
class SettingsDialog;
class ConfigurationManager;
class ProtocolHandler;

// 前向声明ProtocolHandler::MeetingInfo
#include "ProtocolHandler.h"

/**
 * @brief 主应用程序类，负责应用程序的生命周期管理和全局功能
 * 
 * 该类实现了单例模式，管理应用程序的启动、关闭、窗口切换、
 * 系统托盘、协议处理等核心功能。
 */
class MainApplication : public QApplication
{
    Q_OBJECT

signals:
    /**
     * @brief ConferenceWindow准备就绪信号
     */
    void conferenceWindowReady();

public:
    /**
     * @brief 构造函数
     * @param argc 命令行参数数量
     * @param argv 命令行参数数组
     */
    explicit MainApplication(int &argc, char **argv);
    
    /**
     * @brief 析构函数
     */
    ~MainApplication() override;
    
    /**
     * @brief 获取应用程序单例实例
     * @return MainApplication指针
     */
    static MainApplication* instance();
    
    /**
     * @brief 初始化应用程序
     * @return 初始化是否成功
     */
    bool initialize();
    
    /**
     * @brief 异步初始化ConferenceWindow（并行初始化）
     */
    void initializeConferenceWindowAsync();
    
    /**
     * @brief 检查ConferenceWindow是否已准备就绪
     * @return 是否已准备就绪
     */
    bool isConferenceWindowReady() const;
    
    /**
     * @brief 显示欢迎窗口
     */
    void showWelcomeWindow();
    
    /**
     * @brief 显示会议窗口
     * @param roomName 房间名称
     * @param serverUrl 服务器URL
     */
    void showConferenceWindow(const QString &roomName, const QString &serverUrl = QString());
    
    /**
     * @brief 显示设置对话框
     */
    void showSettingsDialog();
    
    /**
     * @brief 处理协议URL
     * @param meetingInfo 会议信息
     */
    void handleProtocolUrl(const ProtocolHandler::MeetingInfo &meetingInfo);
    
    /**
     * @brief 获取配置管理器
     * @return ConfigurationManager指针
     */
    ConfigurationManager* configurationManager() const;
    
    /**
     * @brief 设置应用程序语言
     * @param language 语言代码 (如: "zh_CN", "en_US")
     */
    void setLanguage(const QString &language);
    
    /**
     * @brief 获取当前语言
     * @return 当前语言代码
     */
    QString currentLanguage() const;
    
    // 主题相关方法已移除，现在只使用单一的main.qss样式文件
    
    /**
     * @brief 检查应用程序是否已初始化
     * @return 是否已初始化
     */
    bool isInitialized() const;

public slots:
    /**
     * @brief 退出应用程序
     */
    void quit();
    
    /**
     * @brief 显示关于对话框
     */
    void showAbout();
    
    /**
     * @brief 处理系统托盘图标激活
     * @param reason 激活原因
     */
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

protected:
    /**
     * @brief 处理应用程序事件
     * @param event 事件对象
     * @return 事件是否被处理
     */
    bool event(QEvent *event) override;

private slots:
    /**
     * @brief 会议窗口关闭处理
     */
    void onConferenceWindowClosed();
    
    /**
     * @brief 欢迎窗口关闭处理
     */
    void onWelcomeWindowClosed();
    


private:
    /**
     * @brief 初始化系统托盘
     */
    void initializeSystemTray();
    
    /**
     * @brief 初始化翻译系统
     */
    void initializeTranslations();
    
    /**
     * @brief 初始化主题
     */
    void initializeTheme();
    
    /**
     * @brief 加载样式表
     * @param themeName 主题名称
     */
    void loadStyleSheet(const QString &themeName);
    
    /**
     * @brief 异步预加载静态资源
     */
    void preloadResourcesAsync();
    
    /**
     * @brief 异步加载样式表
     * @param themeName 主题名称
     */
    void loadStyleSheetAsync(const QString &themeName);
    
    /**
     * @brief 样式表异步加载完成处理
     * @param styleSheet 加载的样式表内容
     */
    void onStyleSheetLoaded(const QString &styleSheet);
    
    // 主题初始化方法已移除
    
    /**
     * @brief 创建托盘菜单
     */
    void createTrayMenu();
    
    /**
     * @brief 解析协议URL
     * @param url 协议URL
     * @param roomName 输出房间名称
     * @param serverUrl 输出服务器URL
     * @return 解析是否成功
     */
    bool parseProtocolUrl(const QString &url, QString &roomName, QString &serverUrl);

private:
    static MainApplication* s_instance;  ///< 单例实例
    
    std::unique_ptr<ConferenceWindow> m_conferenceWindow;     ///< 会议窗口
    std::unique_ptr<WelcomeWindow> m_welcomeWindow;           ///< 欢迎窗口
    std::unique_ptr<SettingsDialog> m_settingsDialog;        ///< 设置对话框
    ConfigurationManager* m_configManager;                   ///< 配置管理器
    std::unique_ptr<ProtocolHandler> m_protocolHandler;      ///< 协议处理器
    
    QSystemTrayIcon* m_trayIcon;        ///< 系统托盘图标
    QMenu* m_trayMenu;                  ///< 托盘菜单
    QAction* m_showAction;              ///< 显示动作
    QAction* m_settingsAction;          ///< 设置动作
    QAction* m_aboutAction;             ///< 关于动作
    QAction* m_quitAction;              ///< 退出动作
    
    QTranslator* m_translator;          ///< 翻译器
    QString m_currentLanguage;          ///< 当前语言
    // 主题相关成员变量已移除
    
    bool m_initialized;                 ///< 是否已初始化
    
    // 并行初始化相关
    bool m_conferenceWindowReady;       ///< ConferenceWindow是否已准备就绪
};

#endif // MAINAPPLICATION_H