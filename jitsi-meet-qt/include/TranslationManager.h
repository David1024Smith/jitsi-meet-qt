#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QObject>
#include <QTranslator>
#include <QLocale>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QDir>
#include <memory>
#include <optional>

/**
 * @brief TranslationManager handles internationalization and localization
 * 
 * This class manages language resources, system language detection,
 * and runtime language switching for the Jitsi Meet Qt application.
 * 
 * Features:
 * - Automatic system language detection
 * - Runtime language switching
 * - Translation file management
 * - Fallback to English for unsupported languages
 */
class TranslationManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Supported languages in the application
     */
    enum class Language {
        Auto,           // Auto-detect system language
        English,        // en
        Chinese,        // zh_CN
        Spanish,        // es
        French,         // fr
        German,         // de
        Japanese,       // ja
        Korean,         // ko
        Russian,        // ru
        Portuguese,     // pt
        Italian         // it
    };

    /**
     * @brief Language information structure
     */
    struct LanguageInfo {
        Language language;
        QString code;           // Language code (e.g., "en", "zh_CN")
        QString nativeName;     // Native language name (e.g., "English", "中文")
        QString englishName;    // English language name
        bool available;         // Whether translation file is available
    };

    explicit TranslationManager(QObject *parent = nullptr);
    ~TranslationManager();

    /**
     * @brief Get singleton instance
     */
    static TranslationManager* instance();

    /**
     * @brief Initialize translation system
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Set application language
     * @param language Target language
     * @return true if language was set successfully
     */
    bool setLanguage(Language language);

    /**
     * @brief Set application language by code
     * @param languageCode Language code (e.g., "en", "zh_CN")
     * @return true if language was set successfully
     */
    bool setLanguage(const QString& languageCode);

    /**
     * @brief Get current language
     */
    Language currentLanguage() const;

    /**
     * @brief Get current language code
     */
    QString currentLanguageCode() const;

    /**
     * @brief Get system language
     */
    Language systemLanguage() const;

    /**
     * @brief Get list of available languages
     */
    QList<LanguageInfo> availableLanguages() const;

    /**
     * @brief Get language info by language enum
     */
    std::optional<LanguageInfo> getLanguageInfo(Language language) const;

    /**
     * @brief Get language info by language code
     */
    std::optional<LanguageInfo> getLanguageInfo(const QString& languageCode) const;

    /**
     * @brief Check if language is supported
     */
    bool isLanguageSupported(Language language) const;
    bool isLanguageSupported(const QString& languageCode) const;

    /**
     * @brief Get translation for key (convenience function)
     */
    QString translate(const QString& context, const QString& key, const QString& disambiguation = QString()) const;

    /**
     * @brief Reload translations (useful for development)
     */
    void reloadTranslations();

signals:
    /**
     * @brief Emitted when language changes
     */
    void languageChanged(Language newLanguage, const QString& languageCode);

    /**
     * @brief Emitted when translation loading fails
     */
    void translationLoadFailed(const QString& languageCode, const QString& error);

private slots:
    void onApplicationLanguageChanged();

private:
    /**
     * @brief Detect system language
     */
    Language detectSystemLanguage() const;

    /**
     * @brief Convert Language enum to language code
     */
    QString languageToCode(Language language) const;

    /**
     * @brief Convert language code to Language enum
     */
    Language codeToLanguage(const QString& code) const;

    /**
     * @brief Load translation file
     */
    bool loadTranslation(const QString& languageCode);

    /**
     * @brief Unload current translation
     */
    void unloadTranslation();

    /**
     * @brief Get translation file path
     */
    QString getTranslationFilePath(const QString& languageCode) const;

    /**
     * @brief Check if translation file exists
     */
    bool translationFileExists(const QString& languageCode) const;

    /**
     * @brief Initialize language info list
     */
    void initializeLanguageInfo();

    /**
     * @brief Update available languages based on existing translation files
     */
    void updateAvailableLanguages();

    static TranslationManager* s_instance;

    Language m_currentLanguage;
    QString m_currentLanguageCode;
    std::unique_ptr<QTranslator> m_translator;
    std::unique_ptr<QTranslator> m_qtTranslator;  // For Qt's built-in strings
    QList<LanguageInfo> m_languageInfo;
    QString m_translationsPath;
    bool m_initialized;
};

// Convenience macro for translation
#define TR(context, key) TranslationManager::instance()->translate(context, key)

#endif // TRANSLATIONMANAGER_H