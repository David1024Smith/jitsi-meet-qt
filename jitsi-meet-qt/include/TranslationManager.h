#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QObject>
#include <QTranslator>
#include <QLocale>
#include <QStringList>

/**
 * @brief 翻译管理器，处理应用程序的多语言支持
 */
class TranslationManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit TranslationManager(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~TranslationManager();
    
    /**
     * @brief 初始化翻译系统
     */
    void initialize();
    
    /**
     * @brief 获取可用语言列表
     * @return 语言代码列表
     */
    QStringList availableLanguages() const;
    
    /**
     * @brief 获取语言显示名称
     * @param languageCode 语言代码
     * @return 显示名称
     */
    QString languageDisplayName(const QString& languageCode) const;
    
    /**
     * @brief 获取当前语言
     * @return 当前语言代码
     */
    QString currentLanguage() const;
    
    /**
     * @brief 设置语言
     * @param languageCode 语言代码，"auto"表示自动检测
     * @return 设置成功返回true
     */
    bool setLanguage(const QString& languageCode);
    
    /**
     * @brief 检测系统语言
     * @return 系统语言代码
     */
    QString detectSystemLanguage() const;
    
    /**
     * @brief 翻译文本
     * @param key 翻译键
     * @param defaultText 默认文本
     * @return 翻译后的文本
     */
    QString translate(const QString& key, const QString& defaultText = QString()) const;

signals:
    /**
     * @brief 语言改变信号
     * @param languageCode 新的语言代码
     */
    void languageChanged(const QString& languageCode);

private slots:
    /**
     * @brief 处理配置管理器的语言改变
     * @param language 新的语言设置
     */
    void onConfigLanguageChanged(const QString& language);

private:
    /**
     * @brief 加载翻译文件
     * @param languageCode 语言代码
     * @return 加载成功返回true
     */
    bool loadTranslation(const QString& languageCode);
    
    /**
     * @brief 卸载当前翻译
     */
    void unloadCurrentTranslation();
    
    /**
     * @brief 获取翻译文件路径
     * @param languageCode 语言代码
     * @return 翻译文件路径
     */
    QString getTranslationFilePath(const QString& languageCode) const;
    
    /**
     * @brief 初始化可用语言列表
     */
    void initializeAvailableLanguages();

private:
    QTranslator* m_appTranslator;
    QTranslator* m_qtTranslator;
    QString m_currentLanguage;
    QStringList m_availableLanguages;
    QMap<QString, QString> m_languageNames;
};

#endif // TRANSLATIONMANAGER_H