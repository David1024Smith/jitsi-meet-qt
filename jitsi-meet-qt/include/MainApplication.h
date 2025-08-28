#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>

class WindowManager;
class ConfigurationManager;
class ProtocolHandler;
class TranslationManager;
class PerformanceManager;
class MemoryLeakDetector;
class StartupOptimizer;
class OptimizedRecentManager;
class MemoryProfiler;

/**
 * @brief 主应用程序类，管理应用程序生命周期和单例模式
 */
class MainApplication : public QApplication
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param argc 命令行参数数量
     * @param argv 命令行参数数组
     */
    MainApplication(int argc, char *argv[]);
    
    /**
     * @brief 析构函数
     */
    ~MainApplication();
    
    /**
     * @brief 获取应用程序单例实例
     * @return MainApplication指针
     */
    static MainApplication* instance();
    
    /**
     * @brief 处理协议URL
     * @param url 协议URL字符串
     */
    void handleProtocolUrl(const QString& url);
    
    /**
     * @brief 获取窗口管理器
     * @return WindowManager指针
     */
    WindowManager* windowManager() const;
    
    /**
     * @brief 获取配置管理器
     * @return ConfigurationManager指针
     */
    ConfigurationManager* configurationManager() const;
    
    /**
     * @brief 获取协议处理器
     * @return ProtocolHandler指针
     */
    ProtocolHandler* protocolHandler() const;
    
    /**
     * @brief 获取翻译管理器
     * @return TranslationManager指针
     */
    TranslationManager* translationManager() const;
    
    /**
     * @brief 获取性能管理器
     * @return PerformanceManager指针
     */
    PerformanceManager* performanceManager() const;
    
    /**
     * @brief 获取优化的最近项目管理器
     * @return OptimizedRecentManager指针
     */
    OptimizedRecentManager* recentManager() const;

public slots:
    /**
     * @brief 处理第二个实例启动
     * @param arguments 命令行参数
     */
    void onSecondInstance(const QString& arguments);
    
    /**
     * @brief 应用程序退出处理
     */
    void onAboutToQuit();
    
    /**
     * @brief 处理窗口改变事件
     * @param type 新的窗口类型
     */
    void onWindowChanged(int type);
    
    /**
     * @brief 处理窗口状态改变事件
     * @param type 窗口类型
     * @param state 新的窗口状态
     */
    void onWindowStateChanged(int type, int state);
    
    /**
     * @brief 处理配置改变事件
     */
    void onConfigurationChanged();
    
    /**
     * @brief 处理内存警告事件
     * @param memoryUsage 当前内存使用量
     */
    void onMemoryWarning(qint64 memoryUsage);
    
    /**
     * @brief 处理内存泄漏检测事件
     * @param leaks 检测到的内存泄漏信息
     */
    void onMemoryLeakDetected(const QVariantList& leaks);
    
    /**
     * @brief 处理窗口间数据传递事件
     * @param fromType 源窗口类型
     * @param toType 目标窗口类型
     * @param data 传递的数据
     */
    void onDataTransferred(int fromType, int toType, const QVariantMap& data);
    
    /**
     * @brief 处理窗口创建事件
     * @param type 窗口类型
     */
    void onWindowCreated(int type);
    
    /**
     * @brief 处理窗口销毁事件
     * @param type 窗口类型
     */
    void onWindowDestroyed(int type);
    
    /**
     * @brief 处理最近项目变更事件
     */
    void onRecentItemsChanged();

private slots:
    /**
     * @brief 处理本地服务器新连接
     */
    void onNewConnection();
    
    /**
     * @brief 处理本地套接字数据
     */
    void onSocketReadyRead();

private:
    /**
     * @brief 设置单例模式
     * @return 如果是第一个实例返回true，否则返回false
     */
    bool setupSingleInstance();
    
    /**
     * @brief 注册协议处理器
     */
    void registerProtocolHandler();
    
    /**
     * @brief 初始化管理器组件
     */
    void initializeManagers();
    
    /**
     * @brief 解析命令行参数
     */
    void parseCommandLineArguments();
    
    /**
     * @brief 发送消息到第一个实例
     * @param message 要发送的消息
     * @return 发送成功返回true
     */
    bool sendMessageToFirstInstance(const QString& message);
    
    /**
     * @brief 设置核心组件间的信号连接
     */
    void setupCoreConnections();
    
    /**
     * @brief 显示初始窗口
     */
    void showInitialWindow();

private:
    static MainApplication* s_instance;
    
    // 单例相关
    QLocalServer* m_localServer;
    QString m_serverName;
    bool m_isFirstInstance;
    
    // 管理器组件
    WindowManager* m_windowManager;
    ConfigurationManager* m_configManager;
    ProtocolHandler* m_protocolHandler;
    TranslationManager* m_translationManager;
    PerformanceManager* m_performanceManager;
    MemoryLeakDetector* m_memoryLeakDetector;
    StartupOptimizer* m_startupOptimizer;
    OptimizedRecentManager* m_recentManager;
    MemoryProfiler* m_memoryProfiler;
    
    // 启动参数
    QString m_startupUrl;
    bool m_showWelcome;
};

#endif // MAINAPPLICATION_H