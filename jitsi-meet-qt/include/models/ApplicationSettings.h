#ifndef APPLICATIONSETTINGS_H
#define APPLICATIONSETTINGS_H

#include <QString>
#include <QRect>
#include <QStringList>
#include <QVariant>
#include <QMetaType>

/**
 * @brief 应用程序设置数据结构
 */
struct ApplicationSettings {
    // 服务器配置
    QString defaultServerUrl;
    int serverTimeout;
    
    // 界面配置
    QString language;
    bool darkMode;
    
    // 窗口配置
    QRect windowGeometry;
    bool maximized;
    bool rememberWindowState;
    
    // 功能配置
    bool autoJoinAudio;
    bool autoJoinVideo;
    int maxRecentItems;
    
    // 最近使用的URL列表
    QStringList recentUrls;
    
    /**
     * @brief 默认构造函数，设置默认值
     */
    ApplicationSettings();
    
    /**
     * @brief 拷贝构造函数
     */
    ApplicationSettings(const ApplicationSettings& other);
    
    /**
     * @brief 赋值操作符
     */
    ApplicationSettings& operator=(const ApplicationSettings& other);
    
    /**
     * @brief 相等比较操作符
     */
    bool operator==(const ApplicationSettings& other) const;
    
    /**
     * @brief 不等比较操作符
     */
    bool operator!=(const ApplicationSettings& other) const;
    
    /**
     * @brief 验证设置的有效性
     * @return 设置是否有效
     */
    bool isValid() const;
    
    /**
     * @brief 重置为默认值
     */
    void resetToDefaults();
    
    /**
     * @brief 转换为QVariantMap
     * @return QVariantMap表示
     */
    QVariantMap toVariantMap() const;
    
    /**
     * @brief 从QVariantMap加载
     * @param map 要加载的数据
     */
    void fromVariantMap(const QVariantMap& map);
    
    /**
     * @brief 获取设置的字符串表示（用于调试）
     * @return 字符串表示
     */
    QString toString() const;
};

// 注册元类型，以便在Qt信号槽中使用
Q_DECLARE_METATYPE(ApplicationSettings)

#endif // APPLICATIONSETTINGS_H