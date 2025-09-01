#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QObject>
#include <QTranslator>
#include <QLocale>
#include <QMap>
#include <QString>
#include <QStringList>

/**
 * @brief 翻译管理器
 * 
 * 负责管理应用程序的语言翻译，支持动态切换语言和加载翻译文件。
 */
class TranslationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentLanguage READ currentLanguage WRITE setCurrentLanguage NOTIFY currentLanguageChanged)
    Q_PROPERTY(QStringList availableLanguages READ availableLanguages NOTIFY availableLanguagesChanged)

public:
    explicit TranslationManager(QObject *parent = nullptr);
    ~TranslationManager();

    /**
     * @brief 获取单例实例
     */
    static TranslationManager* instance();

    /**
     * @brief 初始化翻译管理器
     * @return 是否成功初始化
     */
    bool initialize();

    /**
     * @brief 关闭翻译管理器
     */
    void shutdown();

    /**
     * @brief 设置当前语言
     * @param language 语言代码（如"en_US"、"zh_CN"等）
     * @return 是否成功设置
     */
    bool setCurrentLanguage(const QString& language);

    /**
     * @brief 设置语言
     * @param language 语言代码
     * @return 是否成功设置
     */
    bool setLanguage(const QString& language);

    /**
     * @brief 获取当前语言
     * @return 当前语言代码
     */
    QString currentLanguage() const;

    /**
     * @brief 获取当前语言代码
     * @return 当前语言代码
     */
    QString currentLanguageCode() const;

    /**
     * @brief 获取当前语言名称
     * @return 当前语言名称
     */
    QString currentLanguageName() const;

    /**
     * @brief 获取可用语言列表
     * @return 语言代码列表
     */
    QStringList availableLanguages() const;

    /**
     * @brief 获取语言名称
     * @param language 语言代码
     * @return 语言名称
     */
    QString getLanguageName(const QString& language) const;

    /**
     * @brief 获取语言代码
     * @param languageName 语言名称
     * @return 语言代码
     */
    QString getLanguageCode(const QString& languageName) const;

    /**
     * @brief 加载翻译文件
     * @param filePath 文件路径
     * @param language 语言代码
     * @return 是否成功加载
     */
    bool loadTranslationFile(const QString& filePath, const QString& language);

    /**
     * @brief 加载所有翻译文件
     * @return 是否成功加载
     */
    bool loadAllTranslations();

    /**
     * @brief 获取系统语言
     * @return 系统语言代码
     */
    QString getSystemLanguage() const;

    /**
     * @brief 获取系统语言枚举
     * @return 系统语言枚举值
     */
    QLocale::Language systemLanguage() const;

    /**
     * @brief 翻译文本
     * @param text 原文本
     * @param context 上下文
     * @return 翻译后的文本
     */
    QString translate(const QString& text, const QString& context = QString()) const;

    /**
     * @brief 设置翻译文件目录
     * @param path 目录路径
     */
    void setTranslationsPath(const QString& path);

    /**
     * @brief 获取翻译文件目录
     * @return 目录路径
     */
    QString translationsPath() const;

    /**
     * @brief 重新加载当前翻译
     * @return 是否成功重新加载
     */
    bool reloadCurrentTranslation();

    /**
     * @brief 检查语言是否可用
     * @param language 语言代码
     * @return 是否可用
     */
    bool isLanguageAvailable(const QString& language) const;

signals:
    /**
     * @brief 当前语言变更信号
     * @param language 语言代码
     */
    void currentLanguageChanged(const QString& language);

    /**
     * @brief 可用语言列表变更信号
     * @param languages 语言代码列表
     */
    void availableLanguagesChanged(const QStringList& languages);

    /**
     * @brief 翻译加载信号
     * @param language 语言代码
     * @param success 是否成功
     */
    void translationLoaded(const QString& language, bool success);

private slots:
    void onLocaleChanged(const QLocale& locale);

private:
    void detectAvailableTranslations();
    bool installTranslator(const QString& language);
    void removeCurrentTranslator();
    QString getTranslationFilePath(const QString& language) const;
    void loadLanguageNames();
    void setupDefaultLanguage();

    static TranslationManager* s_instance;
    QTranslator m_translator;
    QString m_currentLanguage;
    QStringList m_availableLanguages;
    QMap<QString, QString> m_languageNames;
    QString m_translationsPath;
    bool m_initialized;
};

#endif // TRANSLATIONMANAGER_H