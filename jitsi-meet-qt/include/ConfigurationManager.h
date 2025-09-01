#ifndef CONFIGURATIONMANAGER_H
#define CONFIGURATIONMANAGER_H

#include <QObject>
#include <QVariant>
#include <QStringList>

class QSettings;

/**
 * @brief 配置管理器
 * 
 * 负责管理应用程序的配置设置，包括用户偏好、系统设置等。
 */
class ConfigurationManager : public QObject
{
    Q_OBJECT

public:
    explicit ConfigurationManager(QObject *parent = nullptr);
    ~ConfigurationManager();

    /**
     * @brief 获取单例实例
     */
    static ConfigurationManager* instance();

    /**
     * @brief 初始化配置管理器
     * @return 是否成功初始化
     */
    bool initialize();

    /**
     * @brief 关闭配置管理器
     */
    void shutdown();

    /**
     * @brief 获取配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 设置配置值
     * @param key 配置键
     * @param value 配置值
     */
    void setValue(const QString& key, const QVariant& value);

    /**
     * @brief 检查是否存在指定键
     * @param key 配置键
     * @return 是否存在
     */
    bool hasKey(const QString& key) const;

    /**
     * @brief 删除配置键
     * @param key 配置键
     */
    void removeKey(const QString& key);

    /**
     * @brief 获取所有配置键
     * @return 配置键列表
     */
    QStringList getAllKeys() const;

    /**
     * @brief 重置所有配置为默认值
     */
    void resetToDefaults();

    /**
     * @brief 检查是否为暗色模式
     * @return 是否为暗色模式
     */
    bool isDarkMode() const;

signals:
    /**
     * @brief 配置更改信号
     * @param key 配置键
     * @param value 新值
     */
    void configurationChanged(const QString& key, const QVariant& value);

    /**
     * @brief 语言更改信号
     * @param language 新语言代码
     */
    void languageChanged(const QString& language);

    /**
     * @brief 暗色模式更改信号
     * @param darkMode 是否为暗色模式
     */
    void darkModeChanged(bool darkMode);

private:
    static ConfigurationManager* s_instance;
    QSettings* m_settings;
    bool m_initialized;
};

#endif // CONFIGURATIONMANAGER_H