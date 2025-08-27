#ifndef CONFIGURATIONMANAGER_H
#define CONFIGURATIONMANAGER_H

#include <QObject>
#include <QSettings>
#include <QRect>
#include <QStringList>
#include "models/ApplicationSettings.h"
#include "models/RecentItem.h"

class WindowStateManager;

/**
 * @brief 配置管理器，处理应用程序设置的读取和保存
 */
class ConfigurationManager : public QObject
{
    Q_OBJECT

public:

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ConfigurationManager(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ConfigurationManager();
    
    /**
     * @brief 加载配置
     * @return 应用程序配置
     */
    ApplicationSettings loadConfiguration();
    
    /**
     * @brief 保存配置
     * @param config 要保存的配置
     */
    void saveConfiguration(const ApplicationSettings& config);
    
    /**
     * @brief 获取服务器URL
     * @return 服务器URL
     */
    QString serverUrl() const;
    
    /**
     * @brief 设置服务器URL
     * @param url 服务器URL
     */
    void setServerUrl(const QString& url);
    
    /**
     * @brief 获取语言设置
     * @return 语言代码
     */
    QString language() const;
    
    /**
     * @brief 设置语言
     * @param language 语言代码
     */
    void setLanguage(const QString& language);
    
    /**
     * @brief 获取最近使用的URL列表
     * @return URL列表
     */
    QStringList recentUrls() const;
    
    /**
     * @brief 添加最近使用的URL
     * @param url URL字符串
     */
    void addRecentUrl(const QString& url);
    
    /**
     * @brief 清除最近使用的URL
     */
    void clearRecentUrls();
    
    /**
     * @brief 获取最近会议项目列表
     * @return 最近会议项目列表
     */
    QList<RecentItem> recentItems() const;
    
    /**
     * @brief 添加最近会议项目
     * @param item 会议项目
     */
    void addRecentItem(const RecentItem& item);
    
    /**
     * @brief 移除最近会议项目
     * @param url 会议URL
     */
    void removeRecentItem(const QString& url);
    
    /**
     * @brief 清除所有最近会议项目
     */
    void clearRecentItems();
    
    /**
     * @brief 设置最近会议项目列表
     * @param items 会议项目列表
     */
    void setRecentItems(const QList<RecentItem>& items);
    
    /**
     * @brief 获取最大最近项目数量
     * @return 最大数量
     */
    int maxRecentItems() const;
    
    /**
     * @brief 设置最大最近项目数量
     * @param maxItems 最大数量
     */
    void setMaxRecentItems(int maxItems);
    
    /**
     * @brief 获取窗口几何信息
     * @return 窗口矩形
     */
    QRect windowGeometry() const;
    
    /**
     * @brief 设置窗口几何信息
     * @param geometry 窗口矩形
     */
    void setWindowGeometry(const QRect& geometry);
    
    /**
     * @brief 获取窗口最大化状态
     * @return 是否最大化
     */
    bool isWindowMaximized() const;
    
    /**
     * @brief 设置窗口最大化状态
     * @param maximized 是否最大化
     */
    void setWindowMaximized(bool maximized);
    
    /**
     * @brief 获取深色模式设置
     * @return 是否启用深色模式
     */
    bool isDarkMode() const;
    
    /**
     * @brief 设置深色模式
     * @param darkMode 是否启用深色模式
     */
    void setDarkMode(bool darkMode);
    
    /**
     * @brief 重置配置为默认值
     */
    void resetToDefaults();
    
    /**
     * @brief 获取当前配置
     * @return 当前应用程序配置
     */
    ApplicationSettings currentConfiguration() const;
    
    /**
     * @brief 验证配置文件完整性
     * @return 配置是否有效
     */
    bool validateConfiguration() const;
    
    /**
     * @brief 获取窗口状态管理器
     * @return 窗口状态管理器实例
     */
    WindowStateManager* windowStateManager() const;

signals:
    /**
     * @brief 配置改变信号
     */
    void configurationChanged();
    
    /**
     * @brief 服务器URL改变信号
     * @param url 新的服务器URL
     */
    void serverUrlChanged(const QString& url);
    
    /**
     * @brief 语言改变信号
     * @param language 新的语言代码
     */
    void languageChanged(const QString& language);
    
    /**
     * @brief 深色模式改变信号
     * @param darkMode 新的深色模式状态
     */
    void darkModeChanged(bool darkMode);
    
    /**
     * @brief 最近项目改变信号
     */
    void recentItemsChanged();

private:
    /**
     * @brief 设置默认配置
     */
    void setDefaults();
    
    /**
     * @brief 验证服务器URL
     * @param url 要验证的URL
     * @return 是否有效
     */
    bool validateServerUrl(const QString& url) const;
    
    /**
     * @brief 验证窗口几何信息
     * @param geometry 要验证的几何信息
     * @return 验证后的几何信息
     */
    QRect validateWindowGeometry(const QRect& geometry) const;
    
    /**
     * @brief 验证并修复配置
     * @param settings 要验证的配置
     * @return 验证后的配置
     */
    ApplicationSettings validateAndFixSettings(const ApplicationSettings& settings) const;

private:
    QSettings* m_settings;
    ApplicationSettings m_config;
    bool m_configLoaded;
    WindowStateManager* m_windowStateManager;
};

#endif // CONFIGURATIONMANAGER_H