#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QMainWindow>
#include <QDialog>
#include <QVariantMap>
#include <QHash>
#include <QPointer>
#include <QTimer>

class WindowStateManager;
class WelcomeWindow;
class ConferenceWindow;
class SettingsDialog;
class ConfigurationManager;
class TranslationManager;

/**
 * @brief 窗口管理器，负责管理所有应用程序窗口
 * 
 * 该类统一管理应用程序中的所有窗口，包括窗口的创建、显示、隐藏、切换、
 * 数据传递、状态同步以及生命周期管理。
 */
class WindowManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 窗口类型枚举
     */
    enum WindowType {
        WelcomeWindow,
        ConferenceWindow,
        SettingsDialog
    };
    Q_ENUM(WindowType)

    /**
     * @brief 窗口状态枚举
     */
    enum WindowState {
        WindowHidden,
        WindowVisible,
        WindowMinimized,
        WindowMaximized
    };
    Q_ENUM(WindowState)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit WindowManager(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~WindowManager();
    
    /**
     * @brief 设置配置管理器
     * @param configManager 配置管理器实例
     */
    void setConfigurationManager(ConfigurationManager* configManager);
    
    /**
     * @brief 设置翻译管理器
     * @param translationManager 翻译管理器实例
     */
    void setTranslationManager(TranslationManager* translationManager);
    
    /**
     * @brief 显示指定类型的窗口
     * @param type 窗口类型
     * @param data 传递给窗口的数据
     */
    void showWindow(WindowType type, const QVariantMap& data = QVariantMap());
    
    /**
     * @brief 关闭指定类型的窗口
     * @param type 窗口类型
     */
    void closeWindow(WindowType type);
    
    /**
     * @brief 隐藏指定类型的窗口
     * @param type 窗口类型
     */
    void hideWindow(WindowType type);
    
    /**
     * @brief 获取当前活动窗口
     * @return 当前窗口指针
     */
    QMainWindow* currentWindow() const;
    
    /**
     * @brief 获取指定类型的窗口
     * @param type 窗口类型
     * @return 窗口指针，如果不存在则返回nullptr
     */
    QWidget* getWindow(WindowType type) const;
    
    /**
     * @brief 获取当前窗口类型
     * @return 当前窗口类型
     */
    WindowType currentWindowType() const;
    
    /**
     * @brief 获取窗口状态
     * @param type 窗口类型
     * @return 窗口状态
     */
    WindowState getWindowState(WindowType type) const;
    
    /**
     * @brief 检查窗口是否存在
     * @param type 窗口类型
     * @return 窗口是否存在
     */
    bool hasWindow(WindowType type) const;
    
    /**
     * @brief 检查窗口是否可见
     * @param type 窗口类型
     * @return 窗口是否可见
     */
    bool isWindowVisible(WindowType type) const;
    
    /**
     * @brief 传递数据到指定窗口
     * @param type 目标窗口类型
     * @param data 要传递的数据
     * @return 是否传递成功
     */
    bool sendDataToWindow(WindowType type, const QVariantMap& data);
    
    /**
     * @brief 同步窗口状态
     * @param type 窗口类型
     */
    void syncWindowState(WindowType type);
    
    /**
     * @brief 保存所有窗口状态
     */
    void saveAllWindowStates();
    
    /**
     * @brief 恢复所有窗口状态
     */
    void restoreAllWindowStates();
    
    /**
     * @brief 清理未使用的窗口
     */
    void cleanupUnusedWindows();
    
    /**
     * @brief 关闭所有窗口
     */
    void closeAllWindows();

signals:
    /**
     * @brief 窗口改变信号
     * @param type 新的窗口类型
     */
    void windowChanged(WindowType type);
    
    /**
     * @brief 窗口状态改变信号
     * @param type 窗口类型
     * @param state 新的窗口状态
     */
    void windowStateChanged(WindowType type, WindowState state);
    
    /**
     * @brief 窗口数据传递信号
     * @param fromType 源窗口类型
     * @param toType 目标窗口类型
     * @param data 传递的数据
     */
    void dataTransferred(WindowType fromType, WindowType toType, const QVariantMap& data);
    
    /**
     * @brief 窗口创建信号
     * @param type 窗口类型
     */
    void windowCreated(WindowType type);
    
    /**
     * @brief 窗口销毁信号
     * @param type 窗口类型
     */
    void windowDestroyed(WindowType type);

public slots:
    /**
     * @brief 处理欢迎窗口的加入会议请求
     * @param url 会议URL
     */
    void onJoinConference(const QString& url);

private slots:
    
    /**
     * @brief 处理会议窗口的返回请求
     */
    void onBackToWelcome();
    
    /**
     * @brief 处理设置请求
     */
    void onSettingsRequested();
    
    /**
     * @brief 处理设置对话框关闭
     */
    void onSettingsDialogClosed();
    
    /**
     * @brief 处理窗口关闭事件
     */
    void onWindowClosed();
    
    /**
     * @brief 处理窗口状态改变
     */
    void onWindowStateChanged();
    
    /**
     * @brief 定时清理检查
     */
    void onCleanupTimer();
    
    /**
     * @brief 处理会议加入成功
     * @param url 会议URL
     */
    void onConferenceJoined(const QString& url);
    
    /**
     * @brief 处理语言改变
     * @param language 新的语言代码
     */
    void onLanguageChanged(const QString& language);

private:
    /**
     * @brief 创建欢迎窗口
     */
    void createWelcomeWindow();
    
    /**
     * @brief 创建会议窗口
     */
    void createConferenceWindow();
    
    /**
     * @brief 创建设置对话框
     */
    void createSettingsDialog();
    
    /**
     * @brief 连接窗口信号
     * @param type 窗口类型
     * @param window 窗口对象
     */
    void connectWindowSignals(WindowType type, QWidget* window);
    
    /**
     * @brief 断开窗口信号
     * @param type 窗口类型
     * @param window 窗口对象
     */
    void disconnectWindowSignals(WindowType type, QWidget* window);
    
    /**
     * @brief 应用窗口数据
     * @param type 窗口类型
     * @param data 数据
     */
    void applyWindowData(WindowType type, const QVariantMap& data);
    
    /**
     * @brief 更新窗口状态
     * @param type 窗口类型
     * @param state 新状态
     */
    void updateWindowState(WindowType type, WindowState state);
    
    /**
     * @brief 获取窗口类型名称
     * @param type 窗口类型
     * @return 类型名称字符串
     */
    QString getWindowTypeName(WindowType type) const;
    
    /**
     * @brief 检查窗口是否需要清理
     * @param type 窗口类型
     * @return 是否需要清理
     */
    bool shouldCleanupWindow(WindowType type) const;

private:
    // 窗口实例
    QPointer<::WelcomeWindow> m_welcomeWindow;
    QPointer<::ConferenceWindow> m_conferenceWindow;
    QPointer<::SettingsDialog> m_settingsDialog;
    
    // 窗口状态跟踪
    QHash<WindowType, WindowState> m_windowStates;
    QHash<WindowType, QVariantMap> m_windowData;
    QHash<WindowType, qint64> m_lastAccessTime;
    
    // 当前状态
    WindowType m_currentWindowType;
    WindowType m_previousWindowType;
    
    // 管理器组件
    WindowStateManager* m_stateManager;
    ConfigurationManager* m_configManager;
    TranslationManager* m_translationManager;
    
    // 清理定时器
    QTimer* m_cleanupTimer;
    
    // 配置选项
    bool m_autoCleanup;
    int m_cleanupInterval;
    qint64 m_windowTimeout;
};

#endif // WINDOWMANAGER_H