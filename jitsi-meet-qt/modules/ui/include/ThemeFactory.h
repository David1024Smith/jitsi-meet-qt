#ifndef THEMEFACTORY_H
#define THEMEFACTORY_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <memory>
#include <functional>

class BaseTheme;

/**
 * @brief 主题工厂类
 * 
 * ThemeFactory负责创建和管理各种主题实例，支持主题的
 * 注册、创建、缓存和生命周期管理。
 */
class ThemeFactory : public QObject
{
    Q_OBJECT

public:
    using ThemeCreator = std::function<std::shared_ptr<BaseTheme>()>;

    explicit ThemeFactory(QObject *parent = nullptr);
    ~ThemeFactory();

    // 主题创建
    std::shared_ptr<BaseTheme> createTheme(const QString& themeName);
    std::shared_ptr<BaseTheme> createDefaultTheme();
    std::shared_ptr<BaseTheme> createThemeFromConfig(const QVariantMap& config);

    // 主题注册
    bool registerTheme(const QString& themeName, ThemeCreator creator);
    bool unregisterTheme(const QString& themeName);
    bool isThemeRegistered(const QString& themeName) const;

    // 主题查询
    QStringList availableThemes() const;
    QStringList registeredThemes() const;
    QString defaultThemeName() const;
    bool hasTheme(const QString& themeName) const;

    // 主题信息
    QString getThemeDisplayName(const QString& themeName) const;
    QString getThemeDescription(const QString& themeName) const;
    QVariantMap getThemeMetadata(const QString& themeName) const;

    // 主题缓存
    void enableCaching(bool enabled);
    bool isCachingEnabled() const;
    void clearCache();
    void clearThemeCache(const QString& themeName);

    // 主题验证
    bool validateTheme(const QString& themeName) const;
    bool validateThemeConfig(const QVariantMap& config) const;

    // 单例访问
    static ThemeFactory* instance();

    // 预定义主题注册
    void registerBuiltinThemes();

signals:
    void themeCreated(const QString& themeName);
    void themeRegistered(const QString& themeName);
    void themeUnregistered(const QString& themeName);
    void cacheCleared();
    void errorOccurred(const QString& error);

private slots:
    void onThemeDestroyed();

private:
    struct ThemeInfo {
        QString name;
        QString displayName;
        QString description;
        ThemeCreator creator;
        QVariantMap metadata;
    };

    void registerDefaultThemes();
    void setupBuiltinThemes();
    std::shared_ptr<BaseTheme> getCachedTheme(const QString& themeName) const;
    void cacheTheme(const QString& themeName, std::shared_ptr<BaseTheme> theme);

    QMap<QString, ThemeInfo> m_registeredThemes;
    QMap<QString, std::weak_ptr<BaseTheme>> m_themeCache;
    QString m_defaultThemeName;
    bool m_cachingEnabled;

    static ThemeFactory* s_instance;
};

#endif // THEMEFACTORY_H